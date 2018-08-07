/*
 * ZX7b (c) Copyright 2013 by Antonio Villena. All rights reserved.
 *
 * Based on ZX7 <http://www.worldofspectrum.org/infoseekid.cgi?id=0027996>
 * (c) Copyright 2012 by Einar Saukas. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of its author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OFFSET  2176  /* range 1..2176 */
#define MAX_LEN    65536  /* range 2..65536 */

typedef struct match_t {
  size_t index;
  struct match_t *next;
} Match;

typedef struct optimal_t {
  size_t bits;
  int offset;
  int len;
} Optimal;

Optimal *optimize(unsigned char *input_data, size_t input_size);

unsigned char *compress(Optimal *optimal, unsigned char *input_data, size_t input_size, size_t *output_size);

int main(int argc, char *argv[]) {
  FILE *ifp;
  FILE *ofp;
  unsigned char *input_data;
  unsigned char *output_data;
  size_t input_size;
  size_t output_size;
  size_t partial_counter;
  size_t total_counter;
  char *output_name;
  int i, j;

  if( argc==1 )
    printf("\nZX7 Backwards compressor v1.01 by Einar Saukas/AntonioVillena, 28 Dec 2013\n\n"
           "  zx7b <input_file> <output_file>\n\n"
           "  <input_file>    Raw input file\n"
           "  <output_file>   Compressed output file\n\n"
           "Example: zx7b Cobra.scr Cobra.zx7b\n"),
    exit(0);
  if( argc!=3 )
    printf("\nInvalid number of parameters\n"),
    exit(-1);

  /* open input file */
  ifp= fopen(argv[1], "rb");
  if( !ifp )
    fprintf(stderr, "Error: Cannot access input file %s\n", argv[1]),
    exit(1);

  /* determine input size */
  fseek(ifp, 0L, SEEK_END);
  input_size= ftell(ifp);
  fseek(ifp, 0L, SEEK_SET);
  if( !input_size )
    fprintf(stderr, "Error: Empty input file %s\n", argv[1]),
    exit(1);

  /* allocate input buffer */
  input_data= (unsigned char *)malloc(input_size);
  if( !input_data )
    fprintf(stderr, "Error: Insufficient memory\n"),
    exit(1);

  /* read input file */
  total_counter= 0;
  do {
    partial_counter = fread(input_data+total_counter, sizeof(char), input_size-total_counter, ifp);
    total_counter += partial_counter;
  } while ( partial_counter > 0 );

  if( total_counter != input_size )
    fprintf(stderr, "Error: Cannot read input file %s\n", argv[1]),
    exit(1);

  /* close input file */
  fclose(ifp);

  /* create output file */
  ofp= fopen(argv[2], "wb");
  if( !ofp )
    fprintf(stderr, "Error: Cannot create output file %s\n", argv[2]),
    exit(1);

  for ( i= 0; i<input_size>>1; i++ )
    j= input_data[i],
    input_data[i]= input_data[input_size-1-i],
    input_data[input_size-1-i]= j;

  /* generate output file */
  output_data= compress(optimize(input_data, input_size), input_data, input_size, &output_size);

  for ( i= 0; i<output_size>>1; i++ )
    j= output_data[i],
    output_data[i]= output_data[output_size-1-i],
    output_data[output_size-1-i]= j;

  /* write output file */
  if( fwrite(output_data, sizeof(char), output_size, ofp) != output_size )
    fprintf(stderr, "Error: Cannot write output file %s\n", output_name),
    exit(1);

  /* close output file */
  fclose(ofp);

  /* done! */
  printf("\nFile %s compressed from %s (%d to %d bytes)\n", argv[2], argv[1], (int) input_size, (int) output_size);
  return 0;
}

unsigned char* output_data;
size_t output_index;
size_t bit_index;
int bit_mask;

void write_byte(int value) {
  output_data[output_index++] = value;
}

void write_bit(int value) {
  if( bit_mask == 0 )
    bit_mask = 128,
    bit_index = output_index,
    write_byte(0);
  if( value > 0 )
    output_data[bit_index] |= bit_mask;
  bit_mask >>= 1;
}

int elias_gamma_bits(int value) {
  int bits;
  bits= 1;
  while ( value > 1 )
    bits+= 2,
    value>>= 1;
  return bits;
}

void write_elias_gamma(int value) {
  int bits= 0, rvalue= 0;
  while ( value>1 )
    ++bits,
    rvalue<<= 1,
    rvalue|= value&1,
    value>>= 1;
  while ( bits-- )
    write_bit(0),
    write_bit(rvalue & 1),
    rvalue>>= 1;
  write_bit(1);
}

unsigned char *compress(Optimal *optimal, unsigned char *input_data, size_t input_size, size_t *output_size) {
  size_t input_index;
  size_t input_prev;
  int offset1;
  int mask;
  int i;

  /* calculate and allocate output buffer */
  input_index= input_size-1;
  *output_size= (optimal[input_index].bits+16+7)/8;
  output_data= (unsigned char *)malloc(*output_size);
  if( !output_data )
    fprintf(stderr, "Error: Insufficient memory\n"),
    exit(1);

  /* un-reverse optimal sequence */
  optimal[input_index].bits= 0;
  while ( input_index > 0 )
    input_prev= input_index - (optimal[input_index].len > 0 ? optimal[input_index].len : 1),
    optimal[input_prev].bits= input_index,
    input_index= input_prev;

  output_index= 0;
  bit_mask= 0;

  /* first byte is always literal */
  write_byte(input_data[0]);

  /* process remaining bytes */
  while ( (input_index= optimal[input_index].bits) > 0)
    if( optimal[input_index].len == 0)
      write_bit(0),
      write_byte(input_data[input_index]);
    else{
      /* sequence indicator */
      write_bit(1);

      /* sequence length */
      write_elias_gamma(optimal[input_index].len-1);

      /* sequence offset */
      offset1= optimal[input_index].offset-1;
      if( offset1 < 128 )
        write_byte(offset1);
      else{
        offset1-= 128;
        write_byte((offset1 & 127) | 128);
        for ( mask= 1024; mask > 127; mask >>= 1)
          write_bit(offset1 & mask);
      }
    }

  /* end mark */
  write_bit(1);
  write_elias_gamma(0xff);
  return output_data;
}

int count_bits(int offset, int len) {
    return 1 + (offset > 128 ? 12 : 8) + elias_gamma_bits(len-1);
}

Optimal* optimize(unsigned char *input_data, size_t input_size) {
    size_t *min;
    size_t *max;
    Match *matches;
    Match *match_slots;
    Optimal *optimal;
    Match *match;
    int match_index;
    int offset;
    size_t len;
    size_t best_len;
    size_t bits;
    size_t i;

    /* allocate all data structures at once */
    min = (size_t *)calloc(MAX_OFFSET+1, sizeof(size_t));
    max = (size_t *)calloc(MAX_OFFSET+1, sizeof(size_t));
    matches = (Match *)calloc(256*256, sizeof(Match));
    match_slots = (Match *)calloc(input_size, sizeof(Match));
    optimal = (Optimal *)calloc(input_size, sizeof(Optimal));

    if (!min || !max || !matches || !match_slots || !optimal) {
         fprintf(stderr, "Error: Insufficient memory\n");
         exit(1);
    }

    /* first byte is always literal */
    optimal[0].bits = 8;

    /* process remaining bytes */
    for ( i= 1; i < input_size; i++ ){
      optimal[i].bits= optimal[i-1].bits + 9;
      match_index= input_data[i-1] << 8 | input_data[i];
      best_len= 1;
      for ( match= &matches[match_index]; match->next != NULL && best_len < MAX_LEN; match = match->next){
        offset= i - match->next->index;
        if( offset > MAX_OFFSET ){
          match->next = NULL;
          break;
        }
        for ( len= 2; len <= MAX_LEN; len++ ){
          if( len > best_len && len&0xff ){
            best_len= len;
            bits= optimal[i-len].bits + count_bits(offset, len);
            if( optimal[i].bits > bits )
              optimal[i].bits= bits,
              optimal[i].offset= offset,
              optimal[i].len= len;
          }
          else if ( i+1 == max[offset]+len && max[offset] != 0 ){
            len= i-min[offset];
            if( len > best_len )
              len= best_len;
          }
          if( i < offset+len || input_data[i-len] != input_data[i-len-offset] )
            break;
        }
        min[offset]= i+1-len;
        max[offset]= i;
      }
      match_slots[i].index= i;
      match_slots[i].next= matches[match_index].next;
      matches[match_index].next= &match_slots[i];
    }

    /* save time by releasing the largest block only, the O.S. will clean everything else later */
    free(match_slots);
    return optimal;
}
