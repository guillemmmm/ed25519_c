#ifndef FE10_INTERNAL_H
#define FE10_INTERNAL_H

#include "fixedint.h"


/*
    fe means field element.
    Here the field is \Z/(2^255-19).
    An element t, entries t[0]...t[9], represents the integer
    t[0]+2^26 t[1]+2^51 t[2]+2^77 t[3]+2^102 t[4]+...+2^230 t[9].
    Bounds on each t[i] vary depending on context.
*/


typedef int32_t fe10[10];


void fe10_0(fe10 h);
void fe10_1(fe10 h);

void fe10_frombytes(fe10 h, const unsigned char *s);
void fe10_tobytes(unsigned char *s, const fe10 h);

void fe10_copy(fe10 h, const fe10 f);
int fe10_isnegative(const fe10 f);
int fe10_isnonzero(const fe10 f);
void fe10_cmov(fe10 f, const fe10 g, unsigned int b);
void fe10_cswap(fe10 f, fe10 g, unsigned int b);

void fe10_neg(fe10 h, const fe10 f);
void fe10_add(fe10 h, const fe10 f, const fe10 g);
void fe10_invert(fe10 out, const fe10 z);
void fe10_sq(fe10 h, const fe10 f);
void fe10_sq2(fe10 h, const fe10 f);
void fe10_mul(fe10 h, const fe10 f, const fe10 g);
void fe10_mul121666(fe10 h, fe10 f);
void fe10_pow22523(fe10 out, const fe10 z);
void fe10_sub(fe10 h, const fe10 f, const fe10 g);

#endif
