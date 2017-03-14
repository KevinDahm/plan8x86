#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "types.h"

#define BLOCK_SIZE 4096

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
    uint32_t block_nums[BLOCK_SIZE / 4 - 1];
} inode_t;

typedef struct block {
    uint8_t data[BLOCK_SIZE];
} block_t;

extern void do_exploration();
extern void file_system_init(void* start, void* end);
extern int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern int32_t read_data_by_inode(inode_t* inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t filesys_open(const int8_t* filename);
extern int32_t filesys_close(int32_t fd);
extern int32_t filesys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t filesys_write(int32_t fd, const void* buf, int32_t nbytes);

typedef struct file_ops {
    int32_t (*open)(const int8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
} file_ops_t;

file_ops_t filesys_ops;

#define FD_CLEAR 0
#define FD_DIR 1
#define FD_FILE 2
#define FD_STDIN 3
#define FD_STDOUT 4
#define FD_RTC 5
#define FD_KBD 6

typedef struct file_desc {
    file_ops_t *ops;
    uint32_t inode;
    int32_t file_pos;
    int32_t flags;
} file_desc_t;

#define FILE_DESCS_LENGTH 8
// TODO: proccesses
file_desc_t file_descs[FILE_DESCS_LENGTH];

#endif
