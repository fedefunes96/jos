/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(divide_zero, T_DIVIDE)
TRAPHANDLER_NOEC(debug, T_DEBUG)
TRAPHANDLER_NOEC(nonMaskableInterrupt, T_NMI)
TRAPHANDLER_NOEC(breakpoint_t, T_BRKPT)
TRAPHANDLER_NOEC(overflow, T_OFLOW)
TRAPHANDLER_NOEC(BOUNDRangeExceeded, T_BOUND)
TRAPHANDLER_NOEC(invalidOpcode, T_ILLOP)
TRAPHANDLER_NOEC(deviceNotAvailable, T_DEVICE)
TRAPHANDLER(doubleFault, T_DBLFLT)
TRAPHANDLER(invalidTSS, T_TSS)
TRAPHANDLER(segmentNotPresent, T_SEGNP)
TRAPHANDLER(stackFault, T_STACK)
TRAPHANDLER(generalProtection, T_GPFLT)
TRAPHANDLER(pageFault, T_PGFLT)
TRAPHANDLER_NOEC(x87FPUFloatingPointError, T_FPERR)
TRAPHANDLER(alignmentCheck, T_ALIGN)
TRAPHANDLER_NOEC(machineCheck, T_MCHK)
TRAPHANDLER_NOEC(SIMDFloatingPointException, T_SIMDERR)
TRAPHANDLER_NOEC(syscall_t, T_SYSCALL)
TRAPHANDLER_NOEC(irq_timer_t, IRQ_TIMER + IRQ_OFFSET)
TRAPHANDLER_NOEC(irq_kbd, IRQ_OFFSET+IRQ_KBD);
TRAPHANDLER_NOEC(irq_serial, IRQ_OFFSET+IRQ_SERIAL);
/*
 * Lab 3: Your code here for _alltraps
 */
.globl _alltraps;
_alltraps:
	pushl %ds
	pushl %es
	pushal
	mov $GD_KD, %ax
	mov %ax, %ds
	mov %ax, %es
	pushl %esp
	call trap
