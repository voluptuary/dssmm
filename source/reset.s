@---------------------------------------------------------------------------------
	.section	.iwram,"ax",%progbits


@;--------------------------------------------------------------
	.code 16
	.global	SoftReset_now
SoftReset_now:
	ldr r1,=SoftReset_now16
	bx  r1
	.code 32
SoftReset_now16:
	adr r1,SoftReset
	adr r3,SoftReset_end
	mov r2,#0x02000000
	add r2,#0x1000 @;533
copy_loop:
	ldr r0,[r1],#4
	str r0,[r2],#4
	cmp r1,r3
	blt copy_loop

	mov		r2,#0x3000000
	@;add		r3,r2,#0x1000
	ldr		r3,=SoftReset_now
	mov		r0,#0
clearL:
	str		r0,[r2],#+4
	cmp		r2,r3
	blt		clearL

	ldr		r2,=RegisterRamReset
	ldr		r3,=0x3008000
	mov		r0,#0
clearL2:
	str		r0,[r2],#+4
	cmp		r2,r3
	blt		clearL2

	mov r0,#0x02000000
	add r0,r0,#0x1000
	add r0,r0,#1
	bx r0
@----------------------------------------------
	.code 16

	.global RegisterRamReset
RegisterRamReset:
	swi		1
	bx		lr

@; Clear EWRAM only after moving off the EWRAM user stack. Keep the reset handoff
@; in IWRAM so returning from RegisterRamReset never touches the cleared stack.
	.global ResetWithEwramClear
	.type ResetWithEwramClear, %function
ResetWithEwramClear:
	ldr	r1, =0x04000208
	movs	r2, #0
	strh	r2, [r1]
	ldr	r1, =0x03007F00
	mov	sp, r1
	bl	RegisterRamReset
	b	SoftReset_now

@----------------------------------------------
	.global	SoftReset
SoftReset:
		ldr		r3, =0x04000208
		mov		r2, #0
		str		r2, [r3, #0]
		@;ldr		r3, =0x03007FFA
		@;mov		r0, #0x2 @;FD
		@;strb		r0, [r3, #0]
		@;swi		0x1
		ldr		r6, =0x03007F00
		mov		r5,#1
		str		r5, [r6]
		ldr		r6, =0x03007FFA
		mov		r7, #0x0
		str		r7, [r6, #0]
		ldr		r1, =__sp_usr
		mov		sp, r1
		swi		0


	@ldr		r3, =0x03007FFA		@ restart flag
	@strb	r0,[r3, #0]
	@ldr		r3, =0x04000208		@ REG_IME
	@mov		r2, #0
	@strb	r2, [r3, #0]
	@ldr		r1, =0x03007f00
	@mov		sp, r1
	@swi		1
	@swi		0

@----------------------------------------------
	.global	HardReset
HardReset:
	ldr		r3, =0x04000208
	mov		r2, #0
	str		r2, [r3, #0]
	ldr		r1, =__sp_usr
	mov		SP, r1
	swi     0x26

	.align
	.ltorg
SoftReset_end:

@ No return to a C frame in EWRAM after clearing/replacing that memory.
@ Keep this outside SoftReset..SoftReset_end so the native stub is unchanged.
    .syntax unified
    .thumb
    .align 2
    .global DE_BootHandoff
    .type DE_BootHandoff, %function
    .thumb_func
DE_BootHandoff:
    movs r4, r0
    ldr r1, =0x03007F00
    mov sp, r1
    movs r0, #0xFC
    cmp r4, #0
    bne 1f
    movs r0, #0xFD
1:  swi 1
    cmp r4, #0
    beq 6f
    cmp r4, #1
    beq 7f
    cmp r4, #4
    bhi 6f
    ldr r1, =0x02000000
    ldr r2, =0x0203FBF8
    movs r0, #0
2:  str r0, [r1]
    adds r1, #4
    cmp r1, r2
    blo 2b
    cmp r4, #3
    beq 6f
    ldr r0, =0x08000000
    ldr r1, =0x02000000
    cmp r4, #4
    beq 4f
    ldr r2, =0x0203FBFC
    ldr r2, [r2]
    lsls r2, r2, #5
    lsrs r2, r2, #5
    ldr r3, =0x3FBF8
    cmp r2, r3
    bhi 8f
    movs r3, #3
    tst r2, r3
    bne 8f
    cmp r2, #0
    beq 8f
3:  ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    subs r2, #4
    bne 3b
    b 5f
4:  swi 0x11
5:  movs r0, #0xFC
    swi 1
    ldr r0, =0x02000000
    bx r0
6:  b SoftReset_now
7:  b HardReset
8:  b 8b
    .ltorg
    .size DE_BootHandoff, .-DE_BootHandoff
