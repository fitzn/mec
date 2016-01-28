/*
 * A modified elias code library implementation.
 *
 * Author: M.F. Nowlan
 */

#include "mec.h"

#include <assert.h>

/* The number of unique values that can be encoded with each bit-length.
 * This excludes the hi/lo leading bits, which add a signal bit for each value.
 * Each bit-length requires that many leading bits plus the middle marker bit.
 */
static uint32_t bins[] = {
  /* 0 - 7 bits */
  0, 2, 6, 14, 30, 62, 126, 254,
  /* 8 - 15 bits */
  510, 1022, 2046, 4094, 8190, 16382, 32766, 65534,
  /* 16 - 23 bits */
  131070, 262142, 524286, 1048574, 2097150, 4194302, 8388606, 16777214,
  /* 24 - 27 bits */
  33554430, 67108862, 134217726, 268435454,
  /* 27 - 31 bits (last value is the max number of unique, encodable values) */
  536870910, 1073741822, 2147483646, 4294967294
};
static const uint32_t bins_size = sizeof(bins) / sizeof(uint32_t);

static ebuf map[] = {
  {  0x02, 3},  /* lo, 0  */
  {  0x04, 3},  /* hi, 0  */
  {  0x03, 3},  /* lo, 1  */
  {  0x05, 3},  /* hi, 1  */
  {  0x04, 5},  /* lo, 2  */
  {  0x18, 5},  /* hi, 2  */
  {  0x05, 5},  /* lo, 3  */
  {  0x19, 5},  /* hi, 3  */
  {  0x06, 5},  /* lo, 4  */
  {  0x1a, 5},  /* hi, 4  */
  {  0x07, 5},  /* lo, 5  */
  {  0x1b, 5},  /* hi, 5  */
  {  0x08, 7},  /* lo, 6  */
  {  0x70, 7},  /* hi, 6  */
  {  0x09, 7},  /* lo, 7  */
  {  0x71, 7},  /* hi, 7  */
  {  0x0a, 7},  /* lo, 8  */
  {  0x72, 7},  /* hi, 8  */
  {  0x0b, 7},  /* lo, 9  */
  {  0x73, 7},  /* hi, 9  */
  {  0x0c, 7},  /* lo, 10 */
  {  0x74, 7},  /* hi, 10 */
  {  0x0d, 7},  /* lo, 11 */
  {  0x75, 7},  /* hi, 11 */
  {  0x0e, 7},  /* lo, 12 */
  {  0x76, 7},  /* hi, 12 */
  {  0x0f, 7},  /* lo, 13 */
  {  0x77, 7},  /* hi, 13 */
};
static const uint32_t map_size = sizeof(map) / sizeof(ebuf);

static uint8_t reverse[] = {
  // 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, ....
  0, 128, 64, 192, 32, 160, 96, 224, 
  16, 144, 80, 208, 48, 176, 112, 240,
  8, 136, 72, 200, 40, 168, 104, 232, 
  24, 152, 88, 216, 56, 184, 120, 248, 
  4, 132, 68, 196, 36, 164, 100, 228, 
  20, 148, 84, 212, 52, 180, 116, 244, 
  12, 140, 76, 204, 44, 172, 108, 236, 
  28, 156, 92, 220, 60, 188, 124, 252, 
  2, 130, 66, 194, 34, 162, 98, 226, 
  18, 146, 82, 210, 50, 178, 114, 242, 
  10, 138, 74, 202, 42, 170, 106, 234, 
  26, 154, 90, 218, 58, 186, 122, 250, 
  6, 134, 70, 198, 38, 166, 102, 230, 
  22, 150, 86, 214, 54, 182, 118, 246, 
  14, 142, 78, 206, 46, 174, 110, 238, 
  30, 158, 94, 222, 62, 190, 126, 254, 
  1, 129, 65, 193, 33, 161, 97, 225, 
  17, 145, 81, 209, 49, 177, 113, 241, 
  9, 137, 73, 201, 41, 169, 105, 233, 
  25, 153, 89, 217, 57, 185, 121, 249, 
  5, 133, 69, 197, 37, 165, 101, 229, 
  21, 149, 85, 213, 53, 181, 117, 245, 
  13, 141, 77, 205, 45, 173, 109, 237, 
  29, 157, 93, 221, 61, 189, 125, 253, 
  3, 131, 67, 195, 35, 163, 99, 227, 
  19, 147, 83, 211, 51, 179, 115, 243, 
  11, 139, 75, 203, 43, 171, 107, 235, 
  27, 155, 91, 219, 59, 187, 123, 251, 
  7, 135, 71, 199, 39, 167, 103, 231, 
  23, 151, 87, 215, 55, 183, 119, 247, 
  15, 143, 79, 207, 47, 175, 111, 239, 
  31, 159, 95, 223, 63, 191, 127, 255
};

ebuf eb_encode(uint32_t value, int hi) {

  assert(value < bins[bins_size - 1]);

  /* See if we can just lookup the pre-computed value first. */
  if (value < (map_size >> 2))
    return map[(value << 1) + ((hi) ? 1 : 0)];

  /* Otherwise, we compute the code.
   * First, we find the number of bits needed to represent this value as
   * an offset from the sum of the number of values from bit-lengths below it.
   * Second, the prefix is either all zeroes followed by a 1, so simply 1, or
   * it's all ones followed by a zero. The length is determined by 'bits' below.
   */
  uint32_t bits, prefix = 1;
  for (bits = 1; bins[bits] <= value; bits++)
    ;
  value -= bins[bits - 1];
  if (hi)
    prefix = ((1 << bits) - 1) << 1;  // Last shift adds in the 0.

  return (ebuf){((prefix << bits) | value), ((bits << 1) + 1)};
}

uint32_t eb_code_len(ebuf eb, int nr_bits) {

  if (!nr_bits)
    return 0;

  uint64_t key = ((uint64_t)1 << (uint64_t)(nr_bits - 1));
  int bits = 0, hi = (eb.buf & key) ? 1 : 0;
  while (key && ((hi && (eb.buf & key)) || (!hi && !(eb.buf & key)))) {
    key >>= 1;
    bits++;
  }
  assert(bits + 1 + bits <= nr_bits);
  return bits + 1 + bits;  // The code is [ leading | marker | value ] bits.
}

uint32_t eb_decode(ebuf eb) {

  assert(eb_code_len(eb, eb.bits) == eb.bits && (eb.bits & 0x1) && eb.bits > 2);

  /* The length of bits indicates the bin. The value indicates the offset. */
  int bits = (eb.bits - 1) >> 1;
  return bins[bits - 1] + (eb.buf & (((uint64_t)1 << bits) - 1));
}

ebuf eb_concat(ebuf l, ebuf r) {
  assert(l.bits + r.bits <= BUF_MAX);  // TODO: handle this case better.
  return (ebuf){((l.buf << r.bits) | (r.buf & ((1 << r.bits) - 1))),
                (l.bits + r.bits)};
}

ebuf eb_split(ebuf *eb) {

  /* Get the code's length. */
  int clen = eb_code_len(*eb, eb->bits); 
  if (!clen) {
    fprintf(stderr, "Warning: eb_split did not find a code.\n");
    return (ebuf){0, 0};
  }

  /* There is a code on the left, so move it to the right in the copy
   * while simply ANDing it out from 'eb' and shortening its bit length.
   */
  int rbits = eb->bits - clen;

  ebuf copy = *eb;
  copy.buf >>= rbits;
  copy.bits = clen;

  eb->bits = rbits;
  eb->buf &= ((uint64_t)1 << rbits) - 1;

  return copy;
}

ebuf eb_align(ebuf eb) {

  int shift = 8 - (eb.bits & 0x7);
  if (shift != 8) {
    eb.buf <<= shift;
    eb.bits += shift;
  }
  return eb;
}

int eb_write(ebuf eb, char *dst, int max) {

  eb = eb_align(eb);
  int bytes = eb.bits >> 3, b = bytes;
  if (bytes > max)
    return 0;

  /* Write bytes from high to low, each byte reversed. */
  while (b--) {
    uint8_t byte = (eb.buf >> (b << 3)) & 0xff;
    *dst++ = reverse[byte];
  }
  return bytes;
}

int eb_read(char *src, int len, ebuf *eb, int nr_codes) {

  int i = 0, bytes = ((len < (BUF_MAX >> 3)) ? len : (BUF_MAX >> 3));
  ebuf ebx = {0, 0};
  *eb = ebx;

  if (!len || !nr_codes)
    return 0;

  /* Read up to BUF_MAX bits into 'ebx'. */
  while (i < bytes) {
    uint8_t byte = (uint8_t)src[i++];
    byte = reverse[byte];
    ebx.buf = (ebx.buf << 8) | byte;
    ebx.bits += 8;
  }
  assert(i > 0 && i <= 8 && ebx.bits > 0);

  /* Step through codes, finding the number of extra bits on the right. */
  int bits = ebx.bits, codes;
  for (codes = 0; codes < nr_codes; codes++) {
    bits -= eb_code_len(ebx, bits);
  }

  /* Drop the extra bits. */
  ebx.buf >>= bits;
  ebx.bits -= bits;
  i -= (bits >> 3);      // We may not have consumed all the bytes we read.
  *eb = ebx;
  return i;
}

