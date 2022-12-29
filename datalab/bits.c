/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  int ax;
  int ay;
  int aans;
  int ans;
  ax = ~x;
  ay = ~y;
  aans = ax | ay;
  ans = ~aans;
  return ans;
}
/* 
 * bitConditional - x ? y : z for each bit respectively
 *   Example: bitConditional(0b00110011, 0b01010101, 0b00001111) = 0b00011101
 *   Legal ops: & | ^ ~
 *   Max ops: 8
 *   Rating: 1
 */
int bitConditional(int x, int y, int z) {
  int ans1;
  int nx;
  int ans2;
  int ans;
  nx = ~x;
  ans1 = x & y;
  ans2 = nx & z;
  ans = ans1 ^ ans2;
  return ans;
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
  int a;
  int M;
  int N;
  int ma;
  int na;
  int xn;
  int xm;
  int mb;
  int nb;
  int ans0,ans;
  int tmp;
  int tmpm;
  int tmpn;
  tmp = 1 << 31;
  a = 0xFF;
  M = m << 3;
  N = n << 3;
  ma = a << M;
  na = a << N;
  tmpm = ~ ( tmp >> M << 1);
  tmpn = ~ ( tmp >> N << 1);
  xn = x & na;
  xm = x & ma;
  mb = (( xm >> M ) & tmpm )<< N;
  nb = (( xn >> N ) & tmpn )<< M;
  ans0 = x & (~(ma | na));
  ans = ans0 | (mb | nb);
  return ans;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int x1;
  int a;
  int b;
  int c;
  int ans;
  x1 = x >> n;
  a = 1 << 31;
  b = a >> n << 1;
  c = ~b;
  ans = c & x1;
  return ans;
}
/* 
 * cleanConsecutive1 - change any consecutive 1 to zeros in the binary form of x.
 *   Consecutive 1 means a set of 1 that contains more than one 1.
 *   Examples cleanConsecutive1(0x10) = 0x10
 *            cleanConsecutive1(0xF0) = 0x0
 *            cleanConsecutive1(0xFFFF0001) = 0x1
 *            cleanConsecutive1(0x4F4F4F4F) = 0x40404040
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 4
 */
int cleanConsecutive1(int x){
  int x1;
  int a;
  int c;
  int ans1;
  int ans2;
  int ans;
  int A;
  int B;
  int C;
  int D;
  x1 = x >> 1;
  a = 1 << 31;
  c = ~a;
  ans1 = c & x1;
  ans2 = x << 1;
  A = ans1 ^ x;
  B = ans2 ^ x;
  C = A & x;
  D = B & x;
  ans = C & D;
  return ans;
}
/* 
 * countTrailingZero - return the number of consecutive 0 from the lowest bit of 
 *   the binary form of x.
 *   YOU MAY USE BIG CONST IN THIS PROBLEM, LIKE 0xFFFF0000
 *   YOU MAY USE BIG CONST IN THIS PROBLEM, LIKE 0xFFFF0000
 *   YOU MAY USE BIG CONST IN THIS PROBLEM, LIKE 0xFFFF0000
 *   Examples countTrailingZero(0x0) = 32, countTrailingZero(0x1) = 0,
 *            countTrailingZero(0xFFFF0000) = 16,
 *            countTrailingZero(0xFFFFFFF0) = 8,
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int countTrailingZero(int x){
  int ans;
  int bit;
  int flag;
  int zero;
  zero = (!(0 ^ x))<<31>>31;
  ans = 0;
  flag = (!(x & 0xFFFF))<<31>>31;
  bit = 16 & flag;
  ans = bit;
  x = x >> bit;
  flag = (!(x & 0xFF))<<31>>31;
  bit = 8 & flag;
  ans = ans + bit;
  x = x >> bit;
  flag = (!(x & 0xF))<<31>>31;
  bit = 4 & flag;
  ans = ans + bit;
  x = x >> bit;
  flag = (!(x & 0x3))<<31>>31;
  bit = 2 & flag;
  ans = ans + bit;
  x = x >> bit;
  flag = !(x & 0x1);
  bit = flag;
  ans = ans + bit;
  return (ans&(~zero))+(zero&32);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  int bia;
  int sign;
  int ans;
  bia =( 1 << n ) + (1<<31>>31);
  sign = x >> 31;
  ans =( x + ( sign & bia ) ) >> n;
  return ans;
}
/* 
 * oneMoreThan - return 1 if y is one more than x, and 0 otherwise
 *   Examples oneMoreThan(0, 1) = 1, oneMoreThan(-1, 1) = 0
 *   Legal ops: ~ & ! ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int oneMoreThan(int x, int y) {
  int negx;
  int del;
  int ans;
  int tmax;
  int tmin;
  int tag;
  tmax = ~(1<<31);
  tmin = 1<<31;
  tag =!((x ^ tmax)|(y ^ tmin));
  negx = ~x + 1;
  del = y + negx;
  ans =(! ( del ^ 1 ))^tag;
  return ans;
}
/*
 * satMul3 - multiplies by 3, saturating to Tmin or Tmax if overflow
 *  Examples: satMul3(0x10000000) = 0x30000000
 *            satMul3(0x30000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0x70000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0xD0000000) = 0x80000000 (Saturate to TMin)
 *            satMul3(0xA0000000) = 0x80000000 (Saturate to TMin)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 3
 */
int satMul3(int x) {
    int x2;
    int x3;
    int a;
    int flag1;//x2 overflow?if y, =0xFFFFFFFF, if n, =0x0
    int flag2;//x+x2 overflow?if y, =0xFFFFFFFF, if n, =0x0
    int flag;
    int sign;
    int ans0;
    int ans;
    int istmin;//if x == 0x10000000, = 00000000
    a = 1<<31;
    istmin =(!(x^a))<<31>>31;
    x2 = x << 1;
    x3 = x2 + x;
    sign = x >> 31;
    flag1 =(x2>>31)^sign;
    flag2 =(x3>>31)^sign;
    flag = flag1 | flag2;
    ans0 = (x3 & (~flag) ) + (flag & ((sign & a)+ ~(sign | a)));
    ans = (ans0 & (~istmin)) + (a & istmin);
    return ans;
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
  int negy;
  int del;
  int signx;
  int signy;
  int flag1;
  int flag2;
  int signd;
  int ans;
  int flag;
  flag = (!(y ^ (1<<31)));//y==1<<31 flag =0x00000001 else flag = 0x00000000
  negy = ~ y + 1;
  del = x + negy;
  signx = x >> 31;
  signy = negy >> 31;
  signd = del >> 31;
  flag1 = signx ^ signy;//=0 if signx == signy
  flag2 = signx ^ signd;//=0 if signx == signd, overflow <-> flag1==0 && flag2 ==0xFFFFFFFF
  ans = (((!((~flag1) & flag2)))&(~(flag<<31>>31)))+(flag & signx);
  return ans;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int signx;
  int signy;
  int del;
  int signd;
  int case1;
  int case2;
  int ans;
  signx = x >> 31;
  signy = y >> 31;
  del = y + ~x + 1;
  signd = del >> 31;
  case1 = (~signx) & signy;//case1 = FFFFFFFF if x>0 y<0
  case2 = signx & (~signy);//case2 = FFFFFFFF if x<0 y>0
  ans = ((case2)|((~case1)&(!signd)))&1;
  return ans;
}
/*
 * trueThreeFourths - multiplies by 3/4 rounding toward 0,
 *   avoiding errors due to overflow
 *   Examples: trueThreeFourths(11) = 8
 *             trueThreeFourths(-9) = -6
 *             trueThreeFourths(1073741824) = 805306368 (no overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int trueThreeFourths(int x){
  int sign;
  int left;
  int bias;
  int ans0;
  int value;
  int ans;
  sign = x >> 31;
  value = x;
  //value = (x & (~sign))+(sign&(~x+1));
  left = value & 0x3;
  value = value >> 2;
  bias = left + (~0) + !left;
  //bias = (2 & ((!(left^0x3))<<1)) + (1 & (!(left^0x2)));//left=11,bias=2;left=10,bias=1;else,bias=0;
  ans0 = bias + value + (value<<1) + ((sign & 1)&(!!left));
  ans = ans0;
  //ans = (ans0 & (~sign))+(sign&(~ans0+1));
  return ans;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  int sign = uf & 0x80000000;
  int exp = uf & 0x7F800000;
  int frac = uf & 0x007FFFFF;
  if(!(exp ^ 0x7F800000))
    return uf;
  if(!(exp ^ 0x0))
  {
    if(!(frac ^ 0x0))
      return (0x0|sign);
    frac = frac << 1;
    if(!((frac & 0x00800000) ^ 0x00800000))
    {
      frac = frac & 0x007FFFFF;
      exp = 0x00800000;
      return sign + exp + frac;
    }
    return sign + exp + frac;
  }
  if(!(exp ^ 0x7F000000))
    return sign ^ 0x07F800000;
  exp = exp + 0x00800000;
  return sign + exp + frac;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  int sign;
  int frac;
  int value;
  int exp;
  int b;
  int rou;
  int roustd;
  int fractmp;
  int fractmpp1;
  int tmp;
  int count = 0x0;
  int a = 0x80000000;
  sign = x & 0x80000000;
  value = x & 0x7FFFFFFF;
  switch (x)
  {
    case 0x0 :
      return 0x0;
    case 0x80000000:
      return 0xCF000000;
    default:
    {
  if(sign)
    value = -x;
  while((a & value) == 0)
  {
    a = a >> 1;
    count = count + 1;
  }
  tmp = 256 >> count;
  frac = ~a & value;
  if( count >= 8)//needn't to round
    frac = frac << (count - 8);
  else
  {
    b = tmp - 1;
    rou = b & frac;
    roustd = tmp >> 1;
    fractmp = frac >> (8 - count);
    fractmpp1 = fractmp + 1;
    if(rou < roustd)
      frac = fractmp;
    else if(rou > roustd)
      frac = fractmpp1;
    else
    {
      if((frac & tmp) == tmp)
        frac = fractmpp1;
      else
        frac = fractmp;
    }
    if(frac & 0x800000)
    {
      frac = 0;
      count = count - 1;
    }
  }
  count = 158 - count;
  exp = count << 23;
  return exp | frac | sign;
    }
  }
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  int sign = uf & 0x80000000;
  int exp = uf & 0x7F800000;
  int frac = uf & 0x007FFFFF;
  unsigned int E;
  int ans;
  if(exp == 0x7F800000)//ifinity and NaN
    return 0x80000000u;
  if(exp < 0x3F800000)//E < 0, too small
    return 0x0;
  if(exp >= 0x4F000000)//E >= 31, too big
    return 0x80000000u;
  if(exp == 0x3F800000)///E == 0
  {
    if(sign == 0x80000000)
      return 0xFFFFFFFF;//return -1
    return 0x1;//return 1
  }
  E = (exp >> 23) - 127;
  frac = frac | 0x00800000;
  if(E <= 23)
    ans = frac >> (23 - E);
  else
    ans = frac << (E - 23);
  if(sign == 0x80000000)
    ans = -ans;
  return ans;
}
/* 
 * float_pwr2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsidgned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned float_pwr2(int x) {
  if(x >= 128)
    return 0x7F800000;
  if(x < -149)
    return 0;
  if(x > -127)
    return (x + 127)<<23;
  else
    return 1<<(x + 149);
}
