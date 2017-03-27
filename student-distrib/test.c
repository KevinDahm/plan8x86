#include "test.h"
#include "system_calls.h"
#include "user_system_calls.h"


#define BUF_SIZE 128

void test() {
    int8_t buf[BUF_SIZE];

    while(1){
        read(0, buf, BUF_SIZE);
    }
}
