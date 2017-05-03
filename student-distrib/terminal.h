#ifndef TERMINAL_H
#define TERMINAL_H
#include "types.h"
#include "filesystem.h"
#include "task.h"
#include "kbd.h"

#define HIST_LENGTH 10
//Initialize stdin/stdout for use
extern void terminal_init();

//open system call for stdin/stdout
extern int32_t terminal_open(const int8_t* filename);

extern int32_t stdin_open(const int8_t* filename);

//close system call for stdin/stdout
extern int32_t terminal_close(int32_t fd);

//Write system call for stdout
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

//Write to stdin - stub function
extern int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes);

//Read system call for stdin
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

//Read from stdout - stub function
extern int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes);

extern void clear_hist();

//System calls for stdin/stdout
file_ops_t stdin_ops;
file_ops_t stdout_ops;

typedef struct hist {
    uint8_t history[HIST_LENGTH][KBD_BUFFER_SIZE];
    int32_t size[HIST_LENGTH];
    int32_t write_index;
    int32_t read_index;
    bool capped;
    bool completed;
} hist_t;

hist_t hist[NUM_TASKS];
#endif // TERMINAL_H
