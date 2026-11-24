/* SPDX-License-Identifier: GPL-3.0-or-later */
/* ROM-only experimental startup: preserve the fixed Pogoshell argument block. */
.syntax unified
.cpu arm7tdmi
.arm
.section .crt0,"ax",%progbits
.global _start
_start:
  b start
  .space 188
start:
  mov r0,#0x04000000
  mov r1,#0
  str r1,[r0,#0x208]
  ldr r0,=0x04000204
  mov r1,#0x4000
  strh r1,[r0]
  mov r0,#0xd2
  msr cpsr_c,r0
  ldr sp,=__sp_irq
  mov r0,#0xdf
  msr cpsr_c,r0
  ldr sp,=__sp_usr
  ldr r0,=0x06000000
  ldr r1,=0x06018000
  bl clear
  ldr r0,=0x04000020
  mov r1,#256
  strh r1,[r0]
  strh r1,[r0,#6]
  mov r1,#0
  strh r1,[r0,#2]
  strh r1,[r0,#4]
  str r1,[r0,#8]
  str r1,[r0,#12]
  strh r1,[r0,#0x30]
  ldr r0,=__bss_start__
  ldr r1,=__bss_end__
  bl clear
  ldr r0,=__sbss_start__
  ldr r1,=__sbss_end__
  bl clear
  ldr r0,=__data_start__
  ldr r1,=__data_end__
  ldr r2,=__data_lma
  bl copy
  ldr r0,=__iwram_start__
  ldr r1,=__iwram_end__
  ldr r2,=__iwram_lma
  bl copy
  ldr r0,=__ewram_start
  ldr r1,=__ewram_end
  ldr r2,=__ewram_lma
  bl copy
#ifdef A26_FIXTURE
  /* Standalone mGBA seam only; runtime plugins receive these from the kernel. */
  ldr r0,=0x0203fbf8
  mov r1,#4096
  str r1,[r0]
  ldr r1,=atari_diagnostic_rom
  str r1,[r0,#4]
  ldr r1,=0xfab0babe
  str r1,[r0,#8]
  mov r1,#2
  str r1,[r0,#12]
#endif
  ldr r3,=__libc_init_array
  bl call_r3
  ldr r3,=main
  bl call_r3
stop:
  b stop
clear:
  mov r2,#0
1: cmp r0,r1
  strlo r2,[r0],#4
  blo 1b
  bx lr
copy:
1: cmp r0,r1
  ldrlo r3,[r2],#4
  strlo r3,[r0],#4
  blo 1b
  bx lr
call_r3:
  bx r3
.global _init
.type _init,%function
_init:
  bx lr
.pool
