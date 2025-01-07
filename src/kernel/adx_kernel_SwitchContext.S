;$Id: adx_kernel_SwitchContext.S 263 2024-11-11 13:00:52Z apolv $

.global SwitchContext

SwitchContext:
;void SwitchContext(ptrCurrentContext, ptrNextContext);//Do NOT invoke from ISR!!!
;Save current Context (SPL, SPH, r2, .., r17, r28, r29, SREG), r1 must be 0 after return
;registers r0, r18, .., r27, r30, r31 may not be saved
movw r26, r24	;X = ptrCurrentContext (r25:r24 - 1st 2-byte parameter)
in r0, 0x3f		;r0 = SREG
in r1, 0x3d		;r1 = SPL
st X+, r1		;*(ptrCurrentContext + 0) = SPL
in r1, 0x3e		;r1 = SPH
st X+, r1		;*(ptrCurrentContext + 1) = SPH
st X+, r2		;*(ptrCurrentContext + 2) = r2
st X+, r3		;...
st X+, r4
st X+, r5
st X+, r6
st X+, r7
st X+, r8
st X+, r9
st X+, r10
st X+, r11
st X+, r12
st X+, r13
st X+, r14
st X+, r15
st X+, r16
st X+, r17
st X+, r28
st X+, r29		;*(ptrCurrentContext + 19) = r29
st X+, r0		;*(ptrCurrentContext + 20) = SREG
;Load new Context (SPL, SPH, r2, .., r17, r28, r29, SREG)
movw r26,r22	;X = ptrNextContext (r23:r22 - 2nd 2-byte parameter)
ld r0, X+		;r0 = *(ptrNextContext + 0) //next SPL
ld r1, X+		;r1 = *(ptrNextContext + 1) //next SPH
ld r2, X+		;r2 = *(ptrNextContext + 2) //next r2
ld r3, X+		;...
ld r4, X+
ld r5, X+
ld r6, X+
ld r7, X+
ld r8, X+
ld r9, X+
ld r10, X+
ld r11, X+
ld r12, X+
ld r13, X+
ld r14, X+
ld r15, X+
ld r16, X+
ld r17, X+
ld r28, X+
ld r29, X+		;r29 = *(ptrNextContext + 19) //next r29
ld r18, X+		;r18 = next SREG
;start of switching context
out 0x3d, r0	;SPL = next SPL //Interrupts are off by XMEGA hardwade until next I/O, i.e. SPH loading (next instruction)
out 0x3e, r1	;SPH = next SPH 
;context switched
out 0x3f, r18	;SREG = next SREG
clr r1			;r1 = 0 //according to avr-gcc calling conventions r1 must be 0 after function return
ret				;gets 3-byte pointer from new (already switched) Stack and jumps to new Task