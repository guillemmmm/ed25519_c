#include "fixedint.h"
#include "fe.h"
#include "fe10_internal.h"

static uint32_t load_4(const unsigned char *in) {
    uint32_t result = (uint32_t) in[0];
    result |= ((uint32_t) in[1]) << 8;
    result |= ((uint32_t) in[2]) << 16;
    result |= ((uint32_t) in[3]) << 24;
    return result;
}

static void store_4(unsigned char *out, uint32_t in) {
    out[0] = (unsigned char) in;
    out[1] = (unsigned char) (in >> 8);
    out[2] = (unsigned char) (in >> 16);
    out[3] = (unsigned char) (in >> 24);
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
        h[i] = load_4(s + 4 * i);
    }
}

static void fe_to_fe10(fe10 out, const fe in) {
    unsigned char s[32];
    fe_to_bytes32(s, in);
    fe10_frombytes(out, s);
}

static void fe_from_fe10(fe out, const fe10 in) {
    unsigned char s[32];
    fe10_tobytes(s, in);
    fe_from_bytes32(out, s);
}

void fe_0(fe h) {
    int i;
    for (i = 0; i < 8; ++i) {
        h[i] = 0;
    }
}

void fe_1(fe h) {
    fe_0(h);
    h[0] = 1;
}

void fe_copy(fe h, const fe f) {
    int i;
    for (i = 0; i < 8; ++i) {
        h[i] = f[i];
    }
}

void fe_frombytes(fe h, const unsigned char *s) {
    fe10 t;
    fe10_frombytes(t, s);
    fe_from_fe10(h, t);
}

void fe_tobytes(unsigned char *s, const fe h) {
    fe10 t;
    fe_to_fe10(t, h);
    fe10_tobytes(s, t);
}

void fe_add(fe h, const fe f, const fe g) {
    fe10 a;
    fe10 b;
    fe10 c;
    fe_to_fe10(a, f);
    fe_to_fe10(b, g);
    fe10_add(c, a, b);
    fe_from_fe10(h, c);
}

void fe_sub(fe h, const fe f, const fe g) {
    fe10 a;
    fe10 b;
    fe10 c;
    fe_to_fe10(a, f);
    fe_to_fe10(b, g);
    fe10_sub(c, a, b);
    fe_from_fe10(h, c);
}

void fe_mul(fe h, const fe f, const fe g) {
    fe10 a;
    fe10 b;
    fe10 c;
    fe_to_fe10(a, f);
    fe_to_fe10(b, g);
    fe10_mul(c, a, b);
    fe_from_fe10(h, c);
}

void fe_sq(fe h, const fe f) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, f);
    fe10_sq(c, a);
    fe_from_fe10(h, c);
}

void fe_sq2(fe h, const fe f) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, f);
    fe10_sq2(c, a);
    fe_from_fe10(h, c);
}

void fe_mul121666(fe h, fe f) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, f);
    fe10_mul121666(c, a);
    fe_from_fe10(h, c);
}

void fe_neg(fe h, const fe f) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, f);
    fe10_neg(c, a);
    fe_from_fe10(h, c);
}

void fe_invert(fe out, const fe z) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, z);
    fe10_invert(c, a);
    fe_from_fe10(out, c);
}

void fe_pow22523(fe out, const fe z) {
    fe10 a;
    fe10 c;
    fe_to_fe10(a, z);
    fe10_pow22523(c, a);
    fe_from_fe10(out, c);
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
    for (i = 0; i < 32; ++i) {
        c |= s[i];
    }
    return (int) ((c - 1) >> 8) + 1;
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
