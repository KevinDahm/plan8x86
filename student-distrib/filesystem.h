#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "types.h"

void* fs_start;
void* fs_end;

typedef struct dentry {
    int8_t name[32];
    int32_t type;
    int32_t inode;
    int8_t reserved[24];
} dentry_t;

typedef struct boot_block {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    int8_t reserved[52];
    dentry_t dentries[63];
} boot_block_t;

extern void do_exploration();


#endif
