#ifndef _RAND_H_
#define _RAND_H_

#include <stdint.h>

/* Mersene twister pseudorandom number generator */

#define _n_ 624

typedef struct mt_state {
    uint32_t state_array[_n_];      // the array for the state vector 
    int state_index;                // index into state vector array, 0 <= state_index <= n-1   always
} mt_state;

#define PERLIN2D_PERMUTATION_TABLE_SIZE 1024 
struct perlin2d {
    unsigned char permutation_table[PERLIN2D_PERMUTATION_TABLE_SIZE];
};

uint32_t random_device();           // generate pseudo random seed
void mt_init_state(mt_state* state, uint32_t seed); 
uint32_t mt_random_uint32(mt_state* state);
uint32_t uniform_uint_distribution(uint32_t num, uint32_t b_, uint32_t e_);

/* Perlin noise */
void perlin2d_init(struct perlin2d *p, mt_state *state);
float perlin2d_noise(struct perlin2d *p, float fx, float fy);
float perlin2d_noise_x(struct perlin2d *p, float fx, float fy, int octaves, float persistence);

/* linear interpolation of t_ between a_ and b_ where 0.0 <= t_ <= 1.0 */ 
float lerp(float a_, float b_, float t_);
int trim(int min, int max, int t_);

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

#endif /* _RAND_H_ */

