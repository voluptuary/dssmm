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
  @ Four halfwords per branch; retain the cartridge's required write width.
  ldrh r3, [r0], #2
  strh r3, [r1], #2
  ldrh r3, [r0], #2
  strh r3, [r1], #2
  ldrh r3, [r0], #2
  strh r3, [r1], #2
  ldrh r3, [r0], #2
  strh r3, [r1], #2
  subs r2, r2, #8
  bhi 2b
  @ Mark this exact restored image for a single snapshot reuse at boot.
  ldr r0, =0x098000e0
  ldr r8, =0x0203ffe4
  ldmia r0, {r1-r3,r7}
  stmia r8, {r1-r3,r7}
  ldr r9, =0x54455244
  str r9, [r8, #-4]
  mov r6, #1
  strh r5, [r4]
  strh r5, [r4]
  strh r6, [r4]
  strh r6, [r4]
  ldr r0, =0x03007ffa
  mov r1, #0
  strb r1, [r0]
  @ Reset video/palettes/IO like native emulator Exit, preserving RAM and snapshot.
  mov r0, #0xfc
  swi 0x010000
  ldr r0, =0x04000204
  mov r1, #0x4000
  strh r1, [r0]
  swi 0
9:
  b 9b
  .pool
pogo_return_code_end:

@ Called through an absolute address from IWRAM, before any card remapping.
@ The reserved EWRAM token is consumed even when the source/hash does not match.
pogo_boot_reuse:
  ldr r1, =0x0203ffe0
  ldr r3, [r1]
  mov r4, #0
  str r4, [r1]
  cmp r12, #5
  bne 9f
  ldr r4, =0x54455244
  cmp r3, r4
  bne 9f
  add r0, r0, #0xe0
  add r1, r1, #4
  mov r3, #4
1:
  ldr r4, [r0], #4
  ldr r5, [r1], #4
  cmp r4, r5
  bne 9f
  subs r3, r3, #1
  bne 1b
  movs r0, #0
  bx lr
9:
  movs r0, #0x08000000
  bx lr
  .pool
