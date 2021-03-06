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

typedef struct fstat {
    uint8_t type;
    uint32_t size;
}fstat_t;

//intializes the filesystem and its operations
extern void file_system_init(void* start, void* end);

//copies a files info to *dentry
extern int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);

//Copies a file information to *dentry
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

//reads file data into buf
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//reads a file name from a directory
extern int32_t read_dir_data(uint32_t index, uint8_t* buf, uint32_t length);

// reads data from a file
extern int32_t read_data_by_inode(inode_t* inode, uint32_t offset, uint8_t* buf, uint32_t length);

// gets index of a file in the directory
extern uint32_t get_index(const int8_t* fname);

//returns size of a file
extern uint32_t get_size(uint32_t inode_index);

//opens a file, by returning the inode
extern int32_t filesys_open(const int8_t* filename);

// closes a file
extern int32_t filesys_close(int32_t fd);

//reads nbytes from a file
extern int32_t filesys_read(int32_t fd, void* buf, int32_t nbytes);

// writes to the filesytem which does nothing
extern int32_t filesys_write(int32_t fd, const void* buf, int32_t nbytes);

// writes file stats to buf
extern int32_t filesys_stat(int32_t fd, void* buf, int32_t nbytes);

#endif
