#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "types.h"

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

typedef struct inode {
    uint32_t length;
    
} inode_t;

extern void do_exploration();

extern void file_system_init(void* start, void* end);

extern int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
