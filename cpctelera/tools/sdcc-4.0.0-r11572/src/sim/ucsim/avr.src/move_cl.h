/* avr.src/move_cl.h */

  virtual int lpm(t_mem code);
  virtual int elpm(t_mem code);
  virtual int spm(t_mem code);
  virtual int espm(t_mem code);
  virtual int ldi_Rd_K(t_mem code);
  virtual int movw_Rd_Rr(t_mem code);
  virtual int lds_Rd_k(t_mem code);
  virtual int ld_Rd_ZS(t_mem code);
  virtual int ld_Rd_SZ(t_mem code);
  virtual int lpm_Rd_Z(t_mem code);
  virtual int lpm_Rd_ZS(t_mem code);
  virtual int elpm_Rd_Z(t_mem code);
  virtual int elpm_Rd_ZS(t_mem code);
  virtual int ld_Rd_YS(t_mem code);
  virtual int ld_Rd_SY(t_mem code);
  virtual int ld_Rd_X(t_mem code);
  virtual int ld_Rd_XS(t_mem code);
  virtual int ld_Rd_SX(t_mem code);
  virtual int pop_Rd(t_mem code);
  virtual int sts_k_Rr(t_mem code);
  virtual int st_ZS_Rr(t_mem code);
  virtual int st_SZ_Rr(t_mem code);
  virtual int st_YS_Rr(t_mem code);
  virtual int st_SY_Rr(t_mem code);
  virtual int st_X_Rr(t_mem code);
  virtual int st_XS_Rr(t_mem code);
  virtual int st_SX_Rr(t_mem code);
  virtual int push_Rr(t_mem code);
  virtual int swap_Rd(t_mem code);
  virtual int ldd_Rd_Z_q(t_mem code);
  virtual int ldd_Rd_Y_q(t_mem code);
  virtual int std_Z_q_Rr(t_mem code);
  virtual int std_Y_q_Rr(t_mem code);
  virtual int in_Rd_A(t_mem code);
  virtual int out_A_Rr(t_mem code);
  virtual int mov_Rd_Rr(t_mem code);

/* End of avr.src/move_cl.h */
