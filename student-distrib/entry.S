
 .text

#define SAVE_ALL \
  pushl %fs; \
  pushl %es; \
  pushl %ds; \
  pushl %eax; \
  pushl %ebp; \
  pushl %edi; \
  pushl %esi; \
  pushl %edx; \
  pushl %ecx; \
  pushl %ebx; \

#define RESTORE_ALL \
  popl %ebx; \
	popl %ecx; \
	popl %edx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	popl %ds; \
	popl %es; \
	popl %fs; \

common_interrupt:
  SAVE_ALL
	movl %esp, %ecx
	call do_IRQ
	jmp ret_from_intr

ret_from_intr:
	RESTORE_ALL
	iret


.globl div0
div0:
  pushl $0xFFFFFFFF
  jmp common_interrupt
