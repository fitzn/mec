/*
 * A modified elias code library.
 *
 * Author: M.F. Nowlan
 */

#ifndef MEC_H
#define MEC_H

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define BUF_MAX             64

struct ebuf_t {
  uint64_t buf;
  int bits;
};
typedef struct ebuf_t ebuf;

#define eb_eq(a, b)         ((a).bits == (b).bits && \
                             ((a).buf << (BUF_MAX - (a).bits)) == \
                             ((b).buf << (BUF_MAX - (b).bits)))
#define eb_hi(c)            ((c).bits ? \
                              ((c).buf & ((uint64_t)1 << ((c).bits - 1))) : 0)
#define eb_print(c)         (printf("ebuf: .buf %"PRIu64" .bits %d\n", \
                                    (c).buf, (c).bits))

/* Returns 'value' encoded in an ebuf, using leading 1's if 'hi' is set. */
ebuf eb_encode(uint32_t value, int hi);

/* Returns the bit-length of the l-to-r code in eb.buf,
 * starting at the bit index 'nr_bits' - 1 and moving right.
 */
uint32_t eb_code_len(ebuf eb, int nr_bits);

/* Returns the value of the mec in eb. */
uint32_t eb_decode(ebuf eb);

/* Returns a new ebuf that is the concatenation of 'l' and 'r' codes. */
ebuf eb_concat(ebuf l, ebuf r);

/* Returns the left-most code in 'eb' and updates it to reflect the removal. */
ebuf eb_split(ebuf *eb);

/* Shifts the ebuf to the left until it is byte aligned. */
ebuf eb_align(ebuf eb);

/* Byte aligns the bits in 'eb' and writes the bytes out to 'dst'.
 * Bytes are reversed and written from left to right (i.e. network-order).
 * Returns the number of bytes written to 'dst' or 0 if insufficient space.
 */
int eb_write(ebuf eb, char *dst, int max);

/* Reads up to 'nr_codes' into 'eb', overwriting any existing bits in 'eb'.
 * Returns the number of whole bytes consumed.
 */
int eb_read(char *src, int len, ebuf *eb, int nr_codes);

#endif // MEC_H
