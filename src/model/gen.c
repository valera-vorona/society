#include "gen.h"
#include "rand.h"
#include "stb_ds.h"
#define JQ_WITH_DOM
#include "jquick.h"
#include <malloc.h>

int individual_distribute(float *items, float v) {
    int i, ie;
    float sum = .0;
    for (i = 0, ie = arrlenu(items); i != ie; ++i) {
        sum += items[i];
        if (v < sum) break;
    }
    return i;
}

void gen_map(struct map *map, float *gen_parts, struct mt_state *mt, struct vec2 size) {
    map->size = size;
    map->tiles = malloc(sizeof(struct tile) * size.x * size.y);

    struct perlin2d perlin;
    perlin2d_init(&perlin, mt);
    for (int i = 0, y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x, ++i) {
            float r = .5 + perlin2d_noise_x(&perlin, (float)x/64., (float)y/64., 5, .7);
            map->tiles[i].type = individual_distribute(gen_parts, r * 100.);
        }
    }
}

void gen_units(struct world *w) {
     for (int y = 0, ye = w->map.size.y; y != ye; ++y) {
        for (int i = y*w->map.size.x, x = 0, xe = w->map.size.x; x != xe; ++i, ++x) {
            struct jq_value *units;
            struct jq_value *u;
            units = jq_find(w->json, "units", 0);

            /* looping through units */
            jq_foreach_array(u, units) {
                struct jq_value *tiles;
                struct jq_value *t;
                tiles = jq_find(u, "tiles", 0);

                /* looping through the unit's tile array */
                jq_foreach_array(t, tiles) {
                    if (t->value.integer == w->map.tiles[i].type) {
                        struct jq_value *prob;
                        prob = jq_find(u, "prob", 0);
                        if ((float)mt_random_uint32(w->mt) / (float)0xffffffff < prob->value.real) {
                            struct unit u = { { x*64, y*64 }, { 0, 0 }, { 0, 0 } };
                            arrput(w->units, u);
                        }   
                    }
                }
            }
        }
    }
}

void gen_world(struct world *w, struct vec2 size, uint32_t seed) {
    gen_map(&w->map, w->gen_parts, w->mt, size);
    gen_units(w);
    /*struct map map;
    struct resource *recources;
    struct unit *units;
    struct building *buildings;
    struct asset *assets;
    struct tool *tools;
    struct receipt *receipts;*/
}

