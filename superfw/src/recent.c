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

/* A failed read must not turn a partial history into a successful replacement. */
static bool history_read_ok = true;

static bool recent_flush_file(const char *path,const t_rentry *rentries,unsigned rcount) {
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

  for (unsigned i = 0; i < rcount; i++) {
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
  if (!history_read_ok) return false;
  FRESULT result=f_mkdir(SYSTEM_DIR);
  if(result!=FR_OK && result!=FR_EXIST)return false;
  if(!recent_flush_file(RECENT_FILEPATH,entries,count))return false;
  /* Retire the old sidecar only after the canonical replacement commits. */
  f_unlink(LEGACY_RECENT_FILEPATH);
  f_unlink(LEGACY_RECENT_FILEPATH ".bak");
  return true;
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

/* The line buffer lives in GBA RAM; only halfwords reach cartridge history. */
static unsigned append_recent_line(t_rentry *entries, unsigned count,
                                   char *line, unsigned length) {
  if (length && line[length-1] == '\r') --length;
  line[length] = 0;
  if (length+1 < MAX_FN_LEN+6) line[length+1] = 0;
  bool nor = length >= 4 && !memcmp(line, "nor:", 4);
  char *path = line + (nor ? 4 : 0);
  unsigned path_length = length - (nor ? 4 : 0);
  if (!path_length || path_length >= MAX_FN_LEN || (!nor && path[0] != '/'))
    return count;
  entries[count].flags = nor ? FLAG_RECENT_NOR : FLAG_RECENT_SD;
  entries[count].fname_offset = file_basename(path) - path;
  dma_memcpy16(entries[count].fpath, path, (path_length+2)/2);
  return count+1;
}

static unsigned read_recent(const char *path, t_rentry *entries, bool *ok) {
  *ok = true;
  char backup[64];
  if (strlen(path)+5 <= sizeof(backup)) {
    strcpy(backup, path);
    strcat(backup, ".bak");
    if (f_stat(path, 0) == FR_NO_FILE && f_stat(backup, 0) == FR_OK &&
        f_rename(backup, path) != FR_OK) {
      *ok = false;
      return 0;
    }
  }
  FIL file;
  FRESULT result = f_open(&file, path, FA_READ);
  if (result != FR_OK) {
    *ok = result == FR_NO_FILE || result == FR_NO_PATH;
    return 0;
  }

  char block[512], line[MAX_FN_LEN+6] __attribute__((aligned(4)));
  unsigned count = 0, used = 0;
  bool discard = false;
  while (count < RECENT_MAXFN_CNT) {
    UINT got;
    if (f_read(&file, block, sizeof(block), &got) != FR_OK) {
      *ok = false;
      break;
    }
    if (!got) {
      if (used && !discard) count = append_recent_line(entries, count, line, used);
      break;
    }
    for (unsigned i = 0; i < got && count < RECENT_MAXFN_CNT; ++i) {
      if (block[i] == '\n') {
        if (!discard) count = append_recent_line(entries, count, line, used);
        used = 0;
        discard = false;
      } else if (!discard) {
        if (!block[i] || used == sizeof(line)-1) discard = true;
        else line[used++] = block[i];
      }
    }
  }
  if (f_close(&file) != FR_OK) *ok = false;
  return count;
}

NOINLINE unsigned recent_load(const char *path, t_rentry *entries) {
  bool ok;
  return read_recent(path, entries, &ok);
}

/* Import the old sidecar once, keeping canonical RECENT.TXT newest-first.
 * NOR records retain their existing nor: encoding in the single history file. */
unsigned recent_load_merged(t_rentry *entries,t_rentry *scratch) {
  bool legacy_ok, canonical_ok;
  unsigned count=read_recent(LEGACY_RECENT_FILEPATH,entries,&legacy_ok);
  unsigned shared=read_recent(RECENT_FILEPATH,scratch,&canonical_ok);
  history_read_ok=legacy_ok && canonical_ok;
  if(!count){memcpy32(entries,scratch,shared*sizeof(*entries));return shared;}
  if(!shared)return count;
  /* Reverse insertion keeps the shared newest-first order. */
  for(unsigned i=shared;i;i--)
    count=insert_recent_fn(entries,count,scratch[i-1].fpath,scratch[i-1].flags);
  return count;
}
