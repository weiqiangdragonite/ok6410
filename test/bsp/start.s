/*
 * ok6410/bsp/start.s
 *
 * Board start.
 */

.section .data
	/* ARM system mode (enable IRQ and FIQ) */
	.equ	USR_MODE, 0x10
	.equ	FIQ_MODE, 0x11
	.equ	IRQ_MODE, 0x12
	.equ	SVC_MODE, 0x13
	.equ	ABT_MODE, 0x17
	.equ	UND_MODE, 0x1B
	.equ	SYS_MODE, 0x1F

	/* Disable IRQ */
	.equ	IRQ_DIS, 0x80
	/* Disable FIQ */
	.equ	FIQ_DIS, 0x40

	/* Stack address: start from 0x5FFFFFF0 (align 4 bytes) */
	.equ	STACK_BASE, 0x5FFFFFF0

	/* SVC size = 32 KB */
	.equ	SVC_STACK_START, STACK_BASE
	.equ	SVC_STACK_SIZE, (32 * 1024)

	/* ABT size = 4 KB */
	.equ	ABT_STACK_START, (SVC_STACK_START - SVC_STACK_SIZE)
	.equ	ABT_STACK_SIZE, (4 * 1024)

	/* UND size = 4 KB */
	.equ	UND_STACK_START, (ABT_STACK_START - ABT_STACK_SIZE)
	.equ	UND_STACK_SIZE, (4 * 1024)

	/* IRQ size = 32 KB */
	.equ	IRQ_STACK_START, (UND_STACK_START - UND_STACK_SIZE)
	.equ	IRQ_STACK_SIZE, (32 * 1024)

	/* FIQ size = 16 KB */
	.equ	FIQ_STACK_START, (IRQ_STACK_START - IRQ_STACK_SIZE)
	.equ	FIQ_STACK_SIZE, (16 * 1024)

	/* SYS stack = USR stack */
	.equ	SYS_STACK_START, (FIQ_STACK_START - FIQ_STACK_SIZE)


.section .text
.global _start
_start:

/*
 * Exception vector addresses
 */

	/* 0x00000000  RESET (SVC) - reset */
	b reset
	/* 0x00000004  UND (UND) - undefined instruction */
	ldr pc, =und
	/* 0x00000008  SWI (SVC) - software interrupt */
	ldr pc, =swi
	/* 0x0000000C  PABT (ABT) - prefetch abort */
	ldr pc, =pabt
	/* 0x00000010  DABT (ABT) - data abort */
	ldr pc, =dabt
	/* 0x00000014  Unused */
	nop
	/* 0x00000018  IRQ (IRQ) - interrupt request */
	ldr pc, =irq_isr
	/* 0x0000001C  FIQ (FIQ) - fast interrupt request */
	ldr pc, =fiq

/*
 * Reset system
 */

reset:
	/* Reset is on SVC mode, and disable IQR and FIQ */
	msr cpsr_c, #(SVC_MODE | IRQ_DIS | FIQ_DIS)

	/* Peripheral port setup */
	/* (ARM1176JZF-S) 3.2.49 c15, Peripheral Port Memory Remap Register */
	ldr r0, =0x70000000
	orr r0, r0, #0x13
	mcr p15, 0, r0, c15, c2, 4

	/* Disable watchdog */
	ldr r0, =0x7E004000
	mov r1, #0
	str r1, [r0]

	/* Set the SVC stack on SRAM (SRAM has 8 kb memory) */
	ldr sp, =(7 * 1024)

	/* Init the cpu clock */
	/* void set_clock(int mode) */
	mov r0, #3
	bl set_clock

	/* Init the SDRAM */
	/* void init_sdram(void) */
	bl init_sdram

	/* Init the NAND Flash */
	/* void init_nand(void) */
	bl init_nand

	/* Relocate the code */
	adr r0, _start		/* the relative address of _start */
	ldr r1, =_start		/* get the absolute address of _start */
	ldr r2, =bss_start	/* get the bss_start address */
	sub r2, r2, r1		/* get the code length (.text and .data) */

	cmp r0, r1		/* compare the relative address and
				   absolute address */
	beq clean_bss		/* if equal, go to clean bss */

	/* Copy the code from nand to sdram, parameter is r0, r1, r2 */
	/* void copy_nand_to_sdram(start_addr, dest_addr, length) */
	bl copy_nand_to_sdram

/* clean bss */
clean_bss:
	ldr r0, =bss_start
	ldr r1, =bss_end
	mov r2, #0
clean_loop:
	/* if bss_start != bss_end, clean */
	cmp r0, r1
	strne r2, [r0], #4
	bne clean_loop

	/* Set the stack on the SDRAM */
	/* SVC stack */
	msr cpsr_c, #(SVC_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =SVC_STACK_START
	mov sp, r0

	/* ABT stack */
	msr cpsr_c, #(ABT_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =ABT_STACK_START
	mov sp, r0

	/* UND stack */
	msr cpsr_c, #(UND_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =UND_STACK_START
	mov sp, r0

	/* IRQ stack */
	msr cpsr_c, #(IRQ_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =IRQ_STACK_START
	mov sp, r0

	/* FIQ stack */
	msr cpsr_c, #(FIQ_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =FIQ_STACK_START
	mov sp, r0

	/* SYS stack and USR stack */
	msr cpsr_c, #(SYS_MODE | IRQ_DIS | FIQ_DIS)
	ldr r0, =SYS_STACK_START
	mov sp, r0

	/* Now in the SYS mode (disable FIQ and IRQ) */

	/* Set the return address */
	ldr lr, =boot_end

	/* Jump to SDRAM, and run the boot function */
	/* void boot(void) */
	ldr pc, =boot

boot_end:
	ldr lr, =halt
	/* Run the main function */
	ldr pc, =main

halt:
	/* Halt the system */
	b halt

/*
 * Handle exception
 */

/* Undefined instruction handler */
und:
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	/* Jump to handle function */
	bl handle_und
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/* Software interrupt handler */
swi:
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	
	/* Get the swi number */
	/* r0 ~ r3 store the caller para */
	ldr r4, [lr, #-4]		/* swi address: lr - 4 */
	bic r4, #0xFF000000		/* the low 24 bits is swi number */

	/* Jump to handle function */
	bl handle_swi
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/* Prefetch abort handler */  
pabt:
	/* Modify the return address */
	sub lr, lr, #4
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	/* Jump to handle function */
	bl handle_pabt
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/* Data abort handler */
dabt:
	/* Modify the return address */
	sub lr, lr, #8
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	/* Jump to handle function */
	bl handle_dabt
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/* IRQ handler */
irq:
	/* Modify the return address */
	sub lr, lr, #4
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	/* Jump to handle function */
	bl handle_irq
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/* FIQ handler */
fiq:
	/* Modify the return address */
	sub lr, lr, #4
	/* Save the content */
	stmfd sp!, {r0-r12, lr}
	/* Jump to handle function */
	bl handle_fiq
	/* Restore the content */
	ldmfd sp!, {r0-r12, pc}^

/*
 * END
 */
	.end
