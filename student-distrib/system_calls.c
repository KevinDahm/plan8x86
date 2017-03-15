#include "system_calls.h"
#include "filesystem.h"
#include "lib.h"
#include "rtc.h"
#include "kbd.h"

int32_t sys_halt(uint8_t status) {
    return 0;
}

int32_t sys_execute(const uint8_t* command) {
    return 0;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    switch (file_descs[fd].flags) {
        case FD_CLEAR:
            return -1;
        case FD_DIR:
            return -1;
        case FD_FILE:
        case FD_RTC:
        case FD_KBD:
            /* printf("READING: %d", file_descs[fd].flags); */
            return (*file_descs[fd].ops->read)(fd, buf, nbytes);
        default:
            return -1;
    }
}

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    switch (file_descs[fd].flags) {
        case FD_RTC:
            (*file_descs[fd].ops->write)(fd, buf, nbytes);
        default:
            return -1;
    }
}

int32_t sys_open(const int8_t* filename) {
    // TODO: stdio
    if (!strncmp(filename, "/dev/stdin", strlen("/dev/stdin"))) {
        file_descs[0].inode = NULL;
        file_descs[0].flags = FD_STDIN;
        return 0;
    }
    if (!strncmp(filename, "/dev/stdout", strlen("/dev/stdout"))) {
        file_descs[1].inode = NULL;
        file_descs[1].flags = FD_STDOUT;
        return 1;
    }
    int i;
    for (i = 2; i < FILE_DESCS_LENGTH; i++) {
        if (file_descs[i].flags == 0) {
            break;
        }
    }
    if (i < FILE_DESCS_LENGTH) {
        if (!strncmp(filename, "/dev/rtc", strlen("/dev/rtc"))) {
            file_descs[i].ops = &rtc_ops;
            file_descs[i].inode = NULL;
            file_descs[i].flags = FD_RTC;
            return i;
        }

        if (!strncmp(filename, "/dev/kbd", strlen("/dev/kbd"))) {
            file_descs[i].ops = &kbd_ops;
            file_descs[i].inode = NULL;
            file_descs[i].flags = FD_KBD;
            return i;
        }

        file_descs[i].inode = (*filesys_ops.open)(filename);
        if (file_descs[i].inode == NULL) {
            return -1;
        }
        dentry_t d;
        if (!read_dentry_by_name(filename, &d)) {
            if (d.type == 1) {
                // TODO: ops for directories
                file_descs[i].file_pos = 0;
                file_descs[i].flags = FD_DIR;
            } else if (d.type == 2) {
                file_descs[i].ops = &filesys_ops;
                file_descs[i].file_pos = 0;
                file_descs[i].flags = FD_FILE;
            } else {
                return -1;
            }
        } else {
            return -1;
        }

        return i;
    } else {
        return -1;
    }
}

int32_t sys_close(int32_t fd) {
    if (fd >= FILE_DESCS_LENGTH || fd < 2)
        return -1;
    file_descs[fd].flags = 0;
    return (*file_descs[fd].ops->close)(fd);
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
