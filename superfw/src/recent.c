/*
 * Copyright (C) 2026 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "recent.h"

#include "common.h"
#include "compiler.h"
#include "gbahw.h"
#include "util.h"
#include "fatfs/ff.h"

#pragma GCC optimize ("Os")

static bool recent_flush_file(const char *path,const t_rentry *rentries,unsigned rcount,bool sd_only) {
  WRITE_LOG("Flushing recently played games (%d entries)", rcount);

  char temporary[64],backup[64];
  if(strlen(path)+5>sizeof(temporary))return false;
  strcpy(temporary,path);strcat(temporary,".tmp");
  strcpy(backup,path);strcat(backup,".bak");
  if(f_stat(path,0)==FR_NO_FILE && f_stat(backup,0)==FR_OK)
    if(f_rename(backup,path)!=FR_OK)return false;
  // Write a replacement before moving the previous history aside.
  FIL fo;
  if (FR_OK != f_open(&fo, temporary, FA_WRITE | FA_CREATE_ALWAYS))
    return false;

  // Write stuff to disk. Use a 1KiB buffer and flush as full blocks fill.
  unsigned coff = 0;
  char tmpbuf[1024];
  tmpbuf[0] = 0;

  unsigned sd_count=0;
  for (unsigned i = 0; i < rcount; i++) {
    if(sd_only && ((rentries[i].flags&FLAG_RECENT_NOR) || rentries[i].fpath[0]!='/'))continue;
    if(sd_only && sd_count++==10)break;
    unsigned fnlen = strlen(rentries[i].fpath);
    if (rentries[i].flags & FLAG_RECENT_NOR) {
      memcpy(&tmpbuf[coff], "nor:", 4);
      coff += 4;
    }
    memcpy(&tmpbuf[coff], rentries[i].fpath, fnlen);
    coff += fnlen;
    tmpbuf[coff++] = '\n';

    if (coff >= 512) {
      UINT wrbytes;
      if (FR_OK != f_write(&fo, tmpbuf, 512, &wrbytes) || wrbytes != 512) {
        f_close(&fo);
        return false;
      }
      // Consume the first 512 written bytes
      memmove(&tmpbuf[0], &tmpbuf[512], coff - 512);
      coff -= 512;
    }
  }

  // Flush the last bytes (if any!)
  if (coff) {
    UINT wrbytes;
    if (FR_OK != f_write(&fo, tmpbuf, coff, &wrbytes) || wrbytes != coff) {
      f_close(&fo);
      return false;
    }
  }

  if(f_close(&fo)!=FR_OK)return false;
  bool existed=f_stat(path,0)==FR_OK;
  if(f_stat(backup,0)==FR_OK && f_unlink(backup)!=FR_OK)return false;
  if(existed && f_rename(path,backup)!=FR_OK)return false;
  if(f_rename(temporary,path)!=FR_OK) {
    if(existed)f_rename(backup,path);
    return false;
  }
  if(existed)f_unlink(backup);
  return true;
}

NOINLINE bool recent_flush(const t_rentry *entries,unsigned count) {
  FRESULT result=f_mkdir(SYSTEM_DIR);
  if(result!=FR_OK && result!=FR_EXIST)return false;
  return recent_flush_file(PRIVATE_RECENT_FILEPATH,entries,count,false) &&
         recent_flush_file(RECENT_FILEPATH,entries,count,true);
}

NOINLINE unsigned insert_recent_fn(t_rentry *rentries, unsigned rcount, const char *fn, unsigned flags) {
  /* Native word copies round upward: stage the bounded name in padded GBA
   * RAM, also protecting a filename that aliases the table being reordered. */
  char stable[MAX_FN_LEN] __attribute__((aligned(4)));
  size_t length=strlen(fn);
  if(length>=sizeof(stable))return rcount;
  memset(stable,0,(length+4)&~3u);memcpy(stable,fn,length+1);fn=stable;
  WRITE_LOG("Adding/bumping recently played game: '%s' [%x]", fn, flags);

  for (unsigned i = 0; i < rcount; i++) {
    if (rentries[i].flags == flags && !strcmp(rentries[i].fpath, fn)) {
      // Found a matching file, move it to position 0, unless it's there already.
      if (i) {
        t_rentry tmp;
        memcpy32(&tmp, &rentries[i], sizeof(tmp));   // Copy entry to tmp
        memmove32(&rentries[1], &rentries[0], i * sizeof(rentries[0]));
        memcpy32(&rentries[0], &tmp, sizeof(tmp));
      }
      return rcount;
    }
  }

  // Not in the list, push all items back and insert it in the first position
  if (rcount) {
    unsigned movecnt = MIN(rcount, RECENT_MAXFN_CNT - 1);
    memmove32(&rentries[1], &rentries[0], movecnt * sizeof(rentries[0]));
  }

  const char *pbn = file_basename(fn);
  rentries[0].fname_offset = pbn - fn;
  rentries[0].flags = flags;
  memcpy32(rentries[0].fpath, fn, strlen(fn) + 1);
  return MIN(rcount + 1, RECENT_MAXFN_CNT);
}

NOINLINE unsigned delete_recent(t_rentry *rentries, unsigned rcount, unsigned entry_num) {
  if (entry_num >= rcount)
    return rcount;

  if (entry_num + 1 < rcount)
    memmove32(&rentries[entry_num], &rentries[entry_num + 1],
              (rcount - (entry_num + 1)) * sizeof(rentries[0]));

  return rcount - 1;
}

NOINLINE unsigned recent_load(const char *fpath, t_rentry *rentries) {
  char backup[64];
  if(strlen(fpath)+5<=sizeof(backup)) {
    strcpy(backup,fpath);strcat(backup,".bak");
    if(f_stat(fpath,0)==FR_NO_FILE && f_stat(backup,0)==FR_OK)
      if(f_rename(backup,fpath)!=FR_OK)return 0;
  }
  FIL fi;
  if (FR_OK != f_open(&fi, fpath, FA_READ))
    return 0;

  // Read data block by block.
  char tmp[1024 + 4];
  unsigned bcount = 0, nentries = 0;
  while (nentries < RECENT_MAXFN_CNT) {
    if (bcount <= 512) {
      UINT rdbytes;
      if (FR_OK != f_read(&fi, &tmp[bcount], 512, &rdbytes)) {
        f_close(&fi);return nentries;
      }
      bcount += rdbytes;
      tmp[bcount] = 0;
    }

    if (!bcount)
      break;

    // Attempt to parse the next path.
    char *p = strchr(tmp, '\n');
    if (!p)
      p = strchr(tmp, '\0');
    *p = 0;        // Add the string end char.

    unsigned cnt = p - tmp + 1;
    if(p>tmp && p[-1]=='\r')p[-1]=0;
    if (cnt > 1 && cnt < sizeof(rentries[nentries].fpath)) {
      rentries[nentries].flags = 0;
      if (!memcmp(tmp, "nor:", 4)) {
        rentries[nentries].flags |= FLAG_RECENT_NOR;
        dma_memcpy16(rentries[nentries].fpath, &tmp[4], (cnt - 4 + 1) / 2);
      }
      else
        dma_memcpy16(rentries[nentries].fpath, tmp, (cnt + 1) / 2);

      rentries[nentries].fname_offset = file_basename(rentries[nentries].fpath) - rentries[nentries].fpath;
      nentries++;
    }

    // Consume the bytes
    if (bcount < cnt)
      bcount = 0;      // Consumed the full buffer (ie. last line or too long).
    else {
      memmove(&tmp[0], &tmp[cnt], bcount - cnt + 1);
      bcount -= cnt;
    }
  }

  WRITE_LOG("Loaded recently played games. %d entries found", nentries);

  f_close(&fi);
  return nentries;
}



/* Use shared SD order when another kernel changes it, retaining older SuperFW
 * entries and private NOR names without putting nor: records in DE RECENT.TXT. */
unsigned recent_load_merged(t_rentry *entries,t_rentry *scratch) {
  unsigned count=recent_load(PRIVATE_RECENT_FILEPATH,entries);
  unsigned shared=recent_load(RECENT_FILEPATH,scratch);
  if(!count){memcpy32(entries,scratch,shared*sizeof(*entries));return shared;}
  unsigned first_sd=0;
  while(first_sd<count && (entries[first_sd].flags&FLAG_RECENT_NOR))++first_sd;
  if(!shared || (first_sd<count && !strcmp(entries[first_sd].fpath,scratch[0].fpath)))return count;
  /* Reverse insertion keeps the shared newest-first order. */
  for(unsigned i=shared;i;i--)
    count=insert_recent_fn(entries,count,scratch[i-1].fpath,scratch[i-1].flags);
  return count;
}
