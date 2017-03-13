#include "filesystem.h"

void do_exploration() {
    int32_t *x = fs_start + 1024;
    int i;
    for (i = 0; i < 2; i++) {
        printf("%d\n", x[i]);
    }
}
