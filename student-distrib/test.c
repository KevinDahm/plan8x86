#include "test.h"
#include "system_calls.h"
#include "page.h"
#include "lib.h"

#define BUF_SIZE 128

void test() {
    sys_write(1, "Hello\n", 7);

    switch_page_directory(1);

    int32_t *MB128 = KERNEL * 32;
    *MB128 = 5;
    printf("%x", *MB128);
}
