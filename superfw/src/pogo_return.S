/* SPDX-License-Identifier: GPL-3.0-or-later
 * SuperCard-only return from a portable emulator to the current firmware image.
 * Runs entirely from reserved cartridge SDRAM; does not depend on emulator RAM.
 */
.arm
.balign 4
pogo_return_code:
  b 1f
  .ascii "DSRT"
1:
  mov r0, #0x04000000
  mov r1, #0
  str r1, [r0, #0x208]
  strh r1, [r0, #0xba]
  strh r1, [r0, #0xc6]
  strh r1, [r0, #0xd2]
  strh r1, [r0, #0xde]
  mov r1, #0x4000
  add r0, r0, #0x200
  strh r1, [r0, #4]
  ldr r0, =0x09800000
  ldr r2, [r0, #0xcc]
  cmp r2, #256
  blo 9f
  movs r3, r2, lsl #23
  bne 9f
  cmp r2, #0x200000
  bhi 9f
  ldr r4, =0x09fffffe
  ldr r5, =0xa55a
  mov r6, #5
  strh r5, [r4]
  strh r5, [r4]
  strh r6, [r4]
  strh r6, [r4]
  mov r1, #0x08000000
2:
  ldrh r3, [r0], #2
  strh r3, [r1], #2
  subs r2, r2, #2
  bhi 2b
  mov r6, #1
  strh r5, [r4]
  strh r5, [r4]
  strh r6, [r4]
  strh r6, [r4]
  ldr r0, =0x03007ffa
  mov r1, #0
  strb r1, [r0]
  swi 0
9:
  b 9b
  .pool
pogo_return_code_end:
