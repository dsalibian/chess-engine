/*
 *  self contained program to find optimal seeds for magic bitboard initialization
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef _WIN32 
    #include <windows.h>
    #include <bcrypt.h>
#else 
    #include <sys/random.h>
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef u64 bitboard;

#define SQR_BB(sqr)     (1ull << (sqr))
#define SQR_FILE(sqr)   ((sqr) % 8)
#define SQR_RANK(sqr)   ((sqr) / 8)

#define POPCNT(t)       ((u32)__builtin_popcountll(t))
#define TZCNT(t)        ((u32)__builtin_ctzll(t))

#define POP_LSB(t)      ((t) & ((t) - 1))

#define FILE_A  0x0101010101010101ull
#define FILE_H  0x8080808080808080ull
#define FILE_AH 0x8181818181818181ull

#define RANK_1  0x00000000000000ffull
#define RANK_8  0xff00000000000000ull
#define RANK_18 0xff000000000000ffull

#define BSP false
#define LEN (1u << (BSP ? 9 : 12))

enum direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NORTH_EAST,
    NORTH_WEST,
    SOUTH_EAST,
    SOUTH_WEST,
};

volatile sig_atomic_t fexit = 0;

void sigint(int sig) {
    fexit = 1;
}

u64 xorshift(u64* state) {
    *state ^= *state << 7;
    *state ^= *state >> 9;

    return *state;
}

u64 ran_u64() {
    u64 t;

#ifdef _WIN32
    BCryptGenRandom(NULL, (PUCHAR)&t, sizeof t, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#else
    getrandom(&t, sizeof(u64), 0); 
#endif

    return t;
}

u64 time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (u64)tv.tv_sec * 1000 + (u64)(tv.tv_usec / 1000);
}

bitboard gen_satts(const u32 sqr, const bitboard blk) {
    const i8 df[8] = { 0,  1,  0, -1,  1, -1,  1, -1};
    const i8 dr[8] = { 1,  0, -1,  0,  1,  1, -1, -1};

    bitboard atts = 0;

    for(i32 dir = BSP ? NORTH_EAST : NORTH; dir <= (BSP ? SOUTH_WEST : WEST); ++dir) {
        for(i32 f = SQR_FILE(sqr), r = SQR_RANK(sqr); (u32)(f += df[dir]) < 8 && (u32)(r += dr[dir]) < 8; ) {
            bitboard s = SQR_BB(8 * r + f);
            atts |= s;

            if(s & blk)
                break;
        }
    }
    
    return atts; 
}

bitboard gen_rmask(const u32 sqr) {
    bitboard m = gen_satts(sqr, 0);

    if(BSP)
        return m & ~(FILE_AH | RANK_18);

    const u32 f = SQR_FILE(sqr);
    const u32 r = SQR_RANK(sqr);

    if(f > 0) m &= ~FILE_A;
    if(f < 7) m &= ~FILE_H;

    if(r > 0) m &= ~RANK_1;
    if(r < 7) m &= ~RANK_8;

    return m;
}

bitboard nth_occupancy_mask(bitboard rmask, const u32 n) {
    const u32 popcnt = POPCNT(rmask);
    bitboard occupancy  = 0;

    for(u32 i = 0; i < popcnt; ++i) {
        u32 shamt = TZCNT(rmask);
        rmask = POP_LSB(rmask);
       
        if(n & (1u << i))
            occupancy |= 1ull << shamt;
    }

    return occupancy;
}

struct tbls {
    const bitboard *occupancies[64], *ref_atts[64];
    const u8 *popcnt;
};

typedef const struct tbls* const restrict tbls_ptr;

struct tbls gen_tbls() {
    struct tbls tbl;

    u8* popcnt = malloc(64);
    tbl.popcnt = popcnt;

    for(u32 s = 0; s < 64; ++s) {
        bitboard rmask = gen_rmask(s);
        popcnt[s] = (u8)POPCNT(rmask); 
         
        u32 u = 1u << popcnt[s]; 

        bitboard* occupancies = malloc(u * sizeof(bitboard));
        bitboard* atts        = malloc(u * sizeof(bitboard));

        tbl.occupancies[s] = occupancies;
        tbl.ref_atts[s]    = atts;

        for(u32 i = 0; i < u; ++i) {
            occupancies[i] = nth_occupancy_mask(rmask, i); 
            atts[i]        = gen_satts(s, occupancies[i]);
        }
    } 

    return tbl;
}

struct searcher_dat {
    bitboard atts[LEN];
    u32 its[LEN], it;
};

typedef struct searcher_dat* const restrict sdat_ptr;

struct searcher_dat searcher_dat_new() {
    struct searcher_dat dat;    

    memset(dat.its, 0, LEN * sizeof(u32));
    dat.it = 1;

    return dat;
}

bool valid_magic(tbls_ptr tbl, sdat_ptr sdat, const u32 sqr, const u64 magic, u32* const restrict j) {
    ++sdat->it;

    for(u32 i = 0; i < 1u << tbl->popcnt[sqr]; ++i) {
        u64 idx = (magic * tbl->occupancies[sqr][i]) >> (64u - tbl->popcnt[sqr]);

        if((sdat->its[idx] != sdat->it)) {
            sdat->its[idx]  = sdat->it;
            sdat->atts[idx] = tbl->ref_atts[sqr][i];
        } else if(sdat->atts[idx] != tbl->ref_atts[sqr][i])
            return false;

        if(j)
            ++*j;
    }

    return true;
}

u64 news(tbls_ptr tbl, sdat_ptr dat) {
    u64 magic, s_last, s = ran_u64();

    do s_last = s, magic = xorshift(&s) & xorshift(&s) & xorshift(&s);
    while(!valid_magic(tbl, dat, 0, magic, NULL));

    return s_last;
}

u32 counts(tbls_ptr tbl, sdat_ptr dat, u64 s, const u32 min) {
    u64 magic;
    u32 j = 0;

    for(u32 sqr = 0; j < min && sqr < 64; ++sqr)
        do magic = xorshift(&s) & xorshift(&s) & xorshift(&s);
        while(!valid_magic(tbl, dat, sqr, magic, &j));

    return j;
}

struct glbl_dat {
    tbls_ptr tbl;
    pthread_mutex_t mtx;

    _Atomic u64 total;
    u64 min_seed;
    u32 min_its;
};

u32 rd_min_its(struct glbl_dat* gdat) {
    pthread_mutex_lock(&gdat->mtx);
    u32 t = gdat->min_its;
    pthread_mutex_unlock(&gdat->mtx);

    return t;
}

void* searcher(void* p) {
    struct glbl_dat* const gdat = p;
    struct searcher_dat dat     = searcher_dat_new();

    for(; !fexit; atomic_fetch_add(&gdat->total, 1)) {
        u64 s = news(gdat->tbl, &dat);
        u32 i = counts(gdat->tbl, &dat, s, rd_min_its(gdat));

        pthread_mutex_lock(&gdat->mtx);
        if(i < gdat->min_its) {
            gdat->min_seed = s;
            gdat->min_its  = i;
        }

        pthread_mutex_unlock(&gdat->mtx);
    }

    return NULL;
}

double avg(const u64 total) {
    static u64 last[10], total_last, sum;

    sum -= last[9];
    memmove(last + 1, last, 9 * sizeof(u64));

    sum += (last[0] = total - total_last);

    total_last = total;

    return (double)sum / 10.;
}

void* printer(void* p) {
    struct glbl_dat* gdat = p;

    for(; !fexit; sleep(1)) {
        u64 total = atomic_load(&gdat->total);

        pthread_mutex_lock(&gdat->mtx);
        u64 s = gdat->min_seed;
        u32 i = gdat->min_its;
        pthread_mutex_unlock(&gdat->mtx);
        
        printf("\r\x1b[K0x%"PRIx64"ull = %u its (%"PRIu64" ttl, %.2f/s)", 
                s, i, total, avg(total));
        fflush(stdout);
    }

    return NULL;
}


void go(const u32 thread_count) {
    const struct tbls tbl = gen_tbls();
    struct glbl_dat gdat = (struct glbl_dat) 
        {.tbl = &tbl, .mtx = PTHREAD_MUTEX_INITIALIZER, .total = 0, .min_its = ~0u};

    u64 t = time_ms();
    pthread_t threads[thread_count + 1];

    printf("\nstarting search with n = %u threads...\n\n", thread_count);

    pthread_create(threads, NULL,  printer, &gdat);
    for(u32 i = 1; i < thread_count + 1; ++i) 
        pthread_create(threads + i, NULL, searcher, &gdat);

    for(u8 i = 0; i < thread_count + 1; ++i) 
        pthread_join(threads[i], NULL);

    printf("\n\nsearched %"PRIu64" seeds in %f days\n", 
            gdat.total, (double)(time_ms() - t) / (1000.0 * 60 * 60 * 24));
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint);

    printf("thread count: ");

    u32 n;
    scanf("%u", &n);
    go(n);

    return 0;
}
