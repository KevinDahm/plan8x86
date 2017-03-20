#ifndef TERMINAL_H
#define TERMINAL_H
#include "types.h"
#include "filesystem.h"

extern int32_t terminal_open(const int8_t* filename);
extern int32_t terminal_close(int32_t fd);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes);
file_ops_t stdin_ops;
file_ops_t stdout_ops;


#endif // TERMINAL_H
