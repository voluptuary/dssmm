/*
 * Copyright (C) 2025 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "patchengine.h"

#define BLK_SIZE     4*1024*1024

static void dummy(unsigned) {}

int main(int argc, char **argv) {
  if (argc <= 1) {
    printf("Usage: %s romfile\n", argv[0]);
    exit(1);
  }

  FILE *fd = fopen(argv[1], "rb");
  if (!fd) {
    printf("Could not open file %s\n", argv[1]);
    exit(1);
  }
  struct stat st;
  stat(argv[1], &st);

  t_patch_builder pb;
  patchengine_init(&pb, st.st_size);
  uint32_t *tmp = malloc(BLK_SIZE);
  if (!tmp) {fclose(fd);return 1;}
  const unsigned step = BLK_SIZE-PATCH_SCAN_LOOKBEHIND-PATCH_SCAN_LOOKAHEAD;
  for (unsigned scan=0; scan<(unsigned)st.st_size; scan+=step) {
    unsigned start=scan>PATCH_SCAN_LOOKBEHIND?scan-PATCH_SCAN_LOOKBEHIND:0;
    unsigned available=(unsigned)st.st_size-start;
    if (available>BLK_SIZE) available=BLK_SIZE;
    unsigned count=(unsigned)st.st_size-scan;
    if (count>step) count=step;
    if (fseek(fd,start,SEEK_SET) || fread(tmp,1,available,fd)!=available) {
      free(tmp);fclose(fd);return 1;
    }
    pb.rom_offset=start;
    if (!patchengine_process_window(tmp,available,scan-start,scan-start+count,&pb,dummy)) {
      fprintf(stderr,"Patch scan failed; no partial result is usable.\n");
      free(tmp);fclose(fd);return 1;
    }
  }

  free(tmp);
  patchengine_finalize(&pb);
  fclose(fd);

  // Print patches for manual inspection:
  printf("Save type: %d\n", pb.p.save_mode);

  printf("WAITCNT patches:\n");
  for (unsigned i = 0; i < pb.p.wcnt_ops; i++)
    printf(" %08x\n", pb.p.op[i]);
  printf("SAVE patches:\n");
  for (unsigned i = 0; i < pb.p.save_ops; i++)
    printf(" %08x\n", pb.p.op[pb.p.wcnt_ops + i]);
  printf("IRQ patches:\n");
  for (unsigned i = 0; i < pb.p.irqh_ops; i++)
    printf(" %08x\n", pb.p.op[pb.p.wcnt_ops + pb.p.save_ops + i]);
  printf("RTC patches:\n");
  for (unsigned i = 0; i < pb.p.rtc_ops; i++)
    printf(" %08x\n", pb.p.op[pb.p.wcnt_ops + pb.p.save_ops + pb.p.irqh_ops + i]);
  printf("Hole addr and size: %x %x\n", pb.p.hole_addr, pb.p.hole_size);
}

