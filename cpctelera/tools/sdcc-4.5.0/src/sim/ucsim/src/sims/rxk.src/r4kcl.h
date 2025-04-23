/*
 * Simulator of microcontrollers (r4kcl.h)
 *
 * Copyright (C) 2020 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#ifndef R4KCL_HEADER
#define R4KCL_HEADER

#include "r3kacl.h"
#include "dp0m4.h"
#include "dpedm4.h"
#include "dpddm4.h"


extern u32_t px8(u32_t px, u8_t offset);
extern u32_t px8se(u32_t px, u8_t offset);
extern u32_t px16(u32_t px, u16_t offset);
extern u32_t px16se(u32_t px, u16_t offset);

//#define raJ	(aJK.r.J)
//#define raK	(aJK.r.K)
//#define raJK	(aJK.JK)
//#define raJKHL	(((u32_t)aJK.JK<<16)+aHL.HL)
//#define raBCDE	(((u32_t)aBC.BC<<16)+aDE.DE)

class cl_r4k: public cl_r3ka
{
public:
  RP(aJK,JK,J,K);
  u32_t rPW, rPX, rPY, rPZ;
  u32_t raPW, raPX, raPY, raPZ;
  u8_t rHTR;
  class cl_cell8 cJ, caJ, cK, caK;
  class cl_cell16 cJK, caJK;
  class cl_cell32 cJKHL, cPW, cPX, cPY, cPZ;
  class cl_cell32 caJKHL, caPW, caPX, caPY, caPZ;
  class cl_cell8 cHTR;
  class cl_cell16 *LXPC;
  class cl_cell32 *cIRR, *caIRR;
 public:
  cl_r4k(class cl_sim *asim);
  virtual int init();
  virtual const char *id_string(void);
  virtual void reset(void);
  
  virtual void make_cpu_hw(void);
  virtual t_addr chip_size() { return 0x1000000; }

  virtual struct dis_entry *dis_entry(t_addr addr);
  virtual struct dis_entry *dis_6d_entry(t_addr addr);
  virtual int longest_inst(void) { return 6; }
  virtual void disass_irr(chars *work, bool dd);
  virtual void disass_irrl(chars *work, bool dd);
  
  virtual void select_IRR(bool dd);
  virtual class cl_cell32 *destIRR(void) { return altd?caIRR:cIRR; }
  virtual class cl_cell32 &destPW(void) { return altd?caPW:cPW; }
  virtual class cl_cell32 &destPX(void) { return altd?caPX:cPX; }
  virtual class cl_cell32 &destPY(void) { return altd?caPY:cPY; }
  virtual class cl_cell32 &destPZ(void) { return altd?caPZ:cPZ; }
  virtual class cl_cell32 &destBCDE(void) { return altd?caBCDE:cBCDE; }
  virtual class cl_cell32 &destJKHL(void) { return altd?caJKHL:cJKHL; }
  virtual class cl_cell16 &destJK(void) { return altd?caJK:cJK; }
  
  virtual void print_regs(class cl_console_base *con);

  virtual int convc_pp(class cl_cell32 &pp);
  virtual int convd_pp(class cl_cell32 &pp);
  
  // move
  virtual int ld_pd_ihtr_hl(class cl_cell32 &dest);
  virtual int ld_irr_iird(class cl_cell16 &ir);			// 1f,14t,0w,4r
  virtual int ld_iird_irr(class cl_cell16 &ir);			// 1f,19t,4w,0r
  virtual int ld_irr_ips_hl(t_mem code);			// 0f,14t,4r,0w
  virtual int ld_irr_ips_d(t_mem code);				// 1f,14t,4r,0w
  virtual int ld_ips_hl_irr(t_mem code);			// 0f,18t,0r,4w
  virtual int ld_ips_d_irr(t_mem code);				// 1f,19t,0r,4w
  virtual int ldl_px_ir(t_mem code);				// 0f,4t,0r,0w
  virtual int ldl_px_irrl(t_mem code);				// 0f,4t,0r,0w
  virtual int ld_px_irr(t_mem code);				// 0f,4t,0r,0w
  virtual int ld_irr_px(t_mem code);				// 0f,4t,0r,0w
  virtual int ld32_imn(u32_t op);				// 2f,19t,0r,4w
  virtual int ld_r32_imn(class cl_cell32 &dest);		// 2f,15t,4r,0w
  virtual int ld_hl_ipsd(u32_t ps);				// 1f,9t,2r,0w
  virtual int ld_ipsd_hl(u32_t ps);				// 1f,10t,0r,2w
  virtual int ld_a_ipshl(u32_t ps);				// 0f,6t,1r,0w
  virtual int ld_ipshl_a(u32_t ps);				// 0f,7t,0r,1w
  virtual int ld_a_ipsd(u32_t ps);				// 1f,6t,1r,0w
  virtual int ld_ipsd_a(u32_t ps);				// 1f,8t,0r,1w
  virtual int ldl_pd_ispn(class cl_cell32 &pd);			// 1f,11t,2r,0w
  virtual int ld_pd_ispn(class cl_cell32 &pd);			// 1f,15t,4r,0w
  virtual int ld_ispn_ps(u32_t ps);				// 1f,19t,0r,4w
  virtual int ld_hl_ipsbc(u32_t ps);				// 0f,10t,2r,0w
  virtual int ld_ipdbc_hl(u32_t pd);				// 0f,12t,0r,2w
  virtual int ldf_pd_ilmn(class cl_cell32 &pd);			// 3f,19t,4r,0w
  virtual int ldf_ilmn_ps(u32_t ps);				// 3f,23t,0r,4w
  virtual int ldf_rr_ilmn(class cl_cell16 &rr);			// 3f,15t,2r,0w
  virtual int ldf_ilmn_rr(u16_t rr);				// 3f,17t,0r,2w
  virtual int ld_pd_klmn(class cl_cell32 &pd);			// 4f,12t,0r,0w
  virtual int ldl_pd_mn(class cl_cell32 &pd);			// 2f,8t,0r,0w
  virtual int copy(int dir);					// 0f,5t,xr,xw
  virtual int pop_pd(class cl_cell32 &pd);			// 0f,13t,4r,0w
  virtual int push_ps(u32_t ps);				// 0f,18t,0r,4w
  
  // arith
  virtual int subhl(class cl_cell16 &dest, u16_t op);
  virtual int test8(u8_t op);					// 0f,2t,0w,0r
  virtual int test16(u16_t op);					// 0f,2t,0w,0r
  virtual int test32(u32_t op);					// 0f,2t,0w,0r
  virtual int flag_cc_hl(t_mem code);				// 0f,4t,0w,0r
  
  // branch
  virtual int lljp_cx(t_mem code);				// 4f,14t,0w,0r
  virtual int lljp_cc(t_mem code);				// 4f,14t,0w,0r
  virtual int jre_cx_cc(bool cond);				// 2f,9t,0w,0r
  virtual int jr_cx_e(bool cond);				// 1f,4t,0w,0r
  virtual int jp_cx_mn(bool cond);				// 2f,7t,0w,0r
  
  virtual void mode3k(void);
  virtual void mode4k(void);

  virtual int EXX(t_mem code);

  // Page 0, m4 mode
  virtual int RL_HL(t_mem code) { return rot17left(destHL(), rHL); }
  virtual int RL_BC(t_mem code) { return rot17left(destBC(), rBC); }
  virtual int SUB_HL_JK(t_mem code) { return subhl(destHL(), rJK); }
  virtual int SUB_HL_DE(t_mem code) { return subhl(destHL(), rDE); }
  virtual int TEST_HL(t_mem code) { return test16(rHL); }
  virtual int CP_HL_D(t_mem code);
  virtual int RLC_BC(t_mem code) { return rot16left(destBC(), rBC); }
  virtual int RLC_DE(t_mem code) { return rot16left(destDE(), rDE); }
  virtual int RRC_BC(t_mem code) { return rot16right(destBC(), rBC); }
  virtual int RRC_DE(t_mem code) { return rot16right(destDE(), rDE); }
  virtual int XOR_HL_DE(t_mem code) { return xor16(destHL(), rHL, rDE); }
  virtual int RR_BC(t_mem code) { return rot17right(destBC(), rBC); }
  virtual int ADD_HL_JK(t_mem code) { return add_hl_ss(rJK); }
  virtual int LD_HL_BC(t_mem code) { return ld_hl_op(rBC); }
  virtual int LD_BC_HL(t_mem code);
  virtual int LD_HL_DE(t_mem code) { return ld_hl_op(rDE); }
  virtual int LDF_iLMN_HL(t_mem code);
  virtual int LDF_HL_iLMN(t_mem code);
  virtual int LD_iMN_BCDE(t_mem code) { return ld32_imn(rBCDE); }
  virtual int LD_iMN_JKHL(t_mem code) { return ld32_imn(rJKHL); }
  virtual int LD_BCDE_iMN(t_mem code) { return ld_r32_imn(destBCDE()); }
  virtual int LD_JKHL_iMN(t_mem code) { return ld_r32_imn(destJKHL()); }
  virtual int LD_HL_iPWd(t_mem code) { return ld_hl_ipsd(rPW); }
  virtual int LD_HL_iPXd(t_mem code) { return ld_hl_ipsd(rPX); }
  virtual int LD_HL_iPYd(t_mem code) { return ld_hl_ipsd(rPY); }
  virtual int LD_HL_iPZd(t_mem code) { return ld_hl_ipsd(rPZ); }
  virtual int LD_iPWd_HL(t_mem code) { return ld_ipsd_hl(rPW); }
  virtual int LD_iPXd_HL(t_mem code) { return ld_ipsd_hl(rPX); }
  virtual int LD_iPYd_HL(t_mem code) { return ld_ipsd_hl(rPY); }
  virtual int LD_iPZd_HL(t_mem code) { return ld_ipsd_hl(rPZ); }
  virtual int LLJP_lxpcmn(t_mem code);
  virtual int LD_imn_JK(t_mem code);
  virtual int LD_JK_imn(t_mem code);
  virtual int LDF_ilmn_A(t_mem code);
  virtual int LDF_A_ilmn(t_mem code);
  virtual int LD_A_iPWHL(t_mem code) { return ld_a_ipshl(rPW); }
  virtual int LD_A_iPXHL(t_mem code) { return ld_a_ipshl(rPX); }
  virtual int LD_A_iPYHL(t_mem code) { return ld_a_ipshl(rPY); }
  virtual int LD_A_iPZHL(t_mem code) { return ld_a_ipshl(rPZ); }
  virtual int LD_iPWHL_HL(t_mem code) { return ld_ipshl_a(rPW); }
  virtual int LD_iPXHL_HL(t_mem code) { return ld_ipshl_a(rPX); }
  virtual int LD_iPYHL_HL(t_mem code) { return ld_ipshl_a(rPY); }
  virtual int LD_iPZHL_HL(t_mem code) { return ld_ipshl_a(rPZ); }
  virtual int LD_A_iPWd(t_mem code) { return ld_a_ipsd(rPW); }
  virtual int LD_A_iPXd(t_mem code) { return ld_a_ipsd(rPX); }
  virtual int LD_A_iPYd(t_mem code) { return ld_a_ipsd(rPY); }
  virtual int LD_A_iPZd(t_mem code) { return ld_a_ipsd(rPZ); }
  virtual int LD_iPWd_A(t_mem code) { return ld_ipsd_a(rPW); }
  virtual int LD_iPXd_A(t_mem code) { return ld_ipsd_a(rPX); }
  virtual int LD_iPYd_A(t_mem code) { return ld_ipsd_a(rPY); }
  virtual int LD_iPZd_A(t_mem code) { return ld_ipsd_a(rPZ); }
  virtual int LLCALL_lxpcmn(t_mem code);
  virtual int LD_LXPC_HL(t_mem code);
  virtual int LD_HL_LXPC(t_mem code);
  virtual int JRE_ee(t_mem code);
  virtual int JR_GT_e(t_mem code) { return jr_cx_e(cond_GT(rF)); }
  virtual int JR_LT_e(t_mem code) { return jr_cx_e(cond_LT(rF)); }
  virtual int JR_GTU_e(t_mem code) { return jr_cx_e(cond_GTU(rF)); }
  virtual int JR_V_e(t_mem code) { return jr_cx_e(cond_V(rF)); }
  virtual int JP_GT_mn(t_mem code) { return jp_cx_mn(cond_GT(rF)); }
  virtual int JP_LT_mn(t_mem code) { return jp_cx_mn(cond_LT(rF)); }
  virtual int JP_GTU_mn(t_mem code) { return jp_cx_mn(cond_GTU(rF)); }
  virtual int JP_V_mn(t_mem code) { return jp_cx_mn(cond_V(rF)); }
  virtual int LD_BCDE_d(t_mem code);
  virtual int LD_JKHL_d(t_mem code);
  virtual int MULU(t_mem code);
  virtual int LD_JK_mn(t_mem code);
  virtual int LD_DE_HL(t_mem code);
  virtual int EX_BC_HL(t_mem code);
  virtual int EX_JKHL_BCDE(t_mem code);
  virtual int EX_JK_HL(t_mem code);
  virtual int CLR_HL(t_mem code);
  
  // Page ED, m4 mode
  virtual int CBM_N(t_mem code);
  virtual int LD_PW_iHTR_HL(t_mem code) { return ld_pd_ihtr_hl(cPW); }
  virtual int LD_PX_iHTR_HL(t_mem code) { return ld_pd_ihtr_hl(cPX); }
  virtual int LD_PY_iHTR_HL(t_mem code) { return ld_pd_ihtr_hl(cPY); }
  virtual int LD_PZ_iHTR_HL(t_mem code) { return ld_pd_ihtr_hl(cPZ); }
  virtual int SBOX_A(t_mem code);
  virtual int IBOX_A(t_mem code);
  virtual int DWJNZ(t_mem code);
  virtual int CP_HL_DE(t_mem code) { return cp16(rHL, rDE); }
  virtual int TEST_BC(t_mem code) { tick(2); return test16(rBC); }
  virtual int LLJP_GT_LXPC_MN(t_mem code)  { return lljp_cx(code); }
  virtual int LLJP_GTU_LXPC_MN(t_mem code) { return lljp_cx(code); }
  virtual int LLJP_LT_LXPC_MN(t_mem code)  { return lljp_cx(code); }
  virtual int LLJP_V_LXPC_MN(t_mem code)   { return lljp_cx(code); }
  virtual int LLJP_NZ_LXPC_MN(t_mem code)  { return lljp_cc(code); }
  virtual int LLJP_Z_LXPC_MN(t_mem code)   { return lljp_cc(code); }
  virtual int LLJP_NC_LXPC_MN(t_mem code)  { return lljp_cc(code); }
  virtual int LLJP_C_LXPC_MN(t_mem code)   { return lljp_cc(code); }
  virtual int PUSH_MN(t_mem code);
  virtual int JRE_GT_EE(t_mem code)  { return jre_cx_cc(cond_GT(rF)); }
  virtual int JRE_LT_EE(t_mem code)  { return jre_cx_cc(cond_LT(rF)); }
  virtual int JRE_GTU_EE(t_mem code) { return jre_cx_cc(cond_GTU(rF)); }
  virtual int JRE_V_EE(t_mem code)   { return jre_cx_cc(rF&flagV); }
  virtual int JRE_NZ_EE(t_mem code)  { return jre_cx_cc(!(rF&flagZ)); }
  virtual int JRE_Z_EE(t_mem code)   { return jre_cx_cc(rF&flagZ); }
  virtual int JRE_NC_EE(t_mem code)  { return jre_cx_cc(!(rF&flagC)); }
  virtual int JRE_C_EE(t_mem code)   { return jre_cx_cc(rF&flagC); }
  virtual int FLAG_NZ_HL(t_mem code) { return flag_cc_hl(code); }
  virtual int FLAG_Z_HL(t_mem code)   { return flag_cc_hl(code); }
  virtual int FLAG_NC_HL(t_mem code)  { return flag_cc_hl(code); }
  virtual int FLAG_C_HL(t_mem code)   { return flag_cc_hl(code); }
  virtual int FLAG_GT_HL(t_mem code)  { return flag_cc_hl(code); }
  virtual int FLAG_LT_HL(t_mem code)  { return flag_cc_hl(code); }
  virtual int FLAG_GTU_HL(t_mem code) { return flag_cc_hl(code); }
  virtual int FLAG_V_HL(t_mem code)   { return flag_cc_hl(code); }
  virtual int CALL_iHL(t_mem code);
  virtual int LLRET(t_mem code);
  virtual int EXP(t_mem code);
  virtual int LD_HTR_A(t_mem code);
  virtual int LD_A_HTR(t_mem code);
  virtual int FSYSCALL(t_mem code);
  virtual int SYSRET(t_mem code);
  virtual int SETUSRP(t_mem code);
  virtual int SETSYSP(t_mem code);
  virtual int LLCALL_iJKHL(t_mem code);
  virtual int LDL_PW_iSPn(t_mem code) { return ldl_pd_ispn(destPW()); }
  virtual int LDL_PX_iSPn(t_mem code) { return ldl_pd_ispn(destPX()); }
  virtual int LDL_PY_iSPn(t_mem code) { return ldl_pd_ispn(destPY()); }
  virtual int LDL_PZ_iSPn(t_mem code) { return ldl_pd_ispn(destPZ()); }
  virtual int LD_PW_iSPn(t_mem code) { return ld_pd_ispn(cPW); }
  virtual int LD_PX_iSPn(t_mem code) { return ld_pd_ispn(cPX); }
  virtual int LD_PY_iSPn(t_mem code) { return ld_pd_ispn(cPY); }
  virtual int LD_PZ_iSPn(t_mem code) { return ld_pd_ispn(cPZ); }
  virtual int LD_iSPn_PW(t_mem code) { return ld_ispn_ps(rPW); }
  virtual int LD_iSPn_PX(t_mem code) { return ld_ispn_ps(rPX); }
  virtual int LD_iSPn_PY(t_mem code) { return ld_ispn_ps(rPY); }
  virtual int LD_iSPn_PZ(t_mem code) { return ld_ispn_ps(rPZ); }
  virtual int LD_HL_iPWBC(t_mem code) { return ld_hl_ipsbc(rPW); }
  virtual int LD_HL_iPXBC(t_mem code) { return ld_hl_ipsbc(rPX); }
  virtual int LD_HL_iPYBC(t_mem code) { return ld_hl_ipsbc(rPY); }
  virtual int LD_HL_iPZBC(t_mem code) { return ld_hl_ipsbc(rPZ); }
  virtual int LD_iPWBC_HL(t_mem code) { return ld_ipdbc_hl(rPW); }
  virtual int LD_iPXBC_HL(t_mem code) { return ld_ipdbc_hl(rPX); }
  virtual int LD_iPYBC_HL(t_mem code) { return ld_ipdbc_hl(rPY); }
  virtual int LD_iPZBC_HL(t_mem code) { return ld_ipdbc_hl(rPZ); }
  virtual int LDF_PW_ilmn(t_mem code) { return ldf_pd_ilmn(destPW()); }
  virtual int LDF_PX_ilmn(t_mem code) { return ldf_pd_ilmn(destPX()); }
  virtual int LDF_PY_ilmn(t_mem code) { return ldf_pd_ilmn(destPY()); }
  virtual int LDF_PZ_ilmn(t_mem code) { return ldf_pd_ilmn(destPZ()); }
  virtual int LDF_ilmn_PW(t_mem code) { return ldf_ilmn_ps(rPW); }
  virtual int LDF_ilmn_PX(t_mem code) { return ldf_ilmn_ps(rPX); }
  virtual int LDF_ilmn_PY(t_mem code) { return ldf_ilmn_ps(rPY); }
  virtual int LDF_ilmn_PZ(t_mem code) { return ldf_ilmn_ps(rPZ); }
  virtual int LDF_BC_ilmn(t_mem code) { return ldf_rr_ilmn(destBC()); }
  virtual int LDF_DE_ilmn(t_mem code) { return ldf_rr_ilmn(destDE()); }
  virtual int LDF_IX_ilmn(t_mem code) { return ldf_rr_ilmn(cIX); }
  virtual int LDF_IY_ilmn(t_mem code) { return ldf_rr_ilmn(cIY); }
  virtual int LDF_ilmn_BC(t_mem code) { return ldf_ilmn_rr(rBC); }
  virtual int LDF_ilmn_DE(t_mem code) { return ldf_ilmn_rr(rDE); }
  virtual int LDF_ilmn_IX(t_mem code) { return ldf_ilmn_rr(rIX); }
  virtual int LDF_ilmn_IY(t_mem code) { return ldf_ilmn_rr(rIY); }
  virtual int LD_PW_klmn(t_mem code) { return ld_pd_klmn(cPW); }
  virtual int LD_PX_klmn(t_mem code) { return ld_pd_klmn(cPX); }
  virtual int LD_PY_klmn(t_mem code) { return ld_pd_klmn(cPY); }
  virtual int LD_PZ_klmn(t_mem code) { return ld_pd_klmn(cPZ); }
  virtual int LDL_PW_mn(t_mem code) { return ldl_pd_mn(destPW()); }
  virtual int LDL_PX_mn(t_mem code) { return ldl_pd_mn(destPX()); }
  virtual int LDL_PY_mn(t_mem code) { return ldl_pd_mn(destPY()); }
  virtual int LDL_PZ_mn(t_mem code) { return ldl_pd_mn(destPZ()); }
  virtual int CONVC_PW(t_mem code) { return convc_pp(cPW); }
  virtual int CONVC_PX(t_mem code) { return convc_pp(cPX); }
  virtual int CONVC_PY(t_mem code) { return convc_pp(cPY); }
  virtual int CONVC_PZ(t_mem code) { return convc_pp(cPZ); }
  virtual int CONVD_PW(t_mem code) { return convd_pp(cPW); }
  virtual int CONVD_PX(t_mem code) { return convd_pp(cPX); }
  virtual int CONVD_PY(t_mem code) { return convd_pp(cPY); }
  virtual int CONVD_PZ(t_mem code) { return convd_pp(cPZ); }
  virtual int CP_JKHL_BCDE(t_mem code) { return cp32(rJKHL, rBCDE); }
  virtual int EX_aBC_HL(t_mem code);
  virtual int EX_aJK_HL(t_mem code);
  virtual int COPY(t_mem code) { return copy(+1); }
  virtual int COPYR(t_mem code) { return copy(-1); }
  virtual int POP_PW(t_mem code) { return pop_pd(destPW()); }
  virtual int POP_PX(t_mem code) { return pop_pd(destPX()); }
  virtual int POP_PY(t_mem code) { return pop_pd(destPY()); }
  virtual int POP_PZ(t_mem code) { return pop_pd(destPZ()); }
  virtual int PUSH_PW(t_mem code) { return push_ps(rPW); }
  virtual int PUSH_PX(t_mem code) { return push_ps(rPX); }
  virtual int PUSH_PY(t_mem code) { return push_ps(rPY); }
  virtual int PUSH_PZ(t_mem code) { return push_ps(rPZ); }
  virtual int ADD_JKHL_BCDE(t_mem code);
  virtual int SUB_JKHL_BCDE(t_mem code);
  virtual int AND_JKHL_BCDE(t_mem code);
  virtual int OR_JKHL_BCDE(t_mem code);
  virtual int XOR_JKHL_BCDE(t_mem code);
  virtual int LD_HL_iSPHL(t_mem code);
  
  // Page DD/FD
  virtual int LD_A_iIRA(t_mem code);
  virtual int TEST_IR(t_mem code) { tick(2); return test16(cIR->get()); }
  virtual int LD_IRR_iHL(t_mem code);
  virtual int LD_IRR_iIXd(t_mem code) { return ld_irr_iird(cIX); }
  virtual int LD_IRR_iIYd(t_mem code) { return ld_irr_iird(cIY); }
  virtual int LD_IRR_iSPn(t_mem code);
  virtual int LD_iHL_IRR(t_mem code);
  virtual int LD_iIXd_IRR(t_mem code) { return ld_iird_irr(cIX); }
  virtual int LD_iIYd_IRR(t_mem code) { return ld_iird_irr(cIY); }
  virtual int LD_iSPn_IRR(t_mem code);
  virtual int NEG_IRR(t_mem coed) { return sub32(0, cIRR->get(),
						 *cIRR, false); }
  virtual int POP_IRR(t_mem code);
  virtual int PUSH_IRR(t_mem code);
  virtual int RL_1_IRR(t_mem code) { return rot33left(*destIRR(),
						      cIRR->get(),
						      1); }
  virtual int RL_2_IRR(t_mem code) { return rot33left(*destIRR(),
						      cIRR->get(),
						      2); }
  virtual int RL_4_IRR(t_mem code) { return rot33left(*destIRR(),
						      cIRR->get(),
						      4); }
  virtual int RLC_1_IRR(t_mem code) { return rot32left(*destIRR(),
						       cIRR->get(),
						       1); }
  virtual int RLC_2_IRR(t_mem code) { return rot32left(*destIRR(),
						       cIRR->get(),
						       2); }
  virtual int RLC_4_IRR(t_mem code) { return rot32left(*destIRR(),
						       cIRR->get(),
						       4); }
  virtual int RLC_8_IRR(t_mem code);
  virtual int RLB_A_IRR(t_mem code);
  virtual int SLA_1_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 1); }
  virtual int SLA_2_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 2); }
  virtual int SLA_4_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 4); }
  virtual int SLL_1_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 1); }
  virtual int SLL_2_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 2); }
  virtual int SLL_4_IRR(t_mem code) { return sla32(*destIRR(), cIRR->get(), 4); }
  virtual int TEST_IRR(t_mem code) { tick(2); return test32(cIRR->get()); }
  virtual int RR_1_IRR(t_mem code) { return rot33right(*destIRR(),
						       cIRR->get(),
						       1); }
  virtual int RR_2_IRR(t_mem code) { return rot33right(*destIRR(),
						       cIRR->get(),
						       2); }
  virtual int RR_4_IRR(t_mem code) { return rot33right(*destIRR(),
						       cIRR->get(),
						       4); }
  virtual int RRC_1_IRR(t_mem code) { return rot32right(*destIRR(),
							cIRR->get(),
							1); }
  virtual int RRC_2_IRR(t_mem code) { return rot32right(*destIRR(),
							cIRR->get(),
							2); }
  virtual int RRC_4_IRR(t_mem code) { return rot32right(*destIRR(),
							cIRR->get(),
							4); }
  virtual int RRC_8_IRR(t_mem code);
  virtual int RRB_A_IRR(t_mem code);
  virtual int SRA_1_IRR(t_mem code) { return sra32(*destIRR(),
						   cIRR->get(),
						   1); }
  virtual int SRA_2_IRR(t_mem code) { return sra32(*destIRR(),
						   cIRR->get(),
						   2); }
  virtual int SRA_4_IRR(t_mem code) { return sra32(*destIRR(),
						   cIRR->get(),
						   4); }
  virtual int SRL_1_IRR(t_mem code) { return srl32(*destIRR(),
						   cIRR->get(),
						   1); }
  virtual int SRL_2_IRR(t_mem code) { return srl32(*destIRR(),
						   cIRR->get(),
						   2); }
  virtual int SRL_4_IRR(t_mem code) { return srl32(*destIRR(),
						   cIRR->get(),
						   4); }
  virtual int LDF_IRR_iLMN(t_mem code);
  virtual int LDF_iLMN_IRR(t_mem code);
  virtual int LD_IRR_iPW_HL(t_mem code) { return ld_irr_ips_hl(code); }
  virtual int LD_IRR_iPX_HL(t_mem code) { return ld_irr_ips_hl(code); }
  virtual int LD_IRR_iPY_HL(t_mem code) { return ld_irr_ips_hl(code); }
  virtual int LD_IRR_iPZ_HL(t_mem code) { return ld_irr_ips_hl(code); }
  virtual int LD_iPW_HL_IRR(t_mem code) { return ld_ips_hl_irr(code); }
  virtual int LD_iPX_HL_IRR(t_mem code) { return ld_ips_hl_irr(code); }
  virtual int LD_iPY_HL_IRR(t_mem code) { return ld_ips_hl_irr(code); }
  virtual int LD_iPZ_HL_IRR(t_mem code) { return ld_ips_hl_irr(code); }
  virtual int LD_IRR_iPW_D(t_mem code) { return ld_irr_ips_d(code); }
  virtual int LD_IRR_iPX_D(t_mem code) { return ld_irr_ips_d(code); }
  virtual int LD_IRR_iPY_D(t_mem code) { return ld_irr_ips_d(code); }
  virtual int LD_IRR_iPZ_D(t_mem code) { return ld_irr_ips_d(code); }
  virtual int LD_iPW_D_IRR(t_mem code) { return ld_ips_d_irr(code); }
  virtual int LD_iPX_D_IRR(t_mem code) { return ld_ips_d_irr(code); }
  virtual int LD_iPY_D_IRR(t_mem code) { return ld_ips_d_irr(code); }
  virtual int LD_iPZ_D_IRR(t_mem code) { return ld_ips_d_irr(code); }
  virtual int LDL_PW_IR(t_mem code) { return ldl_px_ir(code); }
  virtual int LDL_PX_IR(t_mem code) { return ldl_px_ir(code); }
  virtual int LDL_PY_IR(t_mem code) { return ldl_px_ir(code); }
  virtual int LDL_PZ_IR(t_mem code) { return ldl_px_ir(code); }
  virtual int LD_PW_IRR(t_mem code) { return ld_px_irr(code); }
  virtual int LD_PX_IRR(t_mem code) { return ld_px_irr(code); }
  virtual int LD_PY_IRR(t_mem code) { return ld_px_irr(code); }
  virtual int LD_PZ_IRR(t_mem code) { return ld_px_irr(code); }
  virtual int LDL_PW_IRRL(t_mem code) { return ldl_px_irrl(code); }
  virtual int LDL_PX_IRRL(t_mem code) { return ldl_px_irrl(code); }
  virtual int LDL_PY_IRRL(t_mem code) { return ldl_px_irrl(code); }
  virtual int LDL_PZ_IRRL(t_mem code) { return ldl_px_irrl(code); }
  virtual int LD_IRR_PW(t_mem code) { return ld_irr_px(code); }
  virtual int LD_IRR_PX(t_mem code) { return ld_irr_px(code); }
  virtual int LD_IRR_PY(t_mem code) { return ld_irr_px(code); }
  virtual int LD_IRR_PZ(t_mem code) { return ld_irr_px(code); }
  virtual int LD_iSP_HL_IRR(t_mem code);
  virtual int CALL_iIR(t_mem code);
  virtual int LD_IRR_iSP_HL(t_mem code);
  
  // Starter of extra pages
  virtual int PAGE_4K6D(t_mem code);
  virtual int PAGE_4K7F(t_mem code);
};

class cl_r4k_cpu: public cl_rxk_cpu
{
protected:
  class cl_r4k *r4uc;
  class cl_cell8 *edmr;
  class cl_memory_cell *stacksegl, *stacksegh;
  class cl_memory_cell *datasegl , *datasegh;
public:
  cl_r4k_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual const char *cfg_help(t_addr addr);
  
  //virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of rxk.src/r4kcl.h */
