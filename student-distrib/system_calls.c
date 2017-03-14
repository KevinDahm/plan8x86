#include "system_calls.h"
#include "filesystem.h"
#include "lib.h"

int32_t sys_halt(uint8_t status) {
    return 0;
}

int32_t sys_execute(const uint8_t* command) {
    return 0;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    if (file_descs[fd].flags == 0) {
        return -1;
    }
    return (*file_descs[fd].ops->read)(fd, buf, nbytes);
}

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    // TODO: EC?
    return 0;
}

int32_t sys_open(const int8_t* filename) {
    // TODO: stdio
    if (!strncmp(filename, "/dev/stdin", strlen("/dev/stdin"))) {
        file_descs[0].inode = NULL;
        file_descs[0].flags = 1;
        return 0;
    }
    if (!strncmp(filename, "/dev/stdout", strlen("/dev/stdout"))) {
        file_descs[1].inode = NULL;
        file_descs[1].flags = 1;
        return 1;
    }
    int i;
    for (i = 2; i < FILE_DESCS_LENGTH; i++) {
        if (file_descs[i].flags == 0) {
            break;
        }
    }
    if (i < FILE_DESCS_LENGTH) {
        file_descs[i].inode = (inode_t *)(*filesys_ops.open)(filename);
        if (file_descs[i].inode == NULL) {
            return -1;
        }
        file_descs[i].ops = &filesys_ops;
        file_descs[i].file_pos = 0;
        file_descs[i].flags = 1;

        return i;
    } else {
        return -1;
    }
}

int32_t sys_close(int32_t fd) {
    file_descs[fd].flags = 0;
    return 0;
}

int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    return 0;
}

int32_t sys_vidmap(uint8_t** screen_start) {
    return 0;
}

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    return 0;
}

int32_t sys_sigreturn(void) {
    return 0;
}

// TODO: processes
void system_calls_init() {
    sys_open("/dev/stdin");
    sys_open("/dev/stdout");
}
