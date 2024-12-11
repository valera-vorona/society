#include"rand.h"
#include "types.h"
#include "stb_ds.h"
#include <time.h>

/* Mersene twister pseudorandom number generator */

#define m 397
#define w 32
#define r 31
#define UMASK (0xffffffffUL << r)
#define LMASK (0xffffffffUL >> (w-r))
#define a 0x9908b0dfUL
#define u 11
#define s 7
#define t 15
#define l 18
#define b 0x9d2c5680UL
#define c 0xefc60000UL
#define f 1812433253UL
#define n _n_

uint32_t random_device() {
   return (clock() + 19650218UL)*8253729  + 2396403;
}

void mt_init_state(mt_state* state, uint32_t seed) {
    uint32_t* state_array = &(state->state_array[0]);
    
    state_array[0] = seed;                          // suggested initial seed = 19650218UL
    
    for (int i=1; i<n; i++) {
        seed = f * (seed ^ (seed >> (w-2))) + i;    // Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
        state_array[i] = seed; 
    }
    
    state->state_index = 0;
}


uint32_t mt_random_uint32(mt_state* state) {
    uint32_t* state_array = &(state->state_array[0]);
    
    int k = state->state_index;      // point to current state location
                                     // 0 <= state_index <= n-1   always
    
//  int k = k - n;                   // point to state n iterations before
//  if (k < 0) k += n;               // modulo n circular indexing
                                     // the previous 2 lines actually do nothing
                                     //  for illustration only
    
    int j = k - (n-1);               // point to state n-1 iterations before
    if (j < 0) j += n;               // modulo n circular indexing

    uint32_t x = (state_array[k] & UMASK) | (state_array[j] & LMASK);
    
    uint32_t xA = x >> 1;
    if (x & 0x00000001UL) xA ^= a;
    
    j = k - (n-m);                   // point to state n-m iterations before
    if (j < 0) j += n;               // modulo n circular indexing
    
    x = state_array[j] ^ xA;         // compute next value in the state
    state_array[k++] = x;            // update new state value
    
    if (k >= n) k = 0;               // modulo n circular indexing
    state->state_index = k;
    
    uint32_t y = x ^ (x >> u);       // tempering 
             y = y ^ ((y << s) & b);
             y = y ^ ((y << t) & c);
    uint32_t z = y ^ (y >> l);

    return z; 
}

uint32_t uniform_uint_distribution(uint32_t num, uint32_t b_, uint32_t e_) {
    return num % (e_ - b_ + 1) + b_;
}

/*
 *
 * Perlin noise
 *
 */

static float quntic_curve(float t_);
float lerp(float a_, float b_, float t_);
static float dot(struct vec2f a_, struct vec2f b_);

void perlin2d_init(struct perlin2d *p, mt_state *state) {
    for (int i = 0; i < PERLIN2D_PERMUTATION_TABLE_SIZE; ++i)
        p->permutation_table[i] = uniform_uint_distribution(mt_random_uint32(state), 0, 255) >> 3 & 3;
        //p->permutation_table[i] = uniform_uint_distribution(mt_random_uint32(state), 0, 255) & 3;
}

static struct vec2f get_pseudo_random_gradient_vector(struct perlin2d *p, int x, int y) {
    struct vec2f rv;
    /* int v = (int)(((x * 1836311903) ^ (y * 2971215073) + 4807526976) & (PERLIN2D_PERMUTATION_TABLE_SIZE - 1)); */
    int v = (int)(((x * 1836311903) ^ ((y * 2971215073) + 4807526976)) & (PERLIN2D_PERMUTATION_TABLE_SIZE - 1));
    v = p->permutation_table[v];

    switch (v) {
    case 0:  rv.x = 1; rv.y = 0; return rv;
    case 1:  rv.x = -1; rv.y = 0; return rv;;
    case 2:  rv.x = 0; rv.y = 1; return rv;
    default: rv.x = 0; rv.y = -1; return rv;
    }
}

/* returns -0.5 <= retval <= 0.5 */
float perlin2d_noise(struct perlin2d *p, float fx, float fy) {
    int left = fx;
    int top  = fy;
    float pointInQuadX = fx - left;
    float pointInQuadY = fy - top;

    struct vec2f topLeftGradient        = get_pseudo_random_gradient_vector(p, left,   top  );
    struct vec2f topRightGradient       = get_pseudo_random_gradient_vector(p, left+1, top  );
    struct vec2f bottomLeftGradient     = get_pseudo_random_gradient_vector(p, left,   top+1);
    struct vec2f bottomRightGradient    = get_pseudo_random_gradient_vector(p, left+1, top+1);

    struct vec2f distanceToTopLeft      = { pointInQuadX,   pointInQuadY   };
    struct vec2f distanceToTopRight     = { pointInQuadX-1, pointInQuadY   };
    struct vec2f distanceToBottomLeft   = { pointInQuadX,   pointInQuadY-1 };
    struct vec2f distanceToBottomRight  = { pointInQuadX-1, pointInQuadY-1 };

    float tx1 = dot(distanceToTopLeft,     topLeftGradient);
    float tx2 = dot(distanceToTopRight,    topRightGradient);
    float bx1 = dot(distanceToBottomLeft,  bottomLeftGradient);
    float bx2 = dot(distanceToBottomRight, bottomRightGradient);

    pointInQuadX = quntic_curve(pointInQuadX);
    pointInQuadY = quntic_curve(pointInQuadY);

    float tx = lerp(tx1, tx2, pointInQuadX);
    float bx = lerp(bx1, bx2, pointInQuadX);
    float tb = lerp(tx, bx, pointInQuadY);

    return tb;
}

/* returns -0.5 <= retval <= 0.5 */
float perlin2d_noise_x(struct perlin2d *p, float fx, float fy, int octaves, float persistence) {
    float amplitude = 1;
    float max = 0;
    float result = 0;

    while (octaves--) {
        max += amplitude;
        result += perlin2d_noise(p, fx, fy) * amplitude;
        amplitude *= persistence;
        fx *= 2;
        fy *= 2;
    }

    return result/max;
}

/*
 * Helper functions
 */

static float quntic_curve(float t_) {
    return t_ * t_ * t_ * (t_ * (t_ * 6 - 15) + 10);
}

float lerp(float a_, float b_, float t_) {
    return a_ + (b_ - a_) * t_;
}

int individual_distribute(float *items, float v) {
    int i, ie;
    float sum = .0;
    for (i = 0, ie = arrlenu(items); i != ie; ++i) {
        sum += items[i];
        if (v < sum) break;
    }
    return i;
}

static float dot(struct vec2f a_, struct vec2f b_) {
    return a_.x * b_.x + a_.y * b_.y;
}

int trim(int min, int max, int t_) {
    if (t_ <= min) return min;
    if (t_ >= max) return max;
    return t_; 
}

