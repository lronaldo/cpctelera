/*
   sha3-256.c
   sha3-256 hash function (once crashed compiler when targeting hc08/s08/mos6502/mos65c02).

   Based on implementation by Eugene Chou (which in turn was based on an implementation by Dr. Markku-Juhani O. Saarinen).
*/

#include <testfwk.h>

#include <stdint.h>
#include <string.h>

#if !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) // mos6052/mos65c02 can't return struct this large yet.
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_ds390) // hc08/s08/ds390 can't return struct yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !( (defined (__SDCC_mos6502) || defined(__SDCC_mos65c02 )) && defined(__SDCC_STACK_AUTO) ) // Lack of memory

#define SHA3_256_MD_LEN 32      // 256-bit digest length in bytes.
#define SHA3_256_ROUNDS 24      // KECCAK rounds to perform for SHA3-256.
#define SHA3_256_WIDTH  200     // 1600-bit width in bytes.
#define SHA3_256_LANES  25      // State is an unrolled 5x5 array of 64-bit lanes.
#define SHA3_256_RATE   136     // 1600-bit width - 512-bit capacity in bytes.

struct Sha3_256 {
    int padpoint;
    int absorbed;
    union {
        uint64_t words[SHA3_256_LANES];
        uint8_t bytes[SHA3_256_WIDTH];
    } state;
};

struct Sha3_256 sha3_256_new(void);

void sha3_256_update(struct Sha3_256 *ctx, uint8_t *data, uint64_t n);

void sha3_256_finalize(struct Sha3_256 *ctx, uint8_t *digest);

void sha3_256_digest(uint8_t *data, uint64_t n, uint8_t *digest);

#define ROTL64(x, y)  (((x) << (y)) | ((x) >> (64 - (y))))

static void theta(struct Sha3_256 *ctx) {
    uint64_t C[5] = { 0 };
    uint64_t D[5] = { 0 };

    for (int i = 0; i < 5; i += 1) {
        C[i]  = ctx->state.words[i];
        C[i] ^= ctx->state.words[i + 5];
        C[i] ^= ctx->state.words[i + 10];
        C[i] ^= ctx->state.words[i + 15];
        C[i] ^= ctx->state.words[i + 20];
    }

    for (int i = 0; i < 5; i += 1) {
        D[i] = C[(i + 4) % 5] ^ ROTL64(C[(i + 1) % 5], 1);
    }

    for (int i = 0; i < 5; i += 1) {
        for (int j = 0; j < 25; j += 5) {
            ctx->state.words[i + j] ^= D[i];
        }
    }
}

static void rho(struct Sha3_256 *ctx) {
    static const int rotations[25] = {
         0,  1, 62, 28, 27,
        36, 44,  6, 55, 20,
         3, 10, 43, 25, 39,
        41, 45, 15, 21,  8,
        18,  2, 61, 56, 14
    };

    for (int i = 0; i < 25; i += 1) {
        ctx->state.words[i] = ROTL64(ctx->state.words[i], rotations[i]);
    }
}

static void pi(struct Sha3_256 *ctx) {
    static const int switcheroo[25] = {
         0, 10, 20,  5, 15,
        16,  1, 11, 21,  6,
         7, 17,  2, 12, 22,
        23,  8, 18,  3, 13,
        14, 24,  9, 19,  4
    };

    uint64_t temp[25] = { 0 };

    for (int i = 0; i < 25; i += 1) {
        temp[i] = ctx->state.words[i];
    }

    for (int i = 0; i < 25; i += 1) {
        ctx->state.words[switcheroo[i]] = temp[i];
    }
}

static void chi(struct Sha3_256 *ctx) {
    uint64_t temp[5] = { 0 };

    for (int j = 0; j < 25; j += 5) {
        for (int i = 0; i < 5; i += 1) {
            temp[i] = ctx->state.words[i + j];
        }

        for (int i = 0; i < 5; i += 1) {
            ctx->state.words[i + j] ^= (~temp[(i + 1) % 5]) & temp[(i + 2) % 5];
        }
    }
}

static void iota(struct Sha3_256 *ctx, uint8_t r) {
    static const uint64_t constants[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };

    ctx->state.words[0] ^= constants[r];
}

static void keccak(struct Sha3_256 *ctx) {
    for (int i = 0; i < SHA3_256_ROUNDS; i += 1) {
        theta(ctx);
        rho(ctx);
        pi(ctx);
        chi(ctx);
        iota(ctx, i);
    }
}

static void absorb(struct Sha3_256 *ctx, uint8_t *data, uint64_t n) {
    for (uint64_t i = 0; i < n; i += 1) {
        ctx->state.bytes[ctx->absorbed++] ^= data[i];

        if (ctx->absorbed == SHA3_256_RATE) {
            keccak(ctx);
            ctx->absorbed = 0;
        }
    }

    ctx->padpoint = ctx->absorbed;
}

static void squeeze(struct Sha3_256 *ctx, uint8_t *digest) {
    ctx->state.bytes[ctx->padpoint] ^= 0x06;
    ctx->state.bytes[SHA3_256_RATE - 1] ^= 0x80;

    keccak(ctx);

    for (int i = 0; i < SHA3_256_MD_LEN; i += 1) {
        digest[i] = ctx->state.bytes[i];
    }

    ctx->padpoint = ctx->absorbed = 0;
    memset(&ctx->state.words, 0, sizeof(ctx->state.words));
}

struct Sha3_256 sha3_256_new(void) {
    struct Sha3_256 ctx;
    memset(&ctx, 0, sizeof(ctx));
    return ctx;
}

void sha3_256_update(struct Sha3_256 *ctx, uint8_t *data, uint64_t n) {
    absorb(ctx, data, n);
}

void sha3_256_finalize(struct Sha3_256 *ctx, uint8_t *digest) {
    squeeze(ctx, digest);
}

void sha3_256_digest(uint8_t *data, uint64_t n, uint8_t *digest) {
    struct Sha3_256 ctx;
    ctx = sha3_256_new();
    absorb(&ctx, data, n);
    squeeze(&ctx, digest);
}

/* structs */
struct pair {
    unsigned char* in; /* input string */
    unsigned char out[32]; /* expected output */
};

/* known input/output pairs */
const struct pair pairs[] = {
    /* 0: empty string */
    {
        "",
        {0xa7, 0xff, 0xc6, 0xf8, 0xbf, 0x1e, 0xd7, 0x66, 0x51, 0xc1, 0x47, 0x56, 0xa0, 0x61, 0xd6, 0x62, 0xf5, 0x80, 0xff, 0x4d, 0xe4, 0x3b, 0x49, 0xfa, 0x82, 0xd8, 0x0a, 0x4b, 0x80, 0xf8, 0x43, 0x4a}
    },
    /* 1: long string */
    {
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
        {0x91, 0x6f, 0x60, 0x61, 0xfe, 0x87, 0x97, 0x41, 0xca, 0x64, 0x69, 0xb4, 0x39, 0x71, 0xdf, 0xdb, 0x28, 0xb1, 0xa3, 0x2d, 0xc3, 0x6c, 0xb3, 0x25, 0x4e, 0x81, 0x2b, 0xe2, 0x7a, 0xad, 0x1d, 0x18}
    }
};

#endif
#endif
#endif

void
testSha (void)
{
#if __STDC_ENDIAN_NATIVE__ // The implementation assumes little-endian
#if !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) // mos6502/mos65c02 can't return struct this large yet
#if !defined(__SDCC_ds390) // ds390 can't return struct yet.
#if !__SDCC_mcs51 && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
    int i;

    for (i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i++) {
      unsigned char out[32];
      sha3_256_digest(pairs[i].in, strlen(pairs[i].in), out);
      ASSERT(!memcmp(out, pairs[i].out, sizeof(out)));
    }
#endif
#endif
#endif
#endif
}

