  .data
unknown_string:
  .ascii "Unknown interrupt\0"
shell_str:
  .ascii "shell"
system_calls_jumptable:
  .long 0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn, sys_vidmap_all, sys_ioperm, sys_thread_create, sys_thread_join, sys_time

  .text

#define SAVE_ALL \
  cld; \
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

.globl execute_shell
execute_shell:
  movl $2, %eax
  movl $shell_str, %ebx
  xorl %ecx, %ecx
  xorl %edx, %edx
  int $0x80
  ret

.globl ignore_int
ignore_int:
  SAVE_ALL
  pushl $unknown_string
  call printf
  add $4, %esp
  RESTORE_ALL
  iret

common_interupt:
  SAVE_ALL
 	pushl %eax
  pushl %ecx
  pushl %edx
  pushl %esp
  add $12, (%esp)
  call backup_uesp
  addl $4, %esp
  popl %edx
  popl %ecx
  popl %eax
  movl %esp, %ecx
  call do_IRQ
  pushl %esp
  call check_for_signals
  addl $4, %esp
  RESTORE_ALL
  addl $8, %esp
  iret

common_exception:
  SAVE_ALL
  pushl %eax
  pushl %ecx
  pushl %edx
  pushl %esp
  add $12, (%esp)
  call backup_uesp
  addl $4, %esp
  popl %edx
  popl %ecx
  popl %eax
  movl 40(%esp), %eax
  movl 44(%esp), %ecx
  pushl %ecx
  pushl %esp
  addl $4, (%esp)
  call *%eax
  addl $8, %esp
  pushl %esp
  call check_for_signals
  addl $4, %esp
  RESTORE_ALL
  addl $8, %esp
  iret

.global system_call
system_call:
  pushl $0
  pushl $0
  SAVE_ALL
  pushl %eax
  pushl %ecx
  pushl %edx
  pushl %esp
  add $12, (%esp)
  call backup_uesp
  addl $4, %esp
  popl %edx
  popl %ecx
  popl %eax
  movl $0, %esi
  cmp %eax, %esi
  je sys_call_err
  movl $16, %esi
  cmp %esi, %eax
  ja sys_call_err
  pushl %edx
  pushl %ecx
  pushl %ebx
  call *system_calls_jumptable(, %eax, 4)
  addl $12, %esp
  movl %eax, 24(%esp) # Kludge to make sure the return value gets out
  pushl %esp
  call check_for_signals
  addl $4, %esp
  RESTORE_ALL
  addl $8, %esp
  iret

sys_call_err:
  movl $-1, 24(%esp)
  RESTORE_ALL
  addl $8, %esp
  iret


.globl irq_0x0
irq_0x0:
  pushl $0
  pushl $0
  SAVE_ALL
  pushl %esp
  call backup_uesp
  addl $4, %esp
  call schedule
  pushl %esp
  call check_for_signals
  addl $4, %esp
  RESTORE_ALL
  add $8, %esp
  iret

.globl irq_0x1
irq_0x1:
  pushl $0
  pushl $0xFFFFFFFE
  jmp common_interupt
.globl irq_0x2
irq_0x2:
  pushl $0
  pushl $0xFFFFFFFD
  jmp common_interupt
.globl irq_0x3
irq_0x3:
  pushl $0
  pushl $0xFFFFFFFC
  jmp common_interupt
.globl irq_0x4
irq_0x4:
  pushl $0
  pushl $0xFFFFFFFB
  jmp common_interupt
.globl irq_0x5
irq_0x5:
  pushl $0
  pushl $0xFFFFFFFA
  jmp common_interupt
.globl irq_0x6
irq_0x6:
  pushl $0
  pushl $0xFFFFFFF9
  jmp common_interupt
.globl irq_0x7
irq_0x7:
  pushl $0
  pushl $0xFFFFFFF8
  jmp common_interupt
.globl irq_0x8
irq_0x8:
  pushl $0
  pushl $0xFFFFFFF7
  jmp common_interupt
.globl irq_0x9
irq_0x9:
  pushl $0
  pushl $0xFFFFFFF6
  jmp common_interupt
.globl irq_0xA
irq_0xA:
  pushl $0
  pushl $0xFFFFFFF5
  jmp common_interupt
.globl irq_0xB
irq_0xB:
  pushl $0
  pushl $0xFFFFFFF4
  jmp common_interupt
.globl irq_0xC
irq_0xC:
  pushl $0
  pushl $0xFFFFFFF3
  jmp common_interupt
.globl irq_0xD
irq_0xD:
  pushl $0
  pushl $0xFFFFFFF2
  jmp common_interupt
.globl irq_0xE
irq_0xE:
  pushl $0
  pushl $0xFFFFFFF1
  jmp common_interupt
.globl irq_0xF
irq_0xF:
  pushl $0
  pushl $0xFFFFFFF0
  jmp common_interupt


.globl divide_error
divide_error:
  pushl $0
  pushl $do_divide_error
  jmp common_exception

.global debug
debug:
  pushl $0
  pushl $do_debug
  jmp common_exception

.global nmi
nmi:
  pushl $0
  pushl $do_nmi
  jmp common_exception

.global int3
int3:
  pushl $0
  pushl $do_int3
  jmp common_exception

.global overflow
overflow:
  pushl $0
  pushl $do_overflow
  jmp common_exception

.global bounds
bounds:
  pushl $0
  pushl $do_bounds
  jmp common_exception

.global invalid_op
invalid_op:
  pushl $0
  pushl $do_invalid_op
  jmp common_exception

.global device_not_available
device_not_available:
  pushl $0
  pushl $do_device_not_available
  jmp common_exception

.global double_fault
double_fault:
  pushl $do_double_fault
  jmp common_exception

.global coprocessor_segment_overrun
coprocessor_segment_overrun:
  pushl $0
  pushl $do_coprocessor_segment_overrun
  jmp common_exception

.global invalid_TSS
invalid_TSS:
  pushl $do_invalid_TSS
  jmp common_exception

.global segment_not_present
segment_not_present:
  pushl $do_segment_not_present
  jmp common_exception

.global stack_segment
stack_segment:
  pushl $do_stack_segment
  jmp common_exception

.global general_protection
general_protection:
  pushl $do_general_protection
  jmp common_exception

.global page_fault
page_fault:
  pushl $do_page_fault
  jmp common_exception

.global coprocessor_error
coprocessor_error:
  pushl $0
  pushl $do_coprocessor_error
  jmp common_exception

.global alignment_check
alignment_check:
  pushl $do_alignment_check
  jmp common_exception

.global machine_check
machine_check:
  pushl $0
  pushl $do_machine_check
  jmp common_exception

.global simd_coprocessor_error
simd_coprocessor_error:
  pushl $0
  pushl $do_simd_coprocessor_error
  jmp common_exception
