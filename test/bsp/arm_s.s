/*
 * ok6410/arm/arm_s.s
 */


/* Global functions */
	.global save_cpsr
	.global restore_cpsr
	.global start_task
	.global switch_task
	.global interrupt_switch_task
	.global irq_isr

/* Global variables */
	.global os_is_running
	.global os_prio_current
	.global os_prio_high_ready
	.global os_tcb_ready_ptr
	.global os_tcb_current_ptr
	.global os_interrupt_counter


/* Define global variables */
os_is_running:
	.word	0x00000000
os_prio_current:
	.word	0x00000000
os_prio_high_ready:
	.word	0x00000000
os_tcb_ready_ptr:
	.word	0x00000000
os_tcb_current_ptr:
	.word	0x00000000 
os_interrupt_counter:
	.word	0x00000000


	/* ARM mode */
	.equ	SYS_MODE, 0x1F
	.equ	IRQ_MODE, 0x12
	.equ	FIQ_MODE, 0x11

	/* Disable IRQ and FIQ */
	.equ	IRQ_DIS, 0x80
	.equ	FIQ_DIS, 0x40

/*
 *
 */
save_cpsr:
	/* Disable IRQ and FIQ */
	mrs r0, cpsr
	orr r1, r0, #(IRQ_DIS | FIQ_DIS)
	msr cpsr_c, r1

	/* Comfirm that we have disabled IRQ and FIQ */
	mrs r1, cpsr
	and r1, r1, #(IRQ_DIS | FIQ_DIS)
	cmp r1, #(IRQ_DIS | FIQ_DIS)
	/* If not disable, try again */
	bne save_cpsr

	/* Return the original CPSR in r0 */
	mov pc, lr

/*
 *
 */
restore_cpsr:
	msr cpsr_c, r0
	mov pc, lr


/*
 * Called from ok6410/kernel/core.c - start_os()
 */
start_task:
	/* Switch to SYS mode and disable IRQ and FIQ */
	msr cpsr_cxsf, #(SYS_MODE | IRQ_DIS | FIQ_DIS)

	/*
	 * Set the os_is_running to TRUE indicating that
	 * os start running once the first task is started
	 */
	ldr r4, =os_is_running
	mov r5, #1
	strb r5, [r4]

	/* Switch to highest priority task */
	ldr r4, =os_tcb_ready_ptr
	ldr r4, [r4]
	/* Get the stack pointer */
	ldr sp, [r4]

	/* POP the remaining registers of the task's */
	b restore_task

/*
 *
 */
switch_task:
	/* Save current task context */
	/* Save PC */
	str lr, [sp, #-4]!
	/* PUSH LR (R14, return address) */
	str lr, [sp, #-4]!
	str r12, [sp, #-4]!
	str r11, [sp, #-4]!
	str r10, [sp, #-4]!
	str r9, [sp, #-4]!
	str r8, [sp, #-4]!
	str r7, [sp, #-4]!
	str r6, [sp, #-4]!
	str r5, [sp, #-4]!
	str r4, [sp, #-4]!
	str r3, [sp, #-4]!
	str r2, [sp, #-4]!
	str r1, [sp, #-4]!
	str r0, [sp, #-4]!
	/* PUSH cpsr */
	mrs r4, cpsr
	str r4, [sp, #-4]!


	/* os_prio_current = os_prio_high_ready */
	ldr r4, =os_prio_current
	ldr r5, =os_prio_high_ready
	ldrb r6, [r5]
	strb r6, [r4]

	/* Save the current task stack pointer (SP) */
	/* os_tcb_current_ptr->tcb_stk_ptr = SP */
	ldr r4, =os_tcb_current_ptr
	ldr r5, [r4]
	str sp, [r5]

	/* os_tcb_current_ptr = os_tcb_ready_ptr */
	ldr r6, =os_tcb_ready_ptr
	ldr r6, [r6]
	str r6, [r4]

	/* SP = os_tcb_ready_ptr->tcb_stk_ptr */
	ldr sp, [r6]

	/* now switch to the new task's context */
	b restore_task

/*
 *
 */
interrupt_switch_task:
	/* os_prio_current = os_prio_high_ready */
	ldr r4, =os_prio_current
	ldr r5, =os_prio_high_ready
	ldrb r6, [r5]
	strb r6, [r4]

	/* os_tcb_current_tr = os_tcb_ready_ptr */
	ldr r4, =os_tcb_current_ptr
	ldr r6, =os_tcb_ready_ptr
	ldr r6, [r6]
	str r6, [r4]

	/* SP = os_tcb_ready_tr->tcb_stk_ptr */
	ldr sp, [r6]

	/* now switch to the new task's context */
	b restore_task

/*
 *
 */
irq_isr:
	/* Now we are in the IRQ mode, use IRQ SP */

	/* PUSH working registers */
	str r3, [sp, #-4]!
	str r2, [sp, #-4]!
	str r1, [sp, #-4]!

	/* Save IRQ stack pointer: r1 = sp */
	mov r1, sp
	/* Adjust IRQ SP back to start */
	add sp, sp, #12

	/* Modify the return address */
	sub r2, lr, #4

	/* Copy SPSR to R3 */
	mrs r3, spsr

	/* Change to SYS mode (disable IRQ and FIQ) */
	msr cpsr_c, #(SYS_MODE | IRQ_DIS | FIQ_DIS)

	/* Now we are in the SYS mode and use SYS SP */

	/* Save context */
	/* Return address PC */
	str r2, [sp, #-4]!
	/* LR */
	str lr, [sp, #-4]!
	str r12, [sp, #-4]!
	str r11, [sp, #-4]!
	str r10, [sp, #-4]!
	str r9, [sp, #-4]!
	str r8, [sp, #-4]!
	str r7, [sp, #-4]!
	str r6, [sp, #-4]!
	str r5, [sp, #-4]!
	str r4, [sp, #-4]!

	/* move r1, r2, r3 (form IRQ) to SYS */
	ldr r4, [r1], #4
	ldr r5, [r1], #4
	ldr r6, [r1], #4
	/* Save r3 */
	str r6, [sp, #-4]!
	/* R2 */
	str r5, [sp, #-4]!
	/* R1 */
	str r4, [sp, #-4]!

	str r0, [sp, #-4]!
	/* PUSH CPSR */
	str r3, [sp, #-4]!

	/* Increase the interrupt counter: ++os_interrupt_counter */
	/* We can call enter_interrupt() here */
	ldr r0, =os_interrupt_counter
	ldrb r1, [r0]
	add r1, r1, #1
	strb r1, [r0]

	/*
	 * if (os_interrupt_counter == 1)
	 *	os_tcb_current_ptr->tcb_stk_ptr = SP
	 */
	cmp r1, #1
	bne irq_isr_continue

	ldr r4, =os_tcb_current_ptr
	ldr r5, [r4]
	str sp, [r5]

irq_isr_continue:
	/* Change back to IRQ mode */
	msr cpsr_c, #(IRQ_MODE | IRQ_DIS | FIQ_DIS)

	/* Go to IRQ handler */
	bl handle_irq

	/* Finish irq and change to SYS mode again */
	msr cpsr_c, #(SYS_MODE | IRQ_DIS | FIQ_DIS)

	/* After handler, exit interrupt */
	bl exit_interrupt

	/*
	 * If we get here, os_interrupt_counter > 0 or
	 * os_prio_high_ready == os_prio_current
	 * so we restore the content and return to the task
	 */
	b restore_task

/*
 * static - just called in this file
 */
restore_task:
	/* POP CPSR */
	ldr r4, [sp], #4
	msr cpsr_cxsf, r4
	/* POP r0 (point to task argument) */
	ldr r0, [sp], #4
	/* POP r1 */
	ldr r1, [sp], #4
	ldr r2, [sp], #4
	ldr r3, [sp], #4
	ldr r4, [sp], #4
	ldr r5, [sp], #4
	ldr r6, [sp], #4
	ldr r7, [sp], #4
	ldr r8, [sp], #4
	ldr r9, [sp], #4
	ldr r10, [sp], #4
	ldr r11, [sp], #4
	ldr r12, [sp], #4
	/* LR (R14) */
	ldr lr, [sp], #4
	/*
	 * PC (R15), task entry point, after pop to PC,
	 * we begin running the new task.
	 */
	ldr pc, [sp], #4
