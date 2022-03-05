#ifndef OP_FUN
#define OP_FUN

#include <stdint.h>

// Gets the 5th bit of i
// We shift to the right with 5 bits so we can have bit on the last position
#define FIMM(i) ((i >> 5) & 1)
#define DR(i) (((i) >> 9) & 0x7)
#define SR1(i) (((i) >> 6) & 0x7)
#define SR2(i) ((i)&0x7)
#define IMM(i) ((i)&0x1F)

#define SEXTIMM(i) sext(IMM(i), 5)

static inline void add(uint16_t i);
static inline void and(uint16_t i);
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
