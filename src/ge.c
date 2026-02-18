#include "ge.h"


/*
r = p + q
*/


void ge_add(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q) {
    //fe_add(r->X, p->Y, p->X);
    r->X = p->X + p->Y;
    //fe_sub(r->Y, p->Y, p->X);
    uint32_t trash;
    r->Y = vsub(p->Y, p->X, trash);
    //fe_mul(r->Z, r->X, q->YplusX);
    r->Z = r->X * q->YplusX;
    //fe_mul(r->Y, r->Y, q->YminusX);
    r->Y = r->Y * q->YminusX;
    //fe_mul(r->T, q->T2d, p->T);
    r->T = q->T2d * p->T;
    //fe_mul(r->X, p->Z, q->Z);
    r->X = p->Z * q->Z;
    //fe_add(t0, r->X, r->X);
    fe tt;
    tt = vdot(r->X, r->X);
    //fe_sub(r->X, r->Z, r->Y);
    uint32_t trash1;
    r->X = vsub(r->Z, r->Y, trash1);
    //fe_add(r->Y, r->Z, r->Y);
    r->Y = r->Z + r->Y;
    //fe_add(r->Z, t0, r->T);
    r->Z = tt + r->T;
    //fe_sub(r->T, t0, r->T);
    uint32_t trash2;
    r->T = vsub(tt, r->T, trash2);
}



static void slide(signed char *r, const unsigned char *a) {
    int i;
    int b;
    int k;

    for (i = 0; i < 256; ++i) {
        r[i] = 1 & (a[i >> 3] >> (i & 7));
    }

    for (i = 0; i < 256; ++i)
        if (r[i]) {
            for (b = 1; b <= 6 && i + b < 256; ++b) {
                if (r[i + b]) {
                    if (r[i] + (r[i + b] << b) <= 15) {
                        r[i] += r[i + b] << b;
                        r[i + b] = 0;
                    } else if (r[i] - (r[i + b] << b) >= -15) {
                        r[i] -= r[i + b] << b;

                        for (k = i + b; k < 256; ++k) {
                            if (!r[k]) {
                                r[k] = 1;
                                break;
                            }

                            r[k] = 0;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
}

/*
r = a * A + b * B
where a = a[0]+256*a[1]+...+256^31 a[31].
and b = b[0]+256*b[1]+...+256^31 b[31].
B is the Ed25519 base point (x,4/5) with x positive.
*/

void ge_double_scalarmult_vartime(ge_p2 *r, const unsigned char *a, const ge_p3 *A, const unsigned char *b) {
    signed char aslide[256];
    signed char bslide[256];
    ge_cached Ai[8]; /* A,3A,5A,7A,9A,11A,13A,15A */
    ge_cached Bi[8]; /* B,3B,5B,7B,9B,11B,13B,15B */
    ge_p1p1 t;
    ge_p3 u;
    ge_p3 A2;
    ge_p3 B;
    ge_p3 B2;
    int i;
    static const unsigned char basepoint[32] = {
        0x58, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6
    };

    ge_frombytes_negate_vartime(&B, basepoint);

    slide(aslide, a);
    slide(bslide, b);

    ge_p3_to_cached(&Ai[0], A);
    ge_p3_dbl(&t, A);
    ge_p1p1_to_p3(&A2, &t);
    ge_add(&t, &A2, &Ai[0]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[1], &u);
    ge_add(&t, &A2, &Ai[1]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[2], &u);
    ge_add(&t, &A2, &Ai[2]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[3], &u);
    ge_add(&t, &A2, &Ai[3]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[4], &u);
    ge_add(&t, &A2, &Ai[4]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[5], &u);
    ge_add(&t, &A2, &Ai[5]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[6], &u);
    ge_add(&t, &A2, &Ai[6]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Ai[7], &u);

    ge_p3_to_cached(&Bi[0], &B);
    ge_p3_dbl(&t, &B);
    ge_p1p1_to_p3(&B2, &t);
    ge_add(&t, &B2, &Bi[0]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[1], &u);
    ge_add(&t, &B2, &Bi[1]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[2], &u);
    ge_add(&t, &B2, &Bi[2]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[3], &u);
    ge_add(&t, &B2, &Bi[3]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[4], &u);
    ge_add(&t, &B2, &Bi[4]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[5], &u);
    ge_add(&t, &B2, &Bi[5]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[6], &u);
    ge_add(&t, &B2, &Bi[6]);
    ge_p1p1_to_p3(&u, &t);
    ge_p3_to_cached(&Bi[7], &u);

    ge_p2_0(r);

    for (i = 255; i >= 0; --i) {
        if (aslide[i] || bslide[i]) {
            break;
        }
    }

    for (; i >= 0; --i) {
        ge_p2_dbl(&t, r);

        if (aslide[i] > 0) {
            ge_p1p1_to_p3(&u, &t);
            ge_add(&t, &u, &Ai[aslide[i] / 2]);
        } else if (aslide[i] < 0) {
            ge_p1p1_to_p3(&u, &t);
            ge_sub(&t, &u, &Ai[(-aslide[i]) / 2]);
        }

        if (bslide[i] > 0) {
            ge_p1p1_to_p3(&u, &t);
            ge_add(&t, &u, &Bi[bslide[i] / 2]);
        } else if (bslide[i] < 0) {
            ge_p1p1_to_p3(&u, &t);
            ge_sub(&t, &u, &Bi[(-bslide[i]) / 2]);
        }

        ge_p1p1_to_p2(r, &t);
    }
}

/*
static const fe d = {
    -10913610, 13857413, -15372611, 6949391, 114729, -8787816, -6275908, -3247719, -18696448, -12055116
};
*/

static fe load_d(void){
    fe d = chess_dont_care(vdata);
    d = vupd_mem(d, 0, 0x135978A3);
    d = vupd_mem(d, 1, 0x75EB4DCA);
    d = vupd_mem(d, 2, 0x4141D8AB);
    d = vupd_mem(d, 3, 0x00700A4D);
    d = vupd_mem(d, 4, 0x7779E898);
    d = vupd_mem(d, 5, 0x8CC74079);
    d = vupd_mem(d, 6, 0x2B6FFE73);
    d = vupd_mem(d, 7, 0x52036CEE);
    return d;
}

/*
static const fe sqrtm1 = {
    -32595792, -7943725, 9377950, 3500415, 12389472, -272473, -25146209, -2005654, 326686, 11406482
};
*/

static fe load_sqrtm1(void){
    fe sqrtm1 = chess_dont_care(vdata);
    sqrtm1 = vupd_mem(sqrtm1, 0, 0x4A0EA0B0);
    sqrtm1 = vupd_mem(sqrtm1, 1, 0xC4EE1B27);
    //sqrtm1 = vupd_mem(sqrtm1, 2, 0xAD2FE478);
    //sqrtm1 = vupd_mem(sqrtm1, 3, 0x2F431806);
    sqrtm1 = vupd_mem(sqrtm1, 4, 0x3DFBD7A7);
    sqrtm1 = vupd_mem(sqrtm1, 5, 0x2B4D0099);
    sqrtm1 = vupd_mem(sqrtm1, 6, 0x4FC1DF0B);
    sqrtm1 = vupd_mem(sqrtm1, 7, 0x2B832480);
    return sqrtm1;
}

int ge_frombytes_negate_vartime(ge_p3 *h, const unsigned char *s) {
    fe u;
    fe v;
    fe v3;
    fe vxx;
    fe check;
    fe_frombytes(&h->Y, s);
    fe_1(&h->Z);
    // fe_sq(u, h->Y);
    u = h->Y * h->Y;
    
    fe d = load_d();
    // fe_mul(v, u, d);
    v = u * d;
    
    // fe_sub(u, u, h->Z);     /* u = y^2-1 */
    uint32_t trash;
    u = vsub(u, h->Z, trash);
    
    // fe_add(v, v, h->Z);     /* v = dy^2+1 */
    v = v + h->Z;
    
    // fe_sq(v3, v);
    v3 = v * v;
    
    // fe_mul(v3, v3, v);      /* v3 = v^3 */
    v3 = v3 * v;
    
    // fe_sq(h->X, v3);
    h->X = v3 * v3;
    
    // fe_mul(h->X, h->X, v);
    h->X = h->X * v;
    
    // fe_mul(h->X, h->X, u);  /* x = uv^7 */
    h->X = h->X * u;
    
    // fe_pow22523(h->X, h->X); /* x = (uv^7)^((q-5)/8) */
    fe_pow22523(&h->X, h->X);
    
    // fe_mul(h->X, h->X, v3);
    h->X = h->X * v3;
    
    // fe_mul(h->X, h->X, u);  /* x = uv^3(uv^7)^((q-5)/8) */
    h->X = h->X * u;
    
    // fe_sq(vxx, h->X);
    vxx = h->X * h->X;
    
    // fe_mul(vxx, vxx, v);
    vxx = vxx * v;
    //fe_sub(check, vxx, u);  /* vx^2-u */
    uint32_t trash1;
    check = vsub(vxx,u,trash1);

    if (fe_isnonzero(check)) {
        //fe_add(check, vxx, u); /* vx^2+u */
        check = vxx + u;

        if (fe_isnonzero(check)) {
            return -1;
        }

        //fe_mul(h->X, h->X, sqrtm1);
        fe sqrtm1 = load_sqrtm1();
        h->X = h->X * sqrtm1;
    }

    if (fe_isnegative(h->X) == (s[31] >> 7)) {
        fe_neg(&h->X, h->X);
    }

    //fe_mul(h->T, h->X, h->Y);
    h->T = h->X*h->Y;
    return 0;
}


/*
r = p + q
*/

void ge_madd(ge_p1p1 *r, const ge_p3 *p, const ge_precomp *q) {
    fe t0;
    // fe_add(r->X, p->Y, p->X);
    r->X = p->Y + p->X;
    
    // fe_sub(r->Y, p->Y, p->X);
    uint32_t trash;
    r->Y = vsub(p->Y, p->X, trash);
    
    // fe_mul(r->Z, r->X, q->yplusx);
    r->Z = r->X * q->yplusx;
    
    // fe_mul(r->Y, r->Y, q->yminusx);
    r->Y = r->Y * q->yminusx;
    
    // fe_mul(r->T, q->xy2d, p->T);
    r->T = q->xy2d * p->T;
    
    // fe_add(t0, p->Z, p->Z);
    t0 = vdot(p->Z, p->Z);
    
    // fe_sub(r->X, r->Z, r->Y);
    uint32_t trash1;
    r->X = vsub(r->Z, r->Y, trash1);
    
    // fe_add(r->Y, r->Z, r->Y);
    r->Y = r->Z + r->Y;
    
    // fe_add(r->Z, t0, r->T);
    r->Z = t0 + r->T;
    
    // fe_sub(r->T, t0, r->T);
    uint32_t trash2;
    r->T = vsub(t0, r->T, trash2);
}


/*
r = p - q
*/

void ge_msub(ge_p1p1 *r, const ge_p3 *p, const ge_precomp *q) {
    fe t0;

    // fe_add(r->X, p->Y, p->X);
    r->X = p->Y + p->X;
    
    // fe_sub(r->Y, p->Y, p->X);
    uint32_t trash;
    r->Y = vsub(p->Y, p->X, trash);
    
    // fe_mul(r->Z, r->X, q->yminusx);
    r->Z = r->X * q->yminusx;
    
    // fe_mul(r->Y, r->Y, q->yplusx);
    r->Y = r->Y * q->yplusx;
    
    // fe_mul(r->T, q->xy2d, p->T);
    r->T = q->xy2d * p->T;
    
    // fe_add(t0, p->Z, p->Z);
    t0 = vdot(p->Z, p->Z);
    
    // fe_sub(r->X, r->Z, r->Y);
    uint32_t trash1;
    r->X = vsub(r->Z, r->Y, trash1);
    
    // fe_add(r->Y, r->Z, r->Y);
    r->Y = r->Z + r->Y;
    
    // fe_sub(r->Z, t0, r->T);
    uint32_t trash2;
    r->Z = vsub(t0, r->T, trash2);
    
    // fe_add(r->T, t0, r->T);
    r->T = t0 + r->T;
}


/*
r = p
*/

void ge_p1p1_to_p2(ge_p2 *r, const ge_p1p1 *p) {
    // fe_mul(r->X, p->X, p->T);
    r->X = p->X * p->T;
    
    // fe_mul(r->Y, p->Y, p->Z);
    r->Y = p->Y * p->Z;
    
    // fe_mul(r->Z, p->Z, p->T);
    r->Z = p->Z * p->T;
}



/*
r = p
*/

void ge_p1p1_to_p3(ge_p3 *r, const ge_p1p1 *p) {
    // fe_mul(r->X, p->X, p->T);
    r->X = p->X * p->T;
    
    // fe_mul(r->Y, p->Y, p->Z);
    r->Y = p->Y * p->Z;
    
    // fe_mul(r->Z, p->Z, p->T);
    r->Z = p->Z * p->T;
    
    // fe_mul(r->T, p->X, p->Y);
    r->T = p->X * p->Y;
}


void ge_p2_0(ge_p2 *h) {
    fe_0(&h->X);
    fe_1(&h->Y);
    fe_1(&h->Z);
}



/*
r = 2 * p
*/

void ge_p2_dbl(ge_p1p1 *r, const ge_p2 *p) {
    fe t0;

    // fe_sq(r->X, p->X);
    r->X = p->X * p->X;
    
    // fe_sq(r->Z, p->Y);
    r->Z = p->Y * p->Y;
    
    fe_sq2(&r->T, p->Z);   // sq2 = 2*(z^2)
    
    // fe_add(r->Y, p->X, p->Y);
    r->Y = p->X + p->Y;
    
    // fe_sq(t0, r->Y);
    t0 = r->Y * r->Y;
    
    // fe_add(r->Y, r->Z, r->X);
    r->Y = r->Z + r->X;
    
    // fe_sub(r->Z, r->Z, r->X);
    uint32_t trash;
    r->Z = vsub(r->Z, r->X, trash);
    
    // fe_sub(r->X, t0, r->Y);
    uint32_t trash1;
    r->X = vsub(t0, r->Y, trash1);
    
    // fe_sub(r->T, r->T, r->Z);
    uint32_t trash2;
    r->T = vsub(r->T, r->Z, trash2);
}


void ge_p3_0(ge_p3 *h) {
    fe_0(&h->X);
    fe_1(&h->Y);
    fe_1(&h->Z);
    fe_0(&h->T);
}


/*
r = 2 * p
*/

void ge_p3_dbl(ge_p1p1 *r, const ge_p3 *p) {
    ge_p2 q;
    ge_p3_to_p2(&q, p);
    ge_p2_dbl(r, &q);
}



/*
r = p
*/

static fe load_d2(void){
    fe d = chess_dont_care(vdata);
    //d = vupd_mem(d, 0, 0x26B2F159);
    d = vupd_mem(d, 1, 0xEBD69B94);
    d = vupd_mem(d, 2, 0x8283B156);
    d = vupd_mem(d, 3, 0x00E0149A);
    d = vupd_mem(d, 4, 0xEEF3D130);
    d = vupd_mem(d, 5, 0x198E80F2);
    d = vupd_mem(d, 6, 0x56DFFCE7);
    d = vupd_mem(d, 7, 0x2406D9DC);
    return d;
}
/*
static const fe d2 = {
    -21827239, -5839606, -30745221, 13898782, 229458, 15978800, -12551817, -6495438, 29715968, 9444199
};
*/
void ge_p3_to_cached(ge_cached *r, const ge_p3 *p) {
    //fe_add(r->YplusX, p->Y, p->X);
    r->YplusX = p->Y + p->X;
    //fe_sub(r->YminusX, p->Y, p->X);
    uint32_t trash;
    r->YminusX = vsub(p->Y, p->X, trash);
    //fe_copy(r->Z, p->Z);
    r->Z = p->Z;
    fe d2 = load_d2();
    //fe_mul(r->T2d, p->T, d2);
    r->T2d = p->T * d2;
}


/*
r = p
*/

void ge_p3_to_p2(ge_p2 *r, const ge_p3 *p) {
    //fe_copy(r->X, p->X);
    r->X = p->X;
    //fe_copy(r->Y, p->Y);
    r->Y = p->Y;
    //fe_copy(r->Z, p->Z);
    r->Z = p->Z;
}


void ge_p3_tobytes(unsigned char *s, const ge_p3 *h) {
    fe recip = chess_dont_care(vdata);
    fe x;
    fe y;
    fe_invert(&recip, h->Z);
    //fe_mul(x, h->X, recip);
    x = h->X * recip;
    //fe_mul(y, h->Y, recip);
    y = h->Y * recip;
    fe_tobytes(s, y);
    s[31] ^= fe_isnegative(x) << 7;
}




/*
h = a * B
where a = a[0]+256*a[1]+...+256^31 a[31]
B is the Ed25519 base point (x,4/5) with x positive.

Preconditions:
  a[31] <= 127
*/

void ge_scalarmult_base(ge_p3 *h, const unsigned char *a) {
    ge_p3 q;
    ge_p3 r3;
    ge_cached qc;
    ge_p1p1 t;
    static const unsigned char basepoint[32] = {
        0x58, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6
    };
    int i;

    ge_p3_0(h);
    ge_frombytes_negate_vartime(&q, basepoint);

    for (i = 0; i < 256; ++i) {
        if ((a[i >> 3] >> (i & 7)) & 1) {
            ge_p3_to_cached(&qc, &q);
            ge_add(&t, h, &qc);
            ge_p1p1_to_p3(h, &t);
        }

        ge_p3_dbl(&t, &q);
        ge_p1p1_to_p3(&r3, &t);
        q = r3;
    }
}


/*
r = p - q
*/

void ge_sub(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q) {
    fe t0;
    
    // fe_add(r->X, p->Y, p->X);
    r->X = p->Y + p->X;
    
    // fe_sub(r->Y, p->Y, p->X);
    uint32_t trash;
    r->Y = vsub(p->Y, p->X, trash);
    
    // fe_mul(r->Z, r->X, q->YminusX);
    r->Z = r->X * q->YminusX;
    
    // fe_mul(r->Y, r->Y, q->YplusX);
    r->Y = r->Y * q->YplusX;
    
    // fe_mul(r->T, q->T2d, p->T);
    r->T = q->T2d * p->T;
    
    // fe_mul(r->X, p->Z, q->Z);
    r->X = p->Z * q->Z;
    
    // fe_add(t0, r->X, r->X);
    t0 = vdot(r->X, r->X);
    
    // fe_sub(r->X, r->Z, r->Y);
    uint32_t trash1;
    r->X = vsub(r->Z, r->Y, trash1);
    
    // fe_add(r->Y, r->Z, r->Y);
    r->Y = r->Z + r->Y;
    
    // fe_sub(r->Z, t0, r->T);
    uint32_t trash2;
    r->Z = vsub(t0, r->T, trash2);
    
    // fe_add(r->T, t0, r->T);
    r->T = t0 + r->T;
}


void ge_tobytes(unsigned char *s, const ge_p2 *h) {
    fe recip = chess_dont_care(vdata);
    fe x;
    fe y;
    fe_invert(&recip, h->Z);
    //fe_mul(x, h->X, recip);
    x = h->X * recip;
    //fe_mul(y, h->Y, recip);
    y = h->Y * recip;
    fe_tobytes(s, y);
    s[31] ^= fe_isnegative(x) << 7;
}
