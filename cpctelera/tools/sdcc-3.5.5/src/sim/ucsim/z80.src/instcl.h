/* z80.src/instcl.h */

  virtual int inst_nop(t_mem code);
  virtual int inst_ld(t_mem code);
  virtual int inst_inc(t_mem code);
  virtual int inst_dec(t_mem code);
  virtual int inst_rlca(t_mem code);
  virtual int inst_rrca(t_mem code);
  virtual int inst_ex(t_mem code);
  virtual int inst_add(t_mem code);
  virtual int inst_djnz(t_mem code);
  virtual int inst_jr(t_mem code);
  virtual int inst_rla(t_mem code);
  virtual int inst_rra(t_mem code);
  virtual int inst_daa(t_mem code);
  virtual int inst_cpl(t_mem code);
  virtual int inst_scf(t_mem code);
  virtual int inst_ccf(t_mem code);
  virtual int inst_halt(t_mem code);
  virtual int inst_adc(t_mem code);
  virtual int inst_sbc(t_mem code);
  virtual int inst_and(t_mem code);
  virtual int inst_xor(t_mem code);
  virtual int inst_or(t_mem code);
  virtual int inst_cp(t_mem code);
  virtual int inst_rst(t_mem code);
  virtual int inst_ret(t_mem code);
  virtual int inst_call(t_mem code);
  virtual int inst_out(t_mem code);
  virtual int inst_push(t_mem code);
  virtual int inst_exx(t_mem code);
  virtual int inst_in(t_mem code);
  virtual int inst_sub(t_mem code);
  virtual int inst_pop(t_mem code);
  virtual int inst_jp(t_mem code);
  virtual int inst_di(t_mem code);
  virtual int inst_ei(t_mem code);

  virtual int inst_fd(void);
  virtual int inst_fd_ld(t_mem code);
  virtual int inst_fd_add(t_mem code);
  virtual int inst_fd_push(t_mem code);
  virtual int inst_fd_inc(t_mem code);
  virtual int inst_fd_dec(t_mem code);
  virtual int inst_fd_misc(t_mem code);

  virtual int inst_dd(void);
  virtual int inst_dd_ld(t_mem code);
  virtual int inst_dd_add(t_mem code);
  virtual int inst_dd_push(t_mem code);
  virtual int inst_dd_inc(t_mem code);
  virtual int inst_dd_dec(t_mem code);
  virtual int inst_dd_misc(t_mem code);

  virtual int inst_ed(void);
  virtual int inst_ed_(t_mem code);

  virtual int inst_cb(void);
  virtual int inst_cb_rlc(t_mem code);
  virtual int inst_cb_rrc(t_mem code);
  virtual int inst_cb_rl(t_mem code);
  virtual int inst_cb_rr(t_mem code);
  virtual int inst_cb_sla(t_mem code);
  virtual int inst_cb_sra(t_mem code);
  virtual int inst_cb_slia(t_mem code);
  virtual int inst_cb_srl(t_mem code);
  virtual int inst_cb_bit(t_mem code);
  virtual int inst_cb_res(t_mem code);
  virtual int inst_cb_set(t_mem code);

  virtual int inst_ddcb(void);
  virtual int inst_ddcb_rlc(t_mem code);
  virtual int inst_ddcb_rrc(t_mem code);
  virtual int inst_ddcb_rl(t_mem code);
  virtual int inst_ddcb_rr(t_mem code);
  virtual int inst_ddcb_sla(t_mem code);
  virtual int inst_ddcb_sra(t_mem code);
  virtual int inst_ddcb_slia(t_mem code);
  virtual int inst_ddcb_srl(t_mem code);
  virtual int inst_ddcb_bit(t_mem code);
  virtual int inst_ddcb_res(t_mem code);
  virtual int inst_ddcb_set(t_mem code);

  virtual int inst_fdcb(void);
  virtual int inst_fdcb_rlc(t_mem code);
  virtual int inst_fdcb_rrc(t_mem code);
  virtual int inst_fdcb_rl(t_mem code);
  virtual int inst_fdcb_rr(t_mem code);
  virtual int inst_fdcb_sla(t_mem code);
  virtual int inst_fdcb_sra(t_mem code);
  virtual int inst_fdcb_slia(t_mem code);
  virtual int inst_fdcb_srl(t_mem code);
  virtual int inst_fdcb_bit(t_mem code);
  virtual int inst_fdcb_res(t_mem code);
  virtual int inst_fdcb_set(t_mem code);

/* End of z80.src/instcl.h */
