#include "filesystem.h"
#include "lib.h"
#include "task.h"

static void* fs_start;
static void* fs_end;

static boot_block_t* boot_block;

/* file_system_init
 * Description: Initializes the filesystem operations
 * Input:   start - pointer to the start of the filesystem
 *          end - pointer to the end of the filesystem
 * Output: none
 * Side Effects: initializes function pointers for system calls
 *               defines the range of the filesystem
 */
void file_system_init(void* start, void* end) {
    fs_start = start;
    fs_end = end;
    boot_block = fs_start;

    filesys_ops.open = filesys_open;
    filesys_ops.close = filesys_close;
    filesys_ops.read = filesys_read;
    filesys_ops.write = filesys_write;
    filesys_ops.stat = filesys_stat;
}

/* filesys_open
 * Description: Opens a file
 * Input:   filename - The name of the file to be opened
 * Return:  The inode for the file specified OR NULL for dir OR -1 for error
 */
int32_t filesys_open(const int8_t* filename) {
    dentry_t dentry;
    // TODO: If file does not already exist create it? EC?
    if (read_dentry_by_name(filename, &dentry) == 0) {
        switch (dentry.type) {
        case 2:
            return dentry.inode;
        case 1:
            return NULL;
        case 0:
        default:
            return -1;
        }
    } else return -1;
}

/* filesys_close
 * Description: Closes a file, currently does nothing
 * Input:   fd - File descriptor for file to be closed
 * Return:  0 for success
 */
int32_t filesys_close(int32_t fd) {
    return 0;
}

/* filesys_stat
 * Description: Gets statistics for the specified file
 * Input:   filename - The name of the file to be opened
 * Return:  The inode for the file specified OR NULL for dir OR -1 for error
 */
int32_t filesys_stat(int32_t fd, void* buf, int32_t nbytes){
    dentry_t e;
    switch(tasks[cur_task]->file_descs[fd].flags) {
    case FD_FILE:
        ((fstat_t*)buf)->type = 2;
        ((fstat_t*)buf)->size = get_size(tasks[cur_task]->file_descs[fd].inode);
        break;
    case FD_DIR:
        read_dentry_by_index(tasks[cur_task]->file_descs[fd].file_pos - 1, &e);
        ((fstat_t*)buf)->type = e.type;
        ((fstat_t*)buf)->size = get_size(e.inode);
        break;
    default:
        return -1;
    }
    return 0;
}

int32_t filesys_read(int32_t fd, void* buf, int32_t nbytes) {
    // Pronounced "red"
    int32_t read;
    switch (tasks[cur_task]->file_descs[fd].flags) {
    case FD_FILE:
        read = read_data(tasks[cur_task]->file_descs[fd].inode, tasks[cur_task]->file_descs[fd].file_pos, (uint8_t*)buf, nbytes);
        tasks[cur_task]->file_descs[fd].file_pos += read;
        break;
    case FD_DIR:
        read = read_dir_data(tasks[cur_task]->file_descs[fd].file_pos, (uint8_t*)buf, nbytes);
        tasks[cur_task]->file_descs[fd].file_pos++;
        break;
    default:
        return -1;
    }
    return read;
}

int32_t read_dir_data(uint32_t index, uint8_t* buf, uint32_t length) {
    if (index >= boot_block->num_dentries)
        return 0;
    uint32_t length_to_read = 32 > length ? length : 32;
    dentry_t d;
    read_dentry_by_index(index, &d);

    uint32_t i;
    for (i = 0; i < length_to_read; i++) {
        buf[i] = ((uint8_t*)d.name)[i];
    }
    return length_to_read;
}

int32_t filesys_write(int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

int32_t read_data_by_inode(inode_t *inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    if (offset > inode->length) {
        return 0;
    }
    uint32_t length_to_read = (inode->length - offset) > length ? length : (inode->length - offset);

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

uint32_t get_index(const int8_t* fname) {
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
            return i;
        }
    }
    return -1;
}

uint32_t get_size(uint32_t inode_index) {
    if (inode_index >= boot_block->num_inodes) {
        return 0;
    }
    inode_t* inode_block = fs_start + ((inode_index + 1) * BLOCK_SIZE);
    return inode_block->length;
}

int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry) {
    uint32_t i;
    if ((i = get_index(fname)) != -1)
        return read_dentry_by_index(i, dentry);
    else
        return -1;
}
