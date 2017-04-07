#ifndef PAGE_H
#define PAGE_H
#include "types.h"

#define DIR_SIZE 1024
#define NUM_DIRS 3 // Number of tasks + 1 for kernel
#define VIDEO   0x000B8000
#define KERNEL  0x00400000

typedef struct __attribute__((packed)) page_dir_kb_entry{
    uint32_t present : 1;
    uint32_t readWrite  : 1;
    uint32_t userSupervisor : 1;
    uint32_t writeThrough : 1;
    uint32_t cacheDisabled : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t pageSize : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t addr : 20;
} page_dir_kb_entry_t;

typedef struct __attribute__((packed)) page_dir_mb_entry{
    uint32_t present : 1;
    uint32_t readWrite  : 1;
    uint32_t userSupervisor : 1;
    uint32_t writeThrough : 1;
    uint32_t cacheDisabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pageSize : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t pgTblAttIdx : 1;
    uint32_t reserved : 9;
    uint32_t addr : 10;
} page_dir_mb_entry_t;

typedef struct __attribute__((packed)) page_table_kb_entry{
    uint32_t present : 1;
    uint32_t readWrite  : 1;
    uint32_t userSupervisor : 1;
    uint32_t writeThrough : 1;
    uint32_t cacheDisabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pgTblAttIdx : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t addr : 20;
} page_table_kb_entry_t;

uint32_t page_directory_tables[NUM_DIRS][DIR_SIZE] __attribute__((aligned (0x1000)));
uint32_t page_tables[NUM_DIRS][DIR_SIZE] __attribute__((aligned (0x1000)));

// Sets PG, PSE, and PE flags. Moves directory address to CR3
extern void init_paging();

extern void switch_page_directory(int pd);

// Clears directory and page tables
extern void clear_tables();

// Creates the basic entries for video memory and kernal in page tables
extern void create_entries();

#endif // PAGE_H
