#define ASM 1
#include "page.h"

    .text

initPaging:
    # Set PG flag in CR0 to 1
    movl    %cr0, %eax
    orl     $0x80, %eax
    movl    %eax, %cr0

    # Set PSE flag in CR4 to 1
    movl    %cr4, %eax
    orl     $0x10, %eax
    movl    %eax, %cr4
