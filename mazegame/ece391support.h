#if !defined(ECE391SUPPORT_H)
#define ECE391SUPPORT_H

#include <stdint.h>
extern uint32_t strlen(const uint8_t* s);
extern void strcpy(uint8_t* dst, const uint8_t* src);
extern void fdputs(int32_t fd, const uint8_t* s);
extern int32_t strcmp(const uint8_t* s1, const uint8_t* s2);
extern int32_t strncmp(const uint8_t* s1, const uint8_t* s2, uint32_t n);
extern uint8_t *itoa(uint32_t value, uint8_t* buf, int32_t radix);
extern uint8_t *strrev(uint8_t* s);
extern void* memset(void* s, int32_t c, uint32_t n);
extern void* memset_word(void* s, int32_t c, uint32_t n);
extern void* memset_dword(void* s, int32_t c, uint32_t n);
extern void* memcpy(void* dest, const void* src, uint32_t n);
extern void* memmove(void* dest, const void* src, uint32_t n);

//extern int32_t snprintf(int8_t* buf, int length, int8_t *format, ...);
extern uint32_t get_time();
extern void srandom(uint32_t seed);
extern uint32_t random();

#define NULL 0

#endif /* ECE391SUPPORT_H */
