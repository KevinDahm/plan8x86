#include "page.h"
#include "lib.h"

#define VIDEO   0x000B8000
#define KERNEL  0x00400000

/* init_paging
 * Description: Sets flags and moves directory address to CR3
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Paging is enabled
 */
void init_paging(){
    asm volatile("              \n\
    movl    %0, %%cr3           \n\
                                \n\
    movl    %%cr4, %%eax        \n\
    orl     $0x00000010, %%eax  \n\
    movl    %%eax, %%cr4        \n\
                                \n\
    movl    %%cr0, %%eax        \n\
    orl     $0x80000001, %%eax  \n\
    movl    %%eax, %%cr0        \n\
    "
    : /* no outputs */
    : "a"(page_directory_table)
    );
}


/* clear_tables
 * Description: Sets tables to write mode but not present
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Tables are set to empty
 */
void clear_tables() {
    memset(page_directory_table, 2, DIR_SIZE * 4);
    memset(page_table, 2, DIR_SIZE * 4);
}


/* create_entries
 * Description: Initializes kernel and video memory in paging tables
 * Inputs:      None
 * Outputs:     None
 * Ret Value:   None
 * Side Effect: Entries are made in the paging tables
 */
void create_entries() {
    page_dir_kb_entry_t* kb_dir_entry = (page_dir_kb_entry_t*)page_directory_table;
    kb_dir_entry->addr = (uint32_t)page_table >> 12;//Lose lower 12 bits (keep 20 high bits)
    kb_dir_entry->avail = 0;
    kb_dir_entry->global = 0;
    kb_dir_entry->pageSize = 0;     //0 for kb
    kb_dir_entry->reserved = 0;
    kb_dir_entry->accessed = 0;
    kb_dir_entry->cacheDisabled = 0;
    kb_dir_entry->writeThrough = 1; //1 for fun
    kb_dir_entry->userSupervisor = 0;
    kb_dir_entry->readWrite = 1;    //Write enabled
    kb_dir_entry->present = 1;

    page_table_kb_entry_t* table_entry =
        (page_table_kb_entry_t*)(page_table + (VIDEO >> 12));
    table_entry->addr = VIDEO >> 12;    //Lose lower 12 bits (keep 20 high bits)
    table_entry->avail = 0;
    table_entry->global = 0;
    table_entry->pgTblAttIdx = 0;
    table_entry->dirty = 0;
    table_entry->accessed = 0;
    table_entry->cacheDisabled = 0;
    table_entry->writeThrough = 1;  //1 for fun
    table_entry->userSupervisor = 0;
    table_entry->readWrite = 1;     //Write enabled
    table_entry->present = 1;

    page_dir_mb_entry_t* mb_dir_entry = (page_dir_mb_entry_t*)(page_directory_table+1);
    mb_dir_entry->addr = KERNEL >> 22;  //Lose lower 22 bits (keep 10 high bits)
    mb_dir_entry->reserved = 0;
    mb_dir_entry->pgTblAttIdx = 0;
    mb_dir_entry->avail = 0;
    mb_dir_entry->global = 1;       //1 for global buffer usage
    mb_dir_entry->pageSize = 1;     //1 for mb
    mb_dir_entry->reserved = 0;
    mb_dir_entry->accessed = 0;
    mb_dir_entry->cacheDisabled = 0;
    mb_dir_entry->writeThrough = 1; //1 or fun
    mb_dir_entry->userSupervisor = 0;
    mb_dir_entry->readWrite = 1;    //Write enabled
    mb_dir_entry->present = 1;

    /* printf("Entry 0: %x\n", page_directory_table[0]); */
    /* printf("Entry 0->0xB8: %x\n", page_table[0xB8]); */
    /* printf("Entry 1: %x\n", page_directory_table[1]); */
    /* printf("Entry 2: %x\n", page_directory_table[2]); */
}
