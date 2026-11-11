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
		ldr		r1, =0x03007f00
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
	ldr		r1, =0x3007f00
	mov		SP, r1
	swi     0x26
	
	.align
	.ltorg
SoftReset_end:

@ DS Style boot modes, after BLUE has selected the game page.
@ Move the user stack before clearing EWRAM. This routine never returns.
    .syntax unified
    .align 2
    .code 16
    .thumb_func
    .global Blue_BootHandoff
    .type Blue_BootHandoff, %function
Blue_BootHandoff:
    movs r4, r0
    ldr r1, =0x04000208
    movs r2, #0
    strh r2, [r1]
    ldr r1, =0x03007F00
    mov sp, r1
    cmp r4, #0
    beq Blue_NativeSoft
    cmp r4, #6
    beq Blue_NativeHard
    movs r0, #0xFC
    bl RegisterRamReset
    cmp r4, #1
    beq Blue_PluginHard
    cmp r4, #5
    beq Blue_PluginSoft
    ldr r1, =0x02000000
    ldr r2, =0x0203FBF8
    movs r0, #0
Blue_ClearPluginRam:
    stmia r1!, {r0}
    cmp r1, r2
    blo Blue_ClearPluginRam
    cmp r4, #2
    beq Blue_CopyPlugin
    cmp r4, #4
    beq Blue_ExpandPlugin
Blue_PluginSoft:
    b SoftReset_now
Blue_PluginHard:
    b HardReset
Blue_NativeSoft:
    movs r0, #0xFD
    bl RegisterRamReset
    b SoftReset_now
Blue_NativeHard:
    movs r0, #0xFD
    bl RegisterRamReset
    b HardReset
Blue_CopyPlugin:
    ldr r1, =0x0203FBFC
    ldr r2, [r1]
    ldr r3, =0x07FFFFFF
    ands r2, r3
    ldr r0, =0x02000000
    ldr r1, =0x08000000
    movs r3, #0
Blue_CopyPluginWord:
    ldr r5, [r1, r3]
    str r5, [r0, r3]
    adds r3, #4
    cmp r3, r2
    blo Blue_CopyPluginWord
    b Blue_StartEwramPlugin
Blue_ExpandPlugin:
    ldr r0, =0x08000000
    ldr r1, =0x02000000
    swi 0x11
Blue_StartEwramPlugin:
    movs r0, #0xFC
    bl RegisterRamReset
    ldr r0, =0x02000000
    bx r0
    .ltorg
    .size Blue_BootHandoff, .-Blue_BootHandoff
