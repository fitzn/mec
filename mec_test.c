/*
 * A simple test script for the modified elias code library.
 *
 * Author: M.F. Nowlan
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mec.h"

int main() {

  /* Check simple equals macro. */
  ebuf a = {0, 0};
  ebuf b = {0, 0};
  assert(eb_eq(a, b));

  a.buf = 132;
  a.bits = 32;
  b.buf = 132;
  b.bits = 32;
  assert(eb_eq(a, b));

  /* Check the encoding of values. */
  int i;
  for (i = 0; i < 30; i++) {

    ebuf code = eb_encode(i, 0);
    assert(eb_code_len(code, code.bits) == code.bits);
    assert(!eb_hi(code));

    code = eb_encode(i, 1);
    assert(eb_code_len(code, code.bits) == code.bits);
    assert(eb_hi(code));
  }

  ebuf one = eb_encode(3, 0);
  ebuf two = eb_encode(9, 0);

  /* Check concat and split. */
  ebuf cat = eb_concat(one, two);
  assert(cat.buf == (one.buf << two.bits) + two.buf);

  ebuf back = eb_split(&cat);
  assert(eb_eq(back, one) && eb_eq(cat, two));

  /* Check align. */
  ebuf eb = {0, 0};
  for (i = 0; i < 9; i++) {

    eb.buf = i;
    eb.bits = i;
    ebuf al = eb_align(eb);
    assert((al.bits & 0x7) == 0);
    assert(al.buf == (i << (8 - (i & 0x7))) || (i == 8 && al.buf == i));

    ebuf same = eb_align(al);
    assert(eb_eq(al, same));
  }

  /* Check write. */
  int max = 8, wout, rout;
  char out[max];
  memset(out, 0, max);
  eb.buf = 0, eb.bits = 0;
  wout = eb_write(eb, out, max);
  assert(!wout); // Shouldn't write anything b/c eb.bits == 0.

  eb = eb_encode(19, 0);
  wout = eb_write(eb, out, 1);
  assert(!wout); // There wasn't enough space.

  int val = 9;
  eb = eb_encode(val, 0);
  wout = eb_write(eb, out, 1);
  assert(wout == 1);

  /* Check read. */
  ebuf check = {0, 0};
  rout = eb_read(out, 8, &check, 1);
  assert(rout == wout && eb_eq(eb, check));

  /* Check a full combo. */
  one = eb_encode(4, 1);
  two = eb_encode(15, 0);
  one = eb_concat(one, two);
  wout = eb_write(one, out, max);
  rout = eb_read(out, max, &two, 2);
  assert(wout == rout && eb_eq(one, two));

  one = eb_concat(one, check);
  wout = eb_write(one, out, max);
  rout = eb_read(out, max, &check, 2);
  assert(eb_eq(two, check));
  rout = eb_read(out, max, &two, 3);
  assert(wout == rout && eb_eq(one, two));

  ebuf orig = eb_split(&one);
  assert(eb_eq(orig, eb_encode(4, 1)));
  orig = eb_split(&one);
  assert(eb_eq(orig, eb_encode(15, 0)));
  orig = eb_split(&one);
  assert(!one.buf && !one.bits);

  uint32_t val2 = eb_decode(orig);
  assert(val == val2);

  /* One last cycle. */
  uint32_t n0 = 8323, n1 = 12;
  one = eb_encode(n0, 0);
  two = eb_encode(n1, 1);
  assert(!eb_hi(one) && eb_hi(two));
  cat = eb_concat(one, two);

  memset(out, 0, max);
  wout = eb_write(cat, out, max);
  rout = eb_read(out, max, &check, 2);
  assert(wout == rout && eb_eq(check, cat));

  ebuf last = eb_split(&check);
  assert(eb_eq(last, one) && eb_eq(check, two));
  assert(eb_decode(last) == n0);

  last = eb_split(&check);
  assert(eb_eq(last, two) && !check.buf && !check.bits);
  assert(eb_hi(last));
  assert(eb_decode(last) == n1);

  printf("ebuf tests passed\n");

  return 0;
}
