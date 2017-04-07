#include "system_calls.h"
#include "filesystem.h"
#include "lib.h"
#include "rtc.h"
#include "kbd.h"
#include "terminal.h"
#include "page.h"
#include "x86_desc.h"

void *cleanup_ptr;

int32_t sys_halt(uint8_t status) {
    printf("BACK IN KERNEL");
    asm volatile(".3: hlt; jmp .3;");
    return 0;
}


int32_t sys_execute(const uint8_t* command) {
    // TODO: Parse command
    int32_t fd;
    int32_t ret = 0;
    if ((fd = sys_open(command)) == -1) {
        return -1;
    }

    // TODO: Don't always use task 1
    switch_page_directory(1);

    fstat_t stats;
    sys_stat(fd, &stats, sizeof(fstat_t));

    int8_t *buf = (int8_t*)0x08048000;

    sys_read(fd, (void*)buf, stats.size);

    if (buf[0] != 0x7F || buf[1] != 0x45 || buf[2] != 0x4C || buf[3] != 0x46) {
        // File is not executable
        ret = -1;

        sys_close(fd);
        return ret;
    }

    uint32_t start = ((uint32_t *)buf)[6];

    // Cast start to a pointer to a function returning int and taking void then call start
    // TODO: Set up TSS and iret into _start
    asm volatile("                      \n\
    cli                                 \n\
    movw $0x2B, %%ax                    \n\
    movw %%ax, %%ds                     \n\
    movw %%ax, %%es                     \n\
    movw %%ax, %%fs                     \n\
    movw %%ax, %%gs                     \n\
                                        \n\
    pushl $0x2B                         \n\
    pushl $0x08400000                   \n\
    pushf                               \n\
    popl %%eax                          \n\
    orl $0x200, %%eax                   \n\
    pushl %%eax                         \n\
    pushl $0x23                         \n\
    pushl %0                            \n\
    iret                                \n\
    "
                 :
                 : "b"(start)
        );

    printf("\nTHIS SHOULD NOT HAVE BEEN REACHED PLEASE REBOOT\n");
    asm volatile(".2: jmp .2;");
    return -1;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    switch (file_descs[fd].flags) {
    case FD_DIR:
    case FD_FILE:
    case FD_RTC:
    case FD_KBD:
    case FD_STDIN:
        return (*file_descs[fd].ops->read)(fd, buf, nbytes);
    default:
        return -1;
    }
}

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    switch (file_descs[fd].flags) {
    case FD_RTC:
    case FD_STDOUT:
        return (*file_descs[fd].ops->write)(fd, buf, nbytes);
    default:
        return -1;
    }
}

int32_t sys_open(const uint8_t* filename) {
    // TODO: stdio
    if (!strncmp((int8_t*)filename, "/dev/stdin", strlen("/dev/stdin"))) {
        file_descs[0].ops = &stdin_ops;
        file_descs[0].inode = NULL;
        file_descs[0].flags = FD_STDIN;
        return 0;
    }
    if (!strncmp((int8_t*)filename, "/dev/stdout", strlen("/dev/stdout"))) {
        file_descs[1].ops  = &stdout_ops;
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
        if (!strncmp((int8_t*)filename, "/dev/rtc", strlen("/dev/rtc"))) {
            file_descs[i].ops = &rtc_ops;
            file_descs[i].inode = NULL;
            file_descs[i].flags = FD_RTC;
            return i;
        }

        if (!strncmp((int8_t*)filename, "/dev/kbd", strlen("/dev/kbd"))) {
            file_descs[i].ops = &kbd_ops;
            file_descs[i].inode = NULL;
            file_descs[i].flags = FD_KBD;
            return i;
        }

        file_descs[i].inode = (*filesys_ops.open)((int8_t*)filename);
        if (file_descs[i].inode == -1) {
            return -1;
        }
        dentry_t d;
        if (read_dentry_by_name((int8_t*)filename, &d) == 0) {

            file_descs[i].ops = &filesys_ops;
            switch (d.type) {
            case 1:
                file_descs[i].file_pos = get_index((int8_t*)filename);
                file_descs[i].flags = FD_DIR;
                break;
            case 2:
                file_descs[i].file_pos = 0;
                file_descs[i].flags = FD_FILE;
                break;
            default:
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
    sys_open((uint8_t *)"/dev/stdin");
    sys_open((uint8_t *)"/dev/stdout");
}

int32_t sys_stat(int32_t fd, void* buf, int32_t nbytes) {
    switch (file_descs[fd].flags) {
    case FD_FILE:
    case FD_DIR:
        return (*file_descs[fd].ops->stat)(fd, buf, nbytes);
    default:
        return -1;
    }
}
