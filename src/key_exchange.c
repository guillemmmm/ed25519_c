#include "ed25519.h"
#include "fe.h"

static fe load_v121666(void){
    fe d = chess_dont_care(vdata);
    d = vupd_mem(d, 0, 0x0001DB42);
    for (int i=1;i<8;i++) {
        d = vupd_reg(d, i, 0);
    }
    return d;
}

void ed25519_key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key) {
    unsigned char e[32];
    unsigned int i;
    
    fe x1 = chess_dont_care(vdata);
    fe x2 = chess_dont_care(vdata);
    fe z2 = chess_dont_care(vdata);
    fe x3;
    fe z3 = chess_dont_care(vdata);
    fe tmp0;
    fe tmp1 = chess_dont_care(vdata);

    int pos;
    unsigned int swap;
    unsigned int b;

    /* copy the private key and make sure it's valid */
    for (i = 0; i < 32; ++i) {
        e[i] = private_key[i];
    }

    e[0] &= 248;
    e[31] &= 63;
    e[31] |= 64;

    /* unpack the public key and convert edwards to montgomery */
    /* due to CodesInChaos: montgomeryX = (edwardsY + 1)*inverse(1 - edwardsY) mod p */
    fe_frombytes(&x1, public_key);
    fe_1(&tmp1);
    //fe_add(tmp0, x1, tmp1);
    tmp0 = x1 + tmp1;
    //fe_sub(tmp1, tmp1, x1);
    uint32_t trash;
    tmp1 = vsub(tmp1, x1, trash);
    const fe tmp1c = tmp1;
    fe_invert(&tmp1, tmp1c);
    //fe_mul(x1, tmp0, tmp1);
    x1 = tmp0 * tmp1;

    fe_1(&x2);
    fe_0(&z2);
    x3 = x1;
    fe_1(&z3);

    swap = 0;
    for (pos = 254; pos >= 0; --pos) {
        b = e[pos / 8] >> (pos & 7);
        b &= 1;
        swap ^= b;
        fe_cswap(&x2, &x3, swap);
        fe_cswap(&z2, &z3, swap);
        swap = b;

        /* from montgomery.h */
        // fe_sub(tmp0, x3, z3);
        uint32_t trash1;
        tmp0 = vsub(x3, z3, trash1);
        
        // fe_sub(tmp1, x2, z2);
        uint32_t trash2;
        tmp1 = vsub(x2, z2, trash2);
        
        // fe_add(x2, x2, z2);
        x2 = x2 + z2;
        
        // fe_add(z2, x3, z3);
        z2 = x3 + z3;
        
        // fe_mul(z3, tmp0, x2);
        z3 = tmp0 * x2;
        
        // fe_mul(z2, z2, tmp1);
        z2 = z2 * tmp1;
        
        // fe_sq(tmp0, tmp1);
        tmp0 = tmp1 * tmp1;
        
        // fe_sq(tmp1, x2);
        tmp1 = x2 * x2;
        
        // fe_add(x3, z3, z2);
        x3 = z3 + z2;
        
        // fe_sub(z2, z3, z2);
        uint32_t trash3;
        z2 = vsub(z3, z2, trash3);
        
        // fe_mul(x2, tmp1, tmp0);
        x2 = tmp1 * tmp0;
        
        // fe_sub(tmp1, tmp1, tmp0);
        uint32_t trash4;
        tmp1 = vsub(tmp1, tmp0, trash4);
        
        // fe_sq(z2, z2);
        z2 = z2 * z2;
        
        //fe_mul121666(z3, tmp1);
        fe v121666 = load_v121666();
        z3 = tmp1 * v121666;
        
        // fe_sq(x3, x3);
        x3 = x3 * x3;
        
        // fe_add(tmp0, tmp0, z3);
        tmp0 = tmp0 + z3;
        
        // fe_mul(z3, x1, z2);
        z3 = x1 * z2;
        
        // fe_mul(z2, tmp1, tmp0);
        z2 = tmp1 * tmp0;
    }

    fe_cswap(&x2, &x3, swap);
    fe_cswap(&z2, &z3, swap);

    const fe z2c = z2;
    fe_invert(&z2, z2c);
    //fe_mul(x2, x2, z2);
    x2 = x2 * z2;
    fe_tobytes(shared_secret, x2);
}
