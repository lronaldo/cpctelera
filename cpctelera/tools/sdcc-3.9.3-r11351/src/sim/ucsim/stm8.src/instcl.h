
/* stm8.src/instcl.h */

   //virtual void incx(void);
   virtual int fetchea(t_mem code, unsigned char prefix);
   virtual int get_1(unsigned int addr);
   virtual int get2(unsigned int addr);
   virtual int get3(unsigned int addr);
   virtual int get_dest(t_mem code, unsigned char prefix);
   virtual int operandw(t_mem code, unsigned char prefix);
   virtual void flag_cvh(int x, int m, int r, bool byte, bool add, int mask);
  
   virtual int inst_adc(t_mem code, unsigned char prefix);
   virtual int inst_add(t_mem code, unsigned char prefix);
   virtual int inst_addw(t_mem code, unsigned char prefix);
   virtual int inst_and(t_mem code, unsigned char prefix);
   virtual int inst_bccmbcpl(t_mem code, unsigned char prefix);
   virtual int inst_bcp(t_mem code, unsigned char prefix);
   virtual int inst_bresbset(t_mem code, unsigned char prefix);
   virtual int inst_btjfbtjt(t_mem code, unsigned char prefix);
   virtual int inst_call(t_mem code, unsigned char prefix);
   virtual int inst_clr(t_mem code, unsigned char prefix);
   virtual int inst_cp(t_mem code, unsigned char prefix);
   virtual int inst_cpw(t_mem code, unsigned char prefix);
   virtual int inst_cpl(t_mem code, unsigned char prefix);
   virtual int inst_dec(t_mem code, unsigned char prefix);
   virtual int inst_div(t_mem code, unsigned char prefix);
   virtual int inst_inc(t_mem code, unsigned char prefix);
   virtual int inst_jp(t_mem code, unsigned char prefix);
   virtual int inst_jr(t_mem code, unsigned char prefix);
   virtual int inst_lda(t_mem code, unsigned char prefix);
   virtual int inst_ldxy(t_mem code, unsigned char prefix);
   virtual int inst_lddst(t_mem code, unsigned char prefix);
   virtual int inst_ldxydst(t_mem code, unsigned char prefix);
   virtual int inst_neg(t_mem code, unsigned char prefix);
   virtual int inst_or(t_mem code, unsigned char prefix);
   virtual int inst_rlc(t_mem code, unsigned char prefix);
   virtual int inst_rrc(t_mem code, unsigned char prefix);
   virtual int inst_sbc(t_mem code, unsigned char prefix);
   virtual int inst_sll(t_mem code, unsigned char prefix);
   virtual int inst_sra(t_mem code, unsigned char prefix);
   virtual int inst_srl(t_mem code, unsigned char prefix);
   virtual int inst_sub(t_mem code, unsigned char prefix);
   virtual int inst_swap(t_mem code, unsigned char prefix);
   virtual int inst_tnz(t_mem code, unsigned char prefix);
   virtual int inst_xor(t_mem code, unsigned char prefix);

/* End of stm8.src/instcl.h */
