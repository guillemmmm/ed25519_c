#include "fixedint.h"
#include "fe.h"

#define FE_MASK UINT64_C(0x7ffffffffffff)

/*
    helper functions
*/
static uint64_t load_3(const unsigned char *in) {
    uint64_t result;

    result = (uint64_t) in[0];
    result |= ((uint64_t) in[1]) << 8;
    result |= ((uint64_t) in[2]) << 16;

    return result;
}

static uint64_t load_4(const unsigned char *in) {
    uint64_t result;

    result = (uint64_t) in[0];
    result |= ((uint64_t) in[1]) << 8;
    result |= ((uint64_t) in[2]) << 16;
    result |= ((uint64_t) in[3]) << 24;

    return result;
}

static uint64_t load_8(const unsigned char *in) {
    uint64_t result;

    result = (uint64_t) in[0];
    result |= ((uint64_t) in[1]) << 8;
    result |= ((uint64_t) in[2]) << 16;
    result |= ((uint64_t) in[3]) << 24;
    result |= ((uint64_t) in[4]) << 32;
    result |= ((uint64_t) in[5]) << 40;
    result |= ((uint64_t) in[6]) << 48;
    result |= ((uint64_t) in[7]) << 56;

    return result;
}

static void store_8(unsigned char *out, uint64_t in) {
    out[0] = (unsigned char) (in);
    out[1] = (unsigned char) (in >> 8);
    out[2] = (unsigned char) (in >> 16);
    out[3] = (unsigned char) (in >> 24);
    out[4] = (unsigned char) (in >> 32);
    out[5] = (unsigned char) (in >> 40);
    out[6] = (unsigned char) (in >> 48);
    out[7] = (unsigned char) (in >> 56);
}

static void fe_reduce(fe h) {
    uint64_t h0 = h[0];
    uint64_t h1 = h[1];
    uint64_t h2 = h[2];
    uint64_t h3 = h[3];
    uint64_t h4 = h[4];
    uint64_t carry0;
    uint64_t carry1;
    uint64_t carry2;
    uint64_t carry3;
    uint64_t carry4;

    carry0 = h0 >> 51;
    h1 += carry0;
    h0 &= FE_MASK;
    carry1 = h1 >> 51;
    h2 += carry1;
    h1 &= FE_MASK;
    carry2 = h2 >> 51;
    h3 += carry2;
    h2 &= FE_MASK;
    carry3 = h3 >> 51;
    h4 += carry3;
    h3 &= FE_MASK;
    carry4 = h4 >> 51;
    h0 += carry4 * 19;
    h4 &= FE_MASK;
    carry0 = h0 >> 51;
    h1 += carry0;
    h0 &= FE_MASK;

    h[0] = h0;
    h[1] = h1;
    h[2] = h2;
    h[3] = h3;
    h[4] = h4;
}

/*
    h = 0
*/

void fe_0(fe h) {
    h[0] = 0;
    h[1] = 0;
    h[2] = 0;
    h[3] = 0;
    h[4] = 0;
}



/*
    h = 1
*/

void fe_1(fe h) {
    h[0] = 1;
    h[1] = 0;
    h[2] = 0;
    h[3] = 0;
    h[4] = 0;
}



/*
    h = f + g
    Can overlap h with f or g.
*/

void fe_add(fe h, const fe f, const fe g) {
    h[0] = f[0] + g[0];
    h[1] = f[1] + g[1];
    h[2] = f[2] + g[2];
    h[3] = f[3] + g[3];
    h[4] = f[4] + g[4];
    fe_reduce(h);
}



/*
    Replace (f,g) with (g,g) if b == 1;
    replace (f,g) with (f,g) if b == 0.

    Preconditions: b in {0,1}.
*/

void fe_cmov(fe f, const fe g, unsigned int b) {
    uint64_t mask = (uint64_t) 0 - (uint64_t) b;

    f[0] ^= mask & (f[0] ^ g[0]);
    f[1] ^= mask & (f[1] ^ g[1]);
    f[2] ^= mask & (f[2] ^ g[2]);
    f[3] ^= mask & (f[3] ^ g[3]);
    f[4] ^= mask & (f[4] ^ g[4]);
}

/*
    Replace (f,g) with (g,f) if b == 1;
    replace (f,g) with (f,g) if b == 0.

    Preconditions: b in {0,1}.
*/

void fe_cswap(fe f, fe g, unsigned int b) {
    uint64_t mask = (uint64_t) 0 - (uint64_t) b;
    uint64_t x0 = mask & (f[0] ^ g[0]);
    uint64_t x1 = mask & (f[1] ^ g[1]);
    uint64_t x2 = mask & (f[2] ^ g[2]);
    uint64_t x3 = mask & (f[3] ^ g[3]);
    uint64_t x4 = mask & (f[4] ^ g[4]);

    f[0] ^= x0;
    f[1] ^= x1;
    f[2] ^= x2;
    f[3] ^= x3;
    f[4] ^= x4;
    g[0] ^= x0;
    g[1] ^= x1;
    g[2] ^= x2;
    g[3] ^= x3;
    g[4] ^= x4;
}



/*
    h = f
*/

void fe_copy(fe h, const fe f) {
    h[0] = f[0];
    h[1] = f[1];
    h[2] = f[2];
    h[3] = f[3];
    h[4] = f[4];
}



/*
    Ignores top bit of h.
*/

void fe_frombytes(fe h, const unsigned char *s) {
    uint64_t h0 = load_8(s) & FE_MASK;
    uint64_t h1 = (load_8(s + 6) >> 3) & FE_MASK;
    uint64_t h2 = (load_8(s + 12) >> 6) & FE_MASK;
    uint64_t h3 = (load_8(s + 19) >> 1) & FE_MASK;
    uint64_t h4 = (load_8(s + 24) >> 12) & FE_MASK;

    h[0] = h0;
    h[1] = h1;
    h[2] = h2;
    h[3] = h3;
    h[4] = h4;
    fe_reduce(h);
}



void fe_invert(fe out, const fe z) {
    fe t0;
    fe t1;
    fe t2;
    fe t3;
    int i;

    fe_sq(t0, z);

    for (i = 1; i < 1; ++i) {
        fe_sq(t0, t0);
    }

    fe_sq(t1, t0);

    for (i = 1; i < 2; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t2, t0);

    for (i = 1; i < 1; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t1, t2);
    fe_sq(t2, t1);

    for (i = 1; i < 5; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);

    for (i = 1; i < 10; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);

    for (i = 1; i < 20; ++i) {
        fe_sq(t3, t3);
    }

    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);

    for (i = 1; i < 10; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);

    for (i = 1; i < 50; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);

    for (i = 1; i < 100; ++i) {
        fe_sq(t3, t3);
    }

    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);

    for (i = 1; i < 50; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);

    for (i = 1; i < 5; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(out, t1, t0);
}



int fe_isnegative(const fe f) {
    unsigned char s[32];

    fe_tobytes(s, f);
    return s[0] & 1;
}



int fe_isnonzero(const fe f) {
    unsigned char s[32];
    unsigned int i;
    unsigned char c = 0;

    fe_tobytes(s, f);

    for (i = 0; i < 32; ++i) {
        c |= s[i];
    }

    return (int) ((c - 1) >> 8) + 1;
}



/*
    h = f * g
    Can overlap h with f or g.
*/

void fe_mul(fe h, const fe f, const fe g) {
    uint64_t f0 = f[0];
    uint64_t f1 = f[1];
    uint64_t f2 = f[2];
    uint64_t f3 = f[3];
    uint64_t f4 = f[4];
    uint64_t g0 = g[0];
    uint64_t g1 = g[1];
    uint64_t g2 = g[2];
    uint64_t g3 = g[3];
    uint64_t g4 = g[4];
    uint64_t f1_19 = f1 * 19;
    uint64_t f2_19 = f2 * 19;
    uint64_t f3_19 = f3 * 19;
    uint64_t f4_19 = f4 * 19;
    unsigned __int128 h0 = (unsigned __int128) f0 * g0 + (unsigned __int128) f1_19 * g4 + (unsigned __int128) f2_19 * g3 + (unsigned __int128) f3_19 * g2 + (unsigned __int128) f4_19 * g1;
    unsigned __int128 h1 = (unsigned __int128) f0 * g1 + (unsigned __int128) f1 * g0 + (unsigned __int128) f2_19 * g4 + (unsigned __int128) f3_19 * g3 + (unsigned __int128) f4_19 * g2;
    unsigned __int128 h2 = (unsigned __int128) f0 * g2 + (unsigned __int128) f1 * g1 + (unsigned __int128) f2 * g0 + (unsigned __int128) f3_19 * g4 + (unsigned __int128) f4_19 * g3;
    unsigned __int128 h3 = (unsigned __int128) f0 * g3 + (unsigned __int128) f1 * g2 + (unsigned __int128) f2 * g1 + (unsigned __int128) f3 * g0 + (unsigned __int128) f4_19 * g4;
    unsigned __int128 h4 = (unsigned __int128) f0 * g4 + (unsigned __int128) f1 * g3 + (unsigned __int128) f2 * g2 + (unsigned __int128) f3 * g1 + (unsigned __int128) f4 * g0;
    uint64_t carry0;
    uint64_t carry1;
    uint64_t carry2;
    uint64_t carry3;
    uint64_t carry4;

    carry0 = (uint64_t) (h0 >> 51);
    h1 += carry0;
    h0 &= FE_MASK;
    carry1 = (uint64_t) (h1 >> 51);
    h2 += carry1;
    h1 &= FE_MASK;
    carry2 = (uint64_t) (h2 >> 51);
    h3 += carry2;
    h2 &= FE_MASK;
    carry3 = (uint64_t) (h3 >> 51);
    h4 += carry3;
    h3 &= FE_MASK;
    carry4 = (uint64_t) (h4 >> 51);
    h0 += (unsigned __int128) carry4 * 19;
    h4 &= FE_MASK;
    carry0 = (uint64_t) (h0 >> 51);
    h1 += carry0;
    h0 &= FE_MASK;

    h[0] = (uint64_t) h0;
    h[1] = (uint64_t) h1;
    h[2] = (uint64_t) h2;
    h[3] = (uint64_t) h3;
    h[4] = (uint64_t) h4;
}



void fe_mul121666(fe h, fe f) {
    unsigned __int128 h0 = (unsigned __int128) f[0] * 121666;
    unsigned __int128 h1 = (unsigned __int128) f[1] * 121666;
    unsigned __int128 h2 = (unsigned __int128) f[2] * 121666;
    unsigned __int128 h3 = (unsigned __int128) f[3] * 121666;
    unsigned __int128 h4 = (unsigned __int128) f[4] * 121666;
    uint64_t carry0;
    uint64_t carry1;
    uint64_t carry2;
    uint64_t carry3;
    uint64_t carry4;

    carry0 = (uint64_t) (h0 >> 51);
    h1 += carry0;
    h0 &= FE_MASK;
    carry1 = (uint64_t) (h1 >> 51);
    h2 += carry1;
    h1 &= FE_MASK;
    carry2 = (uint64_t) (h2 >> 51);
    h3 += carry2;
    h2 &= FE_MASK;
    carry3 = (uint64_t) (h3 >> 51);
    h4 += carry3;
    h3 &= FE_MASK;
    carry4 = (uint64_t) (h4 >> 51);
    h0 += (unsigned __int128) carry4 * 19;
    h4 &= FE_MASK;
    carry0 = (uint64_t) (h0 >> 51);
    h1 += carry0;
    h0 &= FE_MASK;

    h[0] = (uint64_t) h0;
    h[1] = (uint64_t) h1;
    h[2] = (uint64_t) h2;
    h[3] = (uint64_t) h3;
    h[4] = (uint64_t) h4;
}



void fe_neg(fe h, const fe f) {
    fe zero;

    fe_0(zero);
    fe_sub(h, zero, f);
}



void fe_pow22523(fe out, const fe z) {
    fe t0;
    fe t1;
    fe t2;
    int i;

    fe_sq(t0, z);

    for (i = 1; i < 1; ++i) {
        fe_sq(t0, t0);
    }

    fe_sq(t1, t0);

    for (i = 1; i < 2; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t0, t0);

    for (i = 1; i < 1; ++i) {
        fe_sq(t0, t0);
    }

    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);

    for (i = 1; i < 5; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);

    for (i = 1; i < 10; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t1, t1, t0);
    fe_sq(t2, t1);

    for (i = 1; i < 20; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);

    for (i = 1; i < 10; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);

    for (i = 1; i < 50; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t1, t1, t0);
    fe_sq(t2, t1);

    for (i = 1; i < 100; ++i) {
        fe_sq(t2, t2);
    }

    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);

    for (i = 1; i < 50; ++i) {
        fe_sq(t1, t1);
    }

    fe_mul(t0, t1, t0);
    fe_sq(t0, t0);

    for (i = 1; i < 2; ++i) {
        fe_sq(t0, t0);
    }

    fe_mul(out, t0, z);
}



void fe_sq(fe h, const fe f) {
    fe_mul(h, f, f);
}



void fe_sq2(fe h, const fe f) {
    fe_mul(h, f, f);
    fe_add(h, h, h);
}



void fe_sub(fe h, const fe f, const fe g) {
    const uint64_t two_p0 = (UINT64_C(1) << 52) - 38;
    const uint64_t two_pn = (UINT64_C(1) << 52) - 2;

    h[0] = f[0] + two_p0 - g[0];
    h[1] = f[1] + two_pn - g[1];
    h[2] = f[2] + two_pn - g[2];
    h[3] = f[3] + two_pn - g[3];
    h[4] = f[4] + two_pn - g[4];
    fe_reduce(h);
}



void fe_tobytes(unsigned char *s, const fe h) {
    uint64_t h0 = h[0];
    uint64_t h1 = h[1];
    uint64_t h2 = h[2];
    uint64_t h3 = h[3];
    uint64_t h4 = h[4];
    uint64_t q;
    uint64_t carry0;
    uint64_t carry1;
    uint64_t carry2;
    uint64_t carry3;
    uint64_t carry4;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;

    q = (19 * h4 + (UINT64_C(1) << 24)) >> 51;
    q = (h0 + q) >> 51;
    q = (h1 + q) >> 51;
    q = (h2 + q) >> 51;
    q = (h3 + q) >> 51;
    q = (h4 + q) >> 51;

    h0 += 19 * q;
    carry0 = h0 >> 51;
    h1 += carry0;
    h0 &= FE_MASK;
    carry1 = h1 >> 51;
    h2 += carry1;
    h1 &= FE_MASK;
    carry2 = h2 >> 51;
    h3 += carry2;
    h2 &= FE_MASK;
    carry3 = h3 >> 51;
    h4 += carry3;
    h3 &= FE_MASK;
    carry4 = h4 >> 51;
    h0 += carry4 * 19;
    h4 &= FE_MASK;
    carry0 = h0 >> 51;
    h1 += carry0;
    h0 &= FE_MASK;

    t0 = h0 | (h1 << 51);
    t1 = (h1 >> 13) | (h2 << 38);
    t2 = (h2 >> 26) | (h3 << 25);
    t3 = (h3 >> 39) | (h4 << 12);

    store_8(s, t0);
    store_8(s + 8, t1);
    store_8(s + 16, t2);
    store_8(s + 24, t3);
}
