#include "filesystem.h"
#include "lib.h"

static void* fs_start;
static void* fs_end;

static boot_block_t* boot_block;

void file_system_init(void* start, void* end) {
    fs_start = start;
    fs_end = end;
    boot_block = fs_start;

    filesys_ops.open = filesys_open;
    filesys_ops.close = filesys_close;
    filesys_ops.read = filesys_read;
    filesys_ops.write = filesys_write;
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

int32_t filesys_open(const int8_t* filename) {
    dentry_t dentry;
    // TODO: If file does not already exist create it? EC?
    if (read_dentry_by_name(filename, &dentry) == 0) {
        return (int32_t)(fs_start + ((dentry.inode + 1) * BLOCK_SIZE));
    } else {
        return NULL;
    }
}

int32_t filesys_close(int32_t fd) {
    return 0;
}

// TODO: Should this have access to file_descs?
int32_t filesys_read(int32_t fd, void* buf, int32_t nbytes) {
    return read_data_by_inode(file_descs[fd].inode, file_descs[fd].file_pos, (uint8_t*)buf, nbytes);
}

int32_t filesys_write(int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

int32_t read_data_by_inode(inode_t *inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t length_to_read = (inode->length - length) > 0 ? length : inode->length;
    if (offset > inode->length)
        return 0;

    uint32_t block_nums_index = offset / BLOCK_SIZE;
    uint32_t block_num = inode->block_nums[block_nums_index];

    if (block_num >= boot_block->num_data_blocks) {
        return 0;
    }
    block_t* curr_block =  fs_start + ((boot_block->num_inodes + 1) * BLOCK_SIZE) + (block_num * BLOCK_SIZE);
    uint32_t b = offset % BLOCK_SIZE;
    uint32_t i = 0;
    while (i < length_to_read) {
        buf[i] = curr_block->data[b];
        i++;
        b++;
        if (b >= BLOCK_SIZE && i < length_to_read) {
            b -= BLOCK_SIZE;
            block_nums_index++;
            block_num = inode->block_nums[block_nums_index];
            if (block_num >= boot_block->num_data_blocks) {
                return 0;
            }
            curr_block =  fs_start + ((boot_block->num_inodes + 1) * BLOCK_SIZE) + (block_num * BLOCK_SIZE);
        }
    }
    buf[length_to_read] = 0;
    return length_to_read;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    if (inode >= boot_block->num_inodes) {
        return -1;
    }
    inode_t* inode_block = fs_start + ((inode + 1) * BLOCK_SIZE);
    return read_data_by_inode(inode_block, offset, buf, length);
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
