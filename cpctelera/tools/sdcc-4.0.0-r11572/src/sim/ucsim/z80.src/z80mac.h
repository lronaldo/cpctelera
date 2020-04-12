/*
 * Simulator of microcontrollers (z80mac.h)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

#if 0
/* made into virtual function in z80_cl class to make integrating
 * banking and/or memory mapped devices easier
 *  -Leland Morrison 2011-09-29 
 */

#define store2(addr, val) { ram->write((t_addr) (addr), val & 0xff); \
                            ram->write((t_addr) (addr+1), (val >> 8) & 0xff); }
#define store1(addr, val) ram->write((t_addr) (addr), val)
#define get1(addr) ram->read((t_addr) (addr))
#define get2(addr) (ram->read((t_addr) (addr)) | (ram->read((t_addr) (addr+1)) << 8) )
#define fetch2() (fetch() | (fetch() << 8))
#define fetch1() fetch()
#endif

#define push2(val) {							\
    t_addr sp_before= regs.SP;						\
    regs.SP-=2;								\
    store2(regs.SP,(val));						\
    class cl_stack_op *so=						\
      new cl_stack_push(instPC,val,sp_before,regs.SP);			\
    so->init();								\
    stack_write(so);							\
  }
#define push1(val) {					\
    t_addr sp_before= regs.SP;				\
    regs.SP-=1;						\
    store1(regs.SP,(val));				\
    class cl_stack_op *so=				\
      new cl_stack_push(instPC,val,sp_before,regs.SP);	\
    so->init();						\
    stack_write(so);					\
  }
#define pop2(var) {var=get2(regs.SP),regs.SP+=2;}
//#define pop1(var) {var=get1(regs.SP),regs.SP+=1;}
#define add_u16_disp(_w, _d) (( (unsigned short)(_w) + (signed char)(_d) ) & 0xffff)
#define parity(val) ( ((val>>7)&1) ^ ((val>>6)&1) ^ ((val>>5)&1) ^ ((val>>4)&1) ^ ((val>>3)&1) ^ ((val>>2)&1) ^ ((val>>1)&1) ^ ((val>>0)&1) ^ 1 )

#define add_A_bytereg(br) {                                         \
   unsigned int accu = (unsigned int)regs.raf.A;                        \
   unsigned int oper = (unsigned int)(br);                          \
   signed int res = (signed char)regs.raf.A + (signed char)(br);        \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                         \
   regs.raf.F &= ~BIT_N;      /* addition */                            \
   if ((accu & 0x0F) + (oper & 0x0F) > 0x0F)  regs.raf.F |= BIT_A;      \
   if ((res < -128) || (res > 127))           regs.raf.F |= BIT_P;      \
   if (accu + oper > 0xFF)                    regs.raf.F |= BIT_C;      \
   regs.raf.A += oper;                                                  \
   if (regs.raf.A == 0)                           regs.raf.F |= BIT_Z;      \
   if (regs.raf.A & 0x80)                         regs.raf.F |= BIT_S;      \
}

#define adc_A_bytereg(br) {                                         \
   unsigned int accu = (unsigned int)regs.raf.A;                        \
   unsigned int oper = (unsigned int)(br);                          \
   signed int res = (signed char)regs.raf.A + (signed char)(br);        \
   if (regs.raf.F & BIT_C) { ++oper; ++res; }                           \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                         \
   regs.raf.F &= ~BIT_N;      /* addition */                            \
   if ((accu & 0x0F) + (oper & 0x0F) > 0x0F)  regs.raf.F |= BIT_A;      \
   if ((res < -128) || (res > 127))           regs.raf.F |= BIT_P;      \
   if (accu + oper > 0xFF)                    regs.raf.F |= BIT_C;      \
   regs.raf.A += oper;                                                  \
   if (regs.raf.A == 0)                           regs.raf.F |= BIT_Z;      \
   if (regs.raf.A & 0x80)                         regs.raf.F |= BIT_S;      \
}

#define add_HL_Word(wr) {                                           \
   unsigned int accu = (unsigned int)regs.HL;                       \
   unsigned int oper = (unsigned int)(wr);                          \
   regs.raf.F &= ~(BIT_A | BIT_C);  /* clear these */                   \
   regs.raf.F &= ~BIT_N;            /* addition */                      \
   if ((accu & 0x0FFF) + (oper & 0x0FFF) > 0x0FFF) regs.raf.F |= BIT_A; \
   if (accu + oper > 0xFFFF)                       regs.raf.F |= BIT_C; \
   regs.HL += oper;                                                 \
}

#define add_IX_Word(wr) {                                           \
   unsigned int accu = (unsigned int)regs_IX_OR_IY;                 \
   unsigned int oper = (unsigned int)(wr);                          \
   regs.raf.F &= ~(BIT_A | BIT_C);  /* clear these */                   \
   regs.raf.F &= ~BIT_N;            /* addition */                      \
   if ((accu & 0x0FFF) + (oper & 0x0FFF) > 0x0FFF) regs.raf.F |= BIT_A; \
   if (accu + oper > 0xFFFF)                       regs.raf.F |= BIT_C; \
   regs_IX_OR_IY += oper;                                           \
}

#define adc_HL_wordreg(reg) {                                       \
   unsigned int accu = (unsigned int)regs.HL;                       \
   unsigned int oper = (unsigned int)(reg);                         \
   signed int res = (signed short)regs.HL + (signed short)(reg);    \
   if (regs.raf.F & BIT_C) { ++oper; ++res; }                           \
   regs.raf.F &= ~(BIT_ALL);        /* clear these */                   \
   regs.raf.F &= ~BIT_N;            /* addition */                      \
   if ((accu & 0x0FFF) + (oper & 0x0FFF) > 0x0FFF) regs.raf.F |= BIT_A; \
   if ((res < -32768) || (res > 32767))            regs.raf.F |= BIT_P; \
   if (accu + oper > 0xFFFF)                       regs.raf.F |= BIT_C; \
   regs.HL += oper;                                                 \
   if (regs.HL == 0)                               regs.raf.F |= BIT_Z; \
   if (regs.HL & 0x8000)                           regs.raf.F |= BIT_S; \
}

#define sub_A_bytereg(br) {                                     \
   unsigned int accu = (unsigned int)regs.raf.A;                    \
   unsigned int oper = (unsigned int)(br);                      \
   signed int res = (signed char)regs.raf.A - (signed char)(br);    \
   regs.raf.F &= ~(BIT_ALL); /* clear these */                      \
   regs.raf.F |= BIT_N;      /* not addition */                     \
   if ((accu & 0x0F) < (oper & 0x0F))      regs.raf.F |= BIT_A;     \
   if ((res < -128) || (res > 127))        regs.raf.F |= BIT_P;     \
   if (accu < oper)                        regs.raf.F |= BIT_C;     \
   regs.raf.A -= oper;                                              \
   if (regs.raf.A == 0)                        regs.raf.F |= BIT_Z;     \
   if (regs.raf.A & 0x80)                      regs.raf.F |= BIT_S;     \
}

#define sbc_A_bytereg(br) {                                     \
   unsigned int accu = (unsigned int)regs.raf.A;                    \
   unsigned int oper = (unsigned int)(br);                      \
   signed int res = (signed char)regs.raf.A - (signed char)(br);    \
   if (regs.raf.F & BIT_C) { ++oper; --res; }                       \
   regs.raf.F &= ~(BIT_ALL); /* clear these */                      \
   regs.raf.F |= BIT_N;      /* not addition */                     \
   if ((accu & 0x0F) < (oper & 0x0F))      regs.raf.F |= BIT_A;     \
   if ((res < -128) || (res > 127))        regs.raf.F |= BIT_P;     \
   if (accu < oper)                        regs.raf.F |= BIT_C;     \
   regs.raf.A -= oper;                                              \
   if (regs.raf.A == 0)                        regs.raf.F |= BIT_Z;     \
   if (regs.raf.A & 0x80)                      regs.raf.F |= BIT_S;     \
}

#define sbc_HL_wordreg(reg) {                                   \
   unsigned int accu = (unsigned int)regs.HL;                   \
   unsigned int oper = (unsigned int)reg;                       \
   signed int res = (signed short)regs.HL - (signed short)(reg);\
   if (regs.raf.F & BIT_C) { ++oper; --res; }                       \
   regs.raf.F &= ~(BIT_ALL); /* clear these */                      \
   regs.raf.F |= BIT_N;      /* not addition */                     \
   if ((accu & 0x0FFF) < (oper & 0x0FFF))  regs.raf.F |= BIT_A;     \
   if ((res < -32768) || (res > 32767))    regs.raf.F |= BIT_P;     \
   if (accu < oper)                        regs.raf.F |= BIT_C;     \
   regs.HL -= oper;                                             \
   if (regs.HL == 0)                       regs.raf.F |= BIT_Z;     \
   if (regs.HL & 0x8000)                   regs.raf.F |= BIT_S;     \
}

#define cp_bytereg(br) {                                    \
   unsigned int accu = (unsigned int)regs.raf.A;                \
   unsigned int oper = (unsigned int)(br);                  \
   regs.raf.F &= ~(BIT_ALL); /* clear these */                  \
   regs.raf.F |= BIT_N;      /* not addition */                 \
   if ((accu & 0x0F) < (oper & 0x0F))  regs.raf.F |= BIT_A;     \
   if ((accu & 0x7F) < (oper & 0x7F))  regs.raf.F |= BIT_P;     \
   if (accu < oper) { regs.raf.F |= BIT_C; regs.raf.F ^= BIT_P; }   \
   accu -= oper;                                            \
   if (accu == 0)   regs.raf.F |= BIT_Z;                        \
   if (accu & 0x80) regs.raf.F |= BIT_S;                        \
}

#define rr_byte(reg) {                              \
   if (regs.raf.F & BIT_C) {                            \
     regs.raf.F &= ~(BIT_ALL);  /* clear these */       \
     if (reg & 0x01)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg >> 1) | 0x80;                       \
   } else {                                         \
     regs.raf.F &= ~(BIT_ALL);  /* clear these */       \
     if (reg & 0x01)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg >> 1);                              \
   }                                                \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define rrc_byte(reg) {                             \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */         \
   if (reg & 0x01) {                                \
     regs.raf.F |= BIT_C;                               \
     reg = (reg >> 1) | 0x80;                       \
   }                                                \
   else                                             \
     reg = (reg >> 1);                              \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define rl_byte(reg) {                              \
   if (regs.raf.F & BIT_C) {                            \
     regs.raf.F &= ~(BIT_ALL);  /* clear these */       \
     if (reg & 0x80)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg << 1) | 0x01;                       \
   } else {                                         \
     regs.raf.F &= ~(BIT_ALL);  /* clear these */       \
     if (reg & 0x80)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg << 1);                              \
   }                                                \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define rlc_byte(reg) {                             \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */         \
   if (reg & 0x80) {                                \
     regs.raf.F |= BIT_C;                               \
     reg = (reg << 1) | 0x01;                       \
   } else                                           \
     reg = (reg << 1);                              \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define sla_byte(reg) {                             \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */         \
   if (reg & 0x80)                                  \
     regs.raf.F |= BIT_C;                               \
   reg = (reg << 1);                                \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define sra_byte(reg) {                             \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */         \
   if (reg & 0x80) {                                \
     if (reg & 0x01)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg >> 1) | 0x80;                       \
   } else {                                         \
     if (reg & 0x01)                                \
       regs.raf.F |= BIT_C;                             \
     reg = (reg >> 1);                              \
   }                                                \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

#define srl_byte(reg) {                             \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */         \
   if (reg & 0x01)                                  \
     regs.raf.F |= BIT_C;                               \
   reg = (reg >> 1);                                \
   if (reg == 0)       regs.raf.F |= BIT_Z;             \
   if (reg & 0x80)     regs.raf.F |= BIT_S;             \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;             \
}

/* following not in my book, best guess based on z80.txt comments */
#define slia_byte(reg) { \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */ \
   if (reg & 0x80)      \
     regs.raf.F |= BIT_C;   \
   reg = (reg << 1) | 1; \
   if (reg == 0) regs.raf.F |= BIT_Z; \
   if (reg & 0x80) regs.raf.F |= BIT_S; \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P; \
}

#define and_A_bytereg(br) {                                 \
   regs.raf.A &= (br);                                          \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                 \
   if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;                     \
   if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;                     \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;                     \
}

#define xor_A_bytereg(br) {                                 \
   regs.raf.A ^= (br);                                          \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                 \
   if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;                     \
   if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;                     \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;                     \
}

#define or_A_bytereg(br) {                                  \
   regs.raf.A |= (br);                                          \
   regs.raf.F &= ~(BIT_ALL);  /* clear these */                 \
   if (regs.raf.A == 0)    regs.raf.F |= BIT_Z;                     \
   if (regs.raf.A & 0x80)  regs.raf.F |= BIT_S;                     \
   if (parity(regs.raf.A)) regs.raf.F |= BIT_P;                     \
}

#define inc(var) /* 8-bit increment */ { var++;                         \
   regs.raf.F &= ~(BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S);  /* clear these */   \
   if (var == 0)          regs.raf.F |= BIT_Z;                              \
   if (var == 0x80)       regs.raf.F |= BIT_P;                              \
   if (var & 0x80)        regs.raf.F |= BIT_S;                              \
   if ((var & 0x0f) == 0) regs.raf.F |= BIT_A;                              \
}

#define dec(var)  {                                                     \
   --var;                                                               \
   regs.raf.F &= ~(BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S);  /* clear these */   \
   regs.raf.F |= BIT_N;  /* Not add */                                      \
   if (var == 0)          regs.raf.F |= BIT_Z;                              \
   if (var == 0x7f)       regs.raf.F |= BIT_P;                              \
   if (var & 0x80)        regs.raf.F |= BIT_S;                              \
   if ((var & 0x0f) == 0) regs.raf.F |= BIT_A;                              \
}

#define bit_byte(reg, _bitnum) {                                        \
   regs.raf.F &= ~(BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S);  /* clear these */   \
   regs.raf.F |= BIT_A;                                                     \
   if (!(reg & (1 << (_bitnum))))                                       \
     regs.raf.F |= BIT_Z;                                                   \
   /* book shows BIT_S & BIT_P as unknown state */                      \
}
