#ifndef TERMINAL_H
#define TERMINAL_H
#include "types.h"
#include "filesystem.h"
#include "task.h"

//Initialize stdin/stdout for use
extern void terminal_init();

//open system call for stdin/stdout
extern int32_t terminal_open(const int8_t* filename);

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

//System calls for stdin/stdout
file_ops_t stdin_ops;
file_ops_t stdout_ops;


#endif // TERMINAL_H
