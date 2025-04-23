typedef enum
  {
    SUB_MOS6502,
    SUB_MOS65C02
  }
MOS6502_SUB_PORT;

typedef struct
  {
    MOS6502_SUB_PORT sub;
  }
MOS6502_OPTS;

extern MOS6502_OPTS mos6502_opts;

#define IS_MOS6502 (mos6502_opts.sub == SUB_MOS6502)
#define IS_MOS65C02 (mos6502_opts.sub == SUB_MOS65C02)

