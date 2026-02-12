#include "fixedint.h"
#include "fe.h"

#define FE51_MASK UINT64_C(0x7ffffffffffff)

typedef uint64_t fe51[5];

typedef struct {
    uint64_t lo;
    uint64_t hi;
} fe_u128;

static uint64_t load_4(const unsigned char *in) {
    uint64_t result = (uint64_t) in[0];
    result |= ((uint64_t) in[1]) << 8;
    result |= ((uint64_t) in[2]) << 16;
    result |= ((uint64_t) in[3]) << 24;
    return result;
}

static uint64_t load_8(const unsigned char *in) {
    uint64_t result = (uint64_t) in[0];
    result |= ((uint64_t) in[1]) << 8;
    result |= ((uint64_t) in[2]) << 16;
    result |= ((uint64_t) in[3]) << 24;
    result |= ((uint64_t) in[4]) << 32;
    result |= ((uint64_t) in[5]) << 40;
    result |= ((uint64_t) in[6]) << 48;
    result |= ((uint64_t) in[7]) << 56;
    return result;
}

static void store_4(unsigned char *out, uint32_t in) {
    out[0] = (unsigned char) in;
    out[1] = (unsigned char) (in >> 8);
    out[2] = (unsigned char) (in >> 16);
    out[3] = (unsigned char) (in >> 24);
}

static void store_8(unsigned char *out, uint64_t in) {
    out[0] = (unsigned char) in;
    out[1] = (unsigned char) (in >> 8);
    out[2] = (unsigned char) (in >> 16);
    out[3] = (unsigned char) (in >> 24);
    out[4] = (unsigned char) (in >> 32);
    out[5] = (unsigned char) (in >> 40);
    out[6] = (unsigned char) (in >> 48);
    out[7] = (unsigned char) (in >> 56);
}

static fe_u128 u128_add(fe_u128 a, fe_u128 b) {
    fe_u128 out;
    out.lo = a.lo + b.lo;
    out.hi = a.hi + b.hi + (out.lo < a.lo);
    return out;
}

static fe_u128 u128_add_u64(fe_u128 a, uint64_t b) {
    fe_u128 out;
    out.lo = a.lo + b;
    out.hi = a.hi + (out.lo < a.lo);
    return out;
}

static fe_u128 u128_mul_u64(uint64_t a, uint64_t b) {
    uint64_t a0 = a & UINT64_C(0xffffffff);
    uint64_t a1 = a >> 32;
    uint64_t b0 = b & UINT64_C(0xffffffff);
    uint64_t b1 = b >> 32;
    uint64_t p0 = a0 * b0;
    uint64_t p1 = a0 * b1;
    uint64_t p2 = a1 * b0;
    uint64_t p3 = a1 * b1;
    uint64_t mid = p1 + p2;
    uint64_t mid_carry = (mid < p1);
    uint64_t lo = p0 + (mid << 32);
    uint64_t lo_carry = (lo < p0);
    fe_u128 out;

    out.lo = lo;
    out.hi = p3 + (mid >> 32) + (mid_carry << 32) + lo_carry;
    return out;
}

static fe_u128 u128_add_mul(fe_u128 acc, uint64_t a, uint64_t b) {
    return u128_add(acc, u128_mul_u64(a, b));
}

static uint64_t u128_shr51(fe_u128 a) {
    return (a.hi << 13) | (a.lo >> 51);
}

static void fe51_reduce(fe51 h) {
    uint64_t c;

    c = h[0] >> 51; h[0] &= FE51_MASK; h[1] += c;
    c = h[1] >> 51; h[1] &= FE51_MASK; h[2] += c;
    c = h[2] >> 51; h[2] &= FE51_MASK; h[3] += c;
    c = h[3] >> 51; h[3] &= FE51_MASK; h[4] += c;
    c = h[4] >> 51; h[4] &= FE51_MASK; h[0] += c * 19;
    c = h[0] >> 51; h[0] &= FE51_MASK; h[1] += c;
}

static void fe51_add(fe51 h, const fe51 f, const fe51 g) {
    h[0] = f[0] + g[0];
    h[1] = f[1] + g[1];
    h[2] = f[2] + g[2];
    h[3] = f[3] + g[3];
    h[4] = f[4] + g[4];
    fe51_reduce(h);
}

static void fe51_sub(fe51 h, const fe51 f, const fe51 g) {
    const uint64_t two_p0 = (UINT64_C(1) << 52) - 38;
    const uint64_t two_pn = (UINT64_C(1) << 52) - 2;

    h[0] = f[0] + two_p0 - g[0];
    h[1] = f[1] + two_pn - g[1];
    h[2] = f[2] + two_pn - g[2];
    h[3] = f[3] + two_pn - g[3];
    h[4] = f[4] + two_pn - g[4];
    fe51_reduce(h);
}

static void fe51_mul(fe51 h, const fe51 f, const fe51 g) {
    uint64_t f0 = f[0], f1 = f[1], f2 = f[2], f3 = f[3], f4 = f[4];
    uint64_t g0 = g[0], g1 = g[1], g2 = g[2], g3 = g[3], g4 = g[4];
    uint64_t f1_19 = f1 * 19, f2_19 = f2 * 19, f3_19 = f3 * 19, f4_19 = f4 * 19;
    fe_u128 h0 = {0,0}, h1 = {0,0}, h2 = {0,0}, h3 = {0,0}, h4 = {0,0};
    uint64_t c;

    h0 = u128_add_mul(h0, f0, g0);
    h0 = u128_add_mul(h0, f1_19, g4);
    h0 = u128_add_mul(h0, f2_19, g3);
    h0 = u128_add_mul(h0, f3_19, g2);
    h0 = u128_add_mul(h0, f4_19, g1);

    h1 = u128_add_mul(h1, f0, g1);
    h1 = u128_add_mul(h1, f1, g0);
    h1 = u128_add_mul(h1, f2_19, g4);
    h1 = u128_add_mul(h1, f3_19, g3);
    h1 = u128_add_mul(h1, f4_19, g2);

    h2 = u128_add_mul(h2, f0, g2);
    h2 = u128_add_mul(h2, f1, g1);
    h2 = u128_add_mul(h2, f2, g0);
    h2 = u128_add_mul(h2, f3_19, g4);
    h2 = u128_add_mul(h2, f4_19, g3);

    h3 = u128_add_mul(h3, f0, g3);
    h3 = u128_add_mul(h3, f1, g2);
    h3 = u128_add_mul(h3, f2, g1);
    h3 = u128_add_mul(h3, f3, g0);
    h3 = u128_add_mul(h3, f4_19, g4);

    h4 = u128_add_mul(h4, f0, g4);
    h4 = u128_add_mul(h4, f1, g3);
    h4 = u128_add_mul(h4, f2, g2);
    h4 = u128_add_mul(h4, f3, g1);
    h4 = u128_add_mul(h4, f4, g0);

    c = u128_shr51(h0); h0.lo &= FE51_MASK; h0.hi = 0; h1 = u128_add_u64(h1, c);
    c = u128_shr51(h1); h1.lo &= FE51_MASK; h1.hi = 0; h2 = u128_add_u64(h2, c);
    c = u128_shr51(h2); h2.lo &= FE51_MASK; h2.hi = 0; h3 = u128_add_u64(h3, c);
    c = u128_shr51(h3); h3.lo &= FE51_MASK; h3.hi = 0; h4 = u128_add_u64(h4, c);
    c = u128_shr51(h4); h4.lo &= FE51_MASK; h4.hi = 0; h0 = u128_add_u64(h0, c * 19);
    c = u128_shr51(h0); h0.lo &= FE51_MASK; h0.hi = 0; h1 = u128_add_u64(h1, c);

    h[0] = h0.lo;
    h[1] = h1.lo;
    h[2] = h2.lo;
    h[3] = h3.lo;
    h[4] = h4.lo;
}

static void fe51_sq(fe51 h, const fe51 f) {
    fe51_mul(h, f, f);
}

static void fe51_sq2(fe51 h, const fe51 f) {
    fe51_mul(h, f, f);
    fe51_add(h, h, h);
}

static void fe51_mul121666(fe51 h, const fe51 f) {
    fe_u128 h0 = u128_mul_u64(f[0], 121666);
    fe_u128 h1 = u128_mul_u64(f[1], 121666);
    fe_u128 h2 = u128_mul_u64(f[2], 121666);
    fe_u128 h3 = u128_mul_u64(f[3], 121666);
    fe_u128 h4 = u128_mul_u64(f[4], 121666);
    uint64_t c;

    c = u128_shr51(h0); h0.lo &= FE51_MASK; h0.hi = 0; h1 = u128_add_u64(h1, c);
    c = u128_shr51(h1); h1.lo &= FE51_MASK; h1.hi = 0; h2 = u128_add_u64(h2, c);
    c = u128_shr51(h2); h2.lo &= FE51_MASK; h2.hi = 0; h3 = u128_add_u64(h3, c);
    c = u128_shr51(h3); h3.lo &= FE51_MASK; h3.hi = 0; h4 = u128_add_u64(h4, c);
    c = u128_shr51(h4); h4.lo &= FE51_MASK; h4.hi = 0; h0 = u128_add_u64(h0, c * 19);
    c = u128_shr51(h0); h0.lo &= FE51_MASK; h0.hi = 0; h1 = u128_add_u64(h1, c);

    h[0] = h0.lo;
    h[1] = h1.lo;
    h[2] = h2.lo;
    h[3] = h3.lo;
    h[4] = h4.lo;
}

static void fe51_frombytes(fe51 h, const unsigned char *s) {
    h[0] = load_8(s) & FE51_MASK;
    h[1] = (load_8(s + 6) >> 3) & FE51_MASK;
    h[2] = (load_8(s + 12) >> 6) & FE51_MASK;
    h[3] = (load_8(s + 19) >> 1) & FE51_MASK;
    h[4] = (load_8(s + 24) >> 12) & FE51_MASK;
    fe51_reduce(h);
}

static void fe51_tobytes(unsigned char *s, const fe51 h) {
    uint64_t h0 = h[0], h1 = h[1], h2 = h[2], h3 = h[3], h4 = h[4];
    uint64_t q, c;

    q = (19 * h4 + (UINT64_C(1) << 24)) >> 51;
    q = (h0 + q) >> 51;
    q = (h1 + q) >> 51;
    q = (h2 + q) >> 51;
    q = (h3 + q) >> 51;
    q = (h4 + q) >> 51;

    h0 += 19 * q;
    c = h0 >> 51; h0 &= FE51_MASK; h1 += c;
    c = h1 >> 51; h1 &= FE51_MASK; h2 += c;
    c = h2 >> 51; h2 &= FE51_MASK; h3 += c;
    c = h3 >> 51; h3 &= FE51_MASK; h4 += c;
    c = h4 >> 51; h4 &= FE51_MASK; h0 += c * 19;
    c = h0 >> 51; h0 &= FE51_MASK; h1 += c;

    store_8(s, h0 | (h1 << 51));
    store_8(s + 8, (h1 >> 13) | (h2 << 38));
    store_8(s + 16, (h2 >> 26) | (h3 << 25));
    store_8(s + 24, (h3 >> 39) | (h4 << 12));
}

static void fe_to_bytes32(unsigned char *s, const fe f) {
    int i;
    for (i = 0; i < 8; ++i) {
        store_4(s + 4 * i, f[i]);
    }
}

static void fe_from_bytes32(fe h, const unsigned char *s) {
    int i;
    for (i = 0; i < 8; ++i) {
        h[i] = (uint32_t) load_4(s + 4 * i);
    }
}

static void fe_to51(fe51 out, const fe in) {
    unsigned char s[32];
    fe_to_bytes32(s, in);
    fe51_frombytes(out, s);
}

static void fe_from51(fe out, const fe51 in) {
    unsigned char s[32];
    fe51_tobytes(s, in);
    fe_from_bytes32(out, s);
}

void fe_0(fe h) {
    int i;
    for (i = 0; i < 8; ++i) h[i] = 0;
}

void fe_1(fe h) {
    fe_0(h);
    h[0] = 1;
}

void fe_copy(fe h, const fe f) {
    int i;
    for (i = 0; i < 8; ++i) h[i] = f[i];
}

void fe_frombytes(fe h, const unsigned char *s) {
    fe51 t;
    fe51_frombytes(t, s);
    fe_from51(h, t);
}

void fe_tobytes(unsigned char *s, const fe h) {
    fe51 t;
    fe_to51(t, h);
    fe51_tobytes(s, t);
}

void fe_add(fe h, const fe f, const fe g) {
    fe51 a, b, c;
    fe_to51(a, f);
    fe_to51(b, g);
    fe51_add(c, a, b);
    fe_from51(h, c);
}

void fe_sub(fe h, const fe f, const fe g) {
    fe51 a, b, c;
    fe_to51(a, f);
    fe_to51(b, g);
    fe51_sub(c, a, b);
    fe_from51(h, c);
}

void fe_mul(fe h, const fe f, const fe g) {
    fe51 a, b, c;
    fe_to51(a, f);
    fe_to51(b, g);
    fe51_mul(c, a, b);
    fe_from51(h, c);
}

void fe_sq(fe h, const fe f) {
    fe_mul(h, f, f);
}

void fe_sq2(fe h, const fe f) {
    fe51 a, c;
    fe_to51(a, f);
    fe51_sq2(c, a);
    fe_from51(h, c);
}

void fe_mul121666(fe h, fe f) {
    fe51 a, c;
    fe_to51(a, f);
    fe51_mul121666(c, a);
    fe_from51(h, c);
}

void fe_neg(fe h, const fe f) {
    fe z;
    fe_0(z);
    fe_sub(h, z, f);
}

void fe_cmov(fe f, const fe g, unsigned int b) {
    uint32_t mask = (uint32_t) 0 - (uint32_t) b;
    int i;
    for (i = 0; i < 8; ++i) {
        f[i] ^= mask & (f[i] ^ g[i]);
    }
}

void fe_cswap(fe f, fe g, unsigned int b) {
    uint32_t mask = (uint32_t) 0 - (uint32_t) b;
    int i;
    for (i = 0; i < 8; ++i) {
        uint32_t x = mask & (f[i] ^ g[i]);
        f[i] ^= x;
        g[i] ^= x;
    }
}

int fe_isnegative(const fe f) {
    unsigned char s[32];
    fe_tobytes(s, f);
    return s[0] & 1;
}

int fe_isnonzero(const fe f) {
    unsigned char s[32];
    unsigned char c = 0;
    int i;
    fe_tobytes(s, f);
    for (i = 0; i < 32; ++i) c |= s[i];
    return (int) ((c - 1) >> 8) + 1;
}

void fe_pow22523(fe out, const fe z) {
    fe t0, t1, t2;
    int i;

    fe_sq(t0, z);
    fe_sq(t1, t0);
    fe_sq(t1, t1);
    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t0, t0);
    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);
    for (i = 1; i < 5; ++i) fe_sq(t1, t1);
    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);
    for (i = 1; i < 10; ++i) fe_sq(t1, t1);
    fe_mul(t1, t1, t0);
    fe_sq(t2, t1);
    for (i = 1; i < 20; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);
    for (i = 1; i < 10; ++i) fe_sq(t1, t1);
    fe_mul(t0, t1, t0);
    fe_sq(t1, t0);
    for (i = 1; i < 50; ++i) fe_sq(t1, t1);
    fe_mul(t1, t1, t0);
    fe_sq(t2, t1);
    for (i = 1; i < 100; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);
    for (i = 1; i < 50; ++i) fe_sq(t1, t1);
    fe_mul(t0, t1, t0);
    fe_sq(t0, t0);
    fe_sq(t0, t0);
    fe_mul(out, t0, z);
}

void fe_invert(fe out, const fe z) {
    fe t0, t1, t2, t3;
    int i;

    fe_sq(t0, z);
    fe_sq(t1, t0);
    fe_sq(t1, t1);
    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t2, t0);
    fe_mul(t1, t1, t2);
    fe_sq(t2, t1);
    for (i = 1; i < 5; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);
    for (i = 1; i < 10; ++i) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);
    for (i = 1; i < 20; ++i) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);
    for (i = 1; i < 10; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1);
    for (i = 1; i < 50; ++i) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2);
    for (i = 1; i < 100; ++i) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2);
    for (i = 1; i < 50; ++i) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t1, t1);
    for (i = 1; i < 5; ++i) fe_sq(t1, t1);
    fe_mul(out, t1, t0);
}
