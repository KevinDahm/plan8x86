#include "filesystem.h"
#include "lib.h"

static void* fs_start;
static void* fs_end;

static boot_block_t* boot_block;

void file_system_init(void* start, void* end) {
    fs_start = start;
    fs_end = end;
    boot_block = fs_start;
}

void do_exploration() {
    printf("num_dentries: %d\n", boot_block->num_dentries);
    printf("num_inodes: %d\n", boot_block->num_inodes);
    printf("num_data_blocks: %d\n", boot_block->num_data_blocks);
    int i;
    for (i = 0; i < boot_block->num_dentries; i++) {
        printf("%s\n", boot_block->dentries[i].name);
    }
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    fs_start + ((inode + 1) * 4096);
}

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if (index < boot_block->num_dentries) {
        memcpy(dentry->name, boot_block->dentries[index].name, 32);
        dentry->type = boot_block->dentries[index].type;
        dentry->inode = boot_block->dentries[index].inode;
        memcpy(dentry->reserved, boot_block->dentries[index].reserved, 24);
        return 0;
    }
    return -1;
}

int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry) {
    uint32_t i;
    uint32_t j;
    uint32_t s1, s2;
    for (i = 0; i < boot_block->num_dentries; i++) {
        int8_t name[33];
        for (j = 0; j < 32; j++) {
            name[j] = boot_block->dentries[i].name[j];
        }
        name[32] = 0;
        s1 = strlen(name);
        s2 = strlen(fname);
        if (s1 != s2) {
            continue;
        }
        if (!strncmp(name, fname, s2)) {
            return read_dentry_by_index(i, dentry);
        }
    }
    return -1;
}
