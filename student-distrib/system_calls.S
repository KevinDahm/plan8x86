  .text

.globl halt
halt:
  pushl %ebp
  movl %esp, %ebp
  movl $1, %eax
  popl %ebx
  int $0x80
  leave
  ret

.globl execute
execute:
  pushl %ebp
  movl %esp, %ebp
  movl $2, %eax
  popl %ebx
  int $0x80
  leave
  ret

.globl read
read:
  pushl %ebp
  movl %esp, %ebp
  movl $3, %eax
  popl %ebx
  popl %ecx
  popl %edx
  int $0x80
  leave
  ret

.globl write
write:
  pushl %ebp
  movl %esp, %ebp
  movl $4, %eax
  popl %ebx
  popl %ecx
  popl %edx
  int $0x80
  leave
  ret

.globl open
open:
  pushl %ebp
  movl %esp, %ebp
  movl $5, %eax
  popl %ebx
  int $0x80
  leave
  ret

.globl close
close:
  pushl %ebp
  movl %esp, %ebp
  movl $6, %eax
  popl %ebx
  int $0x80
  leave
  ret

.globl getargs
getargs:
  pushl %ebp
  movl %esp, %ebp
  movl $7, %eax
  popl %ebx
  popl %ecx
  int $0x80
  leave
  ret

.globl vidmap
vidmap:
  pushl %ebp
  movl %esp, %ebp
  movl $8, %eax
  popl %ebx
  int $0x80
  leave
  ret

.globl set_handler
set_handler:
  pushl %ebp
  movl %esp, %ebp
  movl $9, %eax
  popl %ebx
  popl %ecx
  int $0x80
  leave
  ret

.globl sigreturn
sigreturn:
 	pushl %ebp
  movl %esp, %ebp
  movl $10, %eax
  int $0x80
  leave
  ret
