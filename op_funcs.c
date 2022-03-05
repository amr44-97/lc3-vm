#ifndef OP_FUN
#define OP_FUN
#include "op_funcs.h"
#include <stdint.h>

static inline void add(uint16_t i);
static inline void and (uint16_t i);
static inline void br(uint16_t i);
static inline void ld(uint16_t i);
static inline void st(uint16_t i);
static inline void jsr(uint16_t i);
static inline void ldr(uint16_t i);
static inline void str(uint16_t i);
static inline void rti(uint16_t i);
static inline void not(uint16_t i);
static inline void ldi(uint16_t i);
static inline void sti(uint16_t i);
static inline void jmp(uint16_t i);
static inline void res(uint16_t i);
static inline void lea(uint16_t i);
static inline void trap(uint16_t i);

#endif
