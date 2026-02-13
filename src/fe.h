#ifndef FE_H
#define FE_H

#include "fixedint.h"

/*
    fe means field element.
    Here the field is \Z/(2^255-19).
    Internal representation in this variant uses 8 limbs of 32 bits
    (little-endian radix 2^32).
*/

typedef uint32_t fe[8];

void fe_0(fe h);
void fe_1(fe h);

void fe_frombytes(fe h, const unsigned char *s);
void fe_tobytes(unsigned char *s, const fe h);

void fe_copy(fe h, const fe f);
int fe_isnegative(const fe f);
int fe_isnonzero(const fe f);
void fe_cmov(fe f, const fe g, unsigned int b);
void fe_cswap(fe f, fe g, unsigned int b);

void fe_neg(fe h, const fe f);
void fe_add(fe h, const fe f, const fe g);
void fe_invert(fe out, const fe z);
void fe_sq(fe h, const fe f);
void fe_sq2(fe h, const fe f);
void fe_mul(fe h, const fe f, const fe g);
void fe_mul121666(fe h, fe f);
void fe_pow22523(fe out, const fe z);
void fe_sub(fe h, const fe f, const fe g);

#endif
