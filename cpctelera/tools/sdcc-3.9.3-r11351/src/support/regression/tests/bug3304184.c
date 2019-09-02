/** bug3304184.c
*/

#include <testfwk.h>

/////////////////////////////////////////////////////////////////////////////
//
// File: util_xor_arrays.c
//
// Copyright S. Brennen Ball, 2010
//
// The author provides no guarantees, warantees, or promises, implied or
//  otherwise.  By using this software you agree to indemnify the author
//  of any damages incurred by using it.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
/////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#define __TARG_FUNC_POSTDECL		__reentrant

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// void util_xor_arrays(const uint8_t * input_block_a, const uint8_t * input_block_b, uint8_t * output_block, const uint16_t len) __TARG_FUNC_POSTDECL
//
// Description:
//  XORs two arrays.
//
// Parameters (when using for pre-encryption):
//  const uint8_t * input_block_a - input block A
//  const uint8_t * input_block_b - input block B
//  uint8_t * output_block - output block
//  uint16_t len - number of bytes to XOR
//
// Return value:
//  None
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void util_xor_arrays(const uint8_t * input_block_a, const uint8_t * input_block_b, uint8_t * output_block, const uint16_t len) __TARG_FUNC_POSTDECL
{
	uint16_t i;

	for(i = 0; i < len; i++)
	{
		output_block[i] = input_block_a[i] ^ input_block_b[i];
	}
}

/* compiling util_xor_arrays gave the following error due to target out of range for cjne
   Error: <a> machine specific addressing or addressing mode error */
void testBug(void)
{
}
