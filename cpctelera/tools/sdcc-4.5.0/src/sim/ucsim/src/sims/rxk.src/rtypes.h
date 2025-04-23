/*
 * Simulator of microcontrollers (rtypes.h)
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

#ifndef RTYPES_HEADER
#define RTYPES_HEADER

#ifdef WORDS_BIGENDIAN
#define RP(N,N16,NH,NL) union			\
		      {				\
			u16_t N16;		\
			struct {		\
			  u8_t NH;		\
			  u8_t NL;		\
			} r;			\
  } N
#define R32(N,N32,N16H,N16L,NHH,NHL,NLH,NLL)	\
  union						\
  {						\
  u32_t N32;					\
  struct {					\
    RP(r16h,N16H,NHH,NHL);			\
    RP(r16l,N16L,NLH,NLL);			\
  } r32;					\
  } N
#else
#define RP(N,N16,NH,NL) union			\
		      {				\
			u16_t N16;		\
			struct {		\
			  u8_t NL;		\
			  u8_t NH;		\
			} r;			\
  } N
#define R32(N,N32,N16H,N16L,NHH,NHL,NLH,NLL)	\
  union						\
  {						\
  u32_t N32;					\
  struct {					\
    RP(r16l,N16L,NLH,NLL);			\
    RP(r16h,N16H,NHH,NHL);			\
  } r32;					\
  } N
#endif

#endif

/* End of rxk.src/rtypes.cc */
