#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// Gets the 5th bit of i
// We shift to the right with 5 bits so we can have bit on the last position
#define FIMM(i) ((i >> 5) & 1)
#define DR(i) (((i) >> 9) & 0x7)
#define SR1(i) (((i) >> 6) & 0x7)
#define SR2(i) ((i)&0x7)
#define IMM(i) ((i)&0x1F)
#define SEXTIMM(i) sext(IMM(i), 5)
#define POFF9(i) sext((i)&0x1FF, 9)

static inline uint16_t sext(uint16_t n, uint16_t b) {
  return ((n >> (b - 1)) & 1) ? (n | (0xFFFF << b)) : n;
}

uint16_t memory[UINT16_MAX + 1] = {0};
uint16_t PC_START = 0x3000;


static inline uint16_t memory_read(uint16_t address) { return memory[address]; }
static inline void memory_write(uint16_t address, uint16_t val) {
  memory[address] = val;
}

enum regist { R0 = 0, R1, R2, R3, R4, R5, R6, R7, RPC, RCND, RCNT };
uint16_t reg[RCNT] = {0};

#define OPCODE(i) ((i) >> 12)
#define INSTR_NUM (16)

typedef void (*op_ex_f)(uint16_t instruction);

enum flags { FP = 1 << 0, FZ = 1 << 1, FN = 1 << 2 };


static inline void uf(enum regist r) {
  if (reg[r] == 0)
    reg[RCND] = FZ;
  else if (reg[r] >> 15)
    reg[RCND] = FN;
  else
    reg[RCND] = FP;
}


/*
write your pointed to functions here
*/




static inline void add(uint16_t i) {
  reg[DR(i)] = reg[SR1(i)] + (FIMM(i) ? SEXTIMM(i) : reg[SR2(i)]);
  uf(DR(i));
};

static inline void and (uint16_t i) {
  reg[DR(i)] = reg[SR1(i)] & (FIMM(i) ? SEXTIMM(i) : reg[SR2(i)]);
  uf(DR(i));
};

static inline void ld(uint16_t i) {
  // opcode DRmem_address
  // [][][][] [][][][] [][][][] [][][][]

  reg[DR(i)] = memory_read(reg[RPC] + POFF9(i));
  uf(DR(i));
}

static inline void ldi(uint16_t i) {
  // https://www.youtube.com/watch?v=359TeV9UvM8

  reg[DR(i)] = memory_read(memory_read(reg[RPC] + POFF9(i)));
  uf(DR(i));
}

#define POFF6(i) sext((i)&0x3F,6)
//#define POFF10(i) ((i<<10)>>10)

static inline void ldr(uint16_t i) {
  reg[DR(i)] = memory_read(reg[DR(i)] +POFF6(i));
    uf(DR(i));
}

// [] [] [] [] | [] [] [] | [][][][][][][][][]
static inline void st(uint16_t i){
    memory_write(reg[RPC] + POFF9(i), reg[DR(i)]);
    //uf(DR(i)); no update flag that there is no computation it is copy paste
}

static inline void sti(uint16_t i){
    memory_write(memory_read(reg[RPC] + POFF9(i)),reg[DR(i)]);
    uf(DR(i));
}


// opcode + DR1 + base +   offset
//   4    +  3  + 3    +    6
//   store base+offset
static inline void str(uint16_t i){
    memory_write(reg[SR1(i)] + POFF6(i) , reg[DR(i)]);
    uf(DR(i));
}


// load memory address into program
// opcode(4) + DR1(3) + OFFSET9(9)

static inline void lea(uint16_t i){

  reg[DR(i)] = reg[RPC] + POFF9(i);

  uf(DR(i));
}

// opcode(4),DR1(3),SR1(3),111111
static inline void not(uint16_t i){
  reg[DR(i)] =~ reg[SR1(i)];
  uf(DR(i));
}

// opcode 4, 000 , baser 3 , 000000

#define BR(i) (((i)>>6)&0x7)
static inline void jmp(uint16_t i){

  reg[RPC] = reg[BR(i)];
}

#define FL(i) ((i>>11)&0x1)
#define POFF11(i) ((i)&0x7FF)
static inline void jsr(uint16_t i ){
  reg[R7] = reg[RPC];
  reg[RPC] = FL(i) ? reg[RPC] + POFF11(i) : reg[BR(i)];
}

// opcode 4, NZP 3, OFFSET9 9
#define FCND(i) (((i)>>9)&0x7)

static inline void br(uint16_t i){
  if (reg[RCND] & FCND(i)) { reg[RPC] = reg[RPC] + POFF9(i);}
}

#define TRPF(i) ((i)&0xFF)
enum { trp_offset = 0x20 };
typedef void (*trap_ex_func)();

static inline void tgetc()  { reg[R0]= getchar();}
static inline void tout()   { fprintf(stdout, "%c",(char) reg[R0]);}
static inline void tputs()  {
  uint16_t *p = memory + reg[R0];
    while(*p) {
        fprintf(stdout, "%c", (char)*p);
        p++;
    }
 }

bool running = true;
static inline void tputsp() { /* code */ }
static inline void tin()    {reg[R0] = getchar(); fprintf(stdout, "%c", reg[R0]);  }
static inline void thalt() { running = false;}
static inline void tinu16()  { fscanf(stdin, "%hu", &reg[R0]); }
static inline void toutu16() { fprintf(stdout, "%hu\n", reg[R0]); }

trap_ex_func trap_ex_f[8] ={tgetc,tout,tputs,tin,tputsp,thalt,tinu16,toutu16};



static inline void trap(uint16_t i){
  trap_ex_f[TRPF(i)-trp_offset]();
}

static inline void res(uint16_t i)
{

}
static inline void rti(uint16_t i)
{
}

op_ex_f op_exec[INSTR_NUM] = {br,  add, ld,  st,  jsr, and, ldr, str,
                           rti, not, ldi, sti, jmp, res, lea, trap};



void start(uint16_t offset){
  reg[RPC] = PC_START + offset;
  while(running){
    uint16_t i = memory_read(reg[RPC]++);
    op_exec[OPCODE(i)](i);
  }
}

void ld_img(char *fname, uint16_t offset) {
    // Open (binary) file containing the VM program
    FILE *in = fopen(fname, "rb");
    if (NULL==in) {
        fprintf(stderr, "Cannot open file %s.\n", fname);
        exit(1);
    }
    // The position from were we start copying the file
    // to the main memory
    uint16_t *p = memory + PC_START + offset;
    // Load the program in memory
    fread(p, sizeof(uint16_t), (UINT16_MAX-PC_START), in);
    // Close the file stream
    fclose(in);
}

int main(int argc, char **argv) {
    ld_img(argv[1], 0x0);
    start(0x0);
    return 0;
}
