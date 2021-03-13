/* pdk.src/instcl.h */

int get_mem(unsigned int addr);
unsigned char add_to(unsigned char initial, int value, bool carry = false);
unsigned char sub_to(unsigned char initial, int value, bool carry = false);
unsigned char get_io(t_addr addr);
void store_io(t_addr addr, unsigned char value);
unsigned char get_SP();
unsigned char get_flags();
void set_flags(unsigned char flags);

enum flag {
  flag_z,
  flag_c,
  flag_ac,
  flag_ov,
};
int get_flag(flag n);
void store_flag(flag n, int value);

int execute(unsigned int code);
int execute_pdk13(unsigned int code);
int execute_pdk14(unsigned int code);
int execute_pdk15(unsigned int code);

/* End of pdk.src/instcl.h */
