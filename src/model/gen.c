#include "gen.h"
#include "rand.h"
#include "ai.h"
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

static void
gen_map(struct map *map, struct mt_state *mt, struct vec2 size) {
    map->size = size;
    map->tiles = malloc(sizeof(struct tile) * size.x * size.y);

    float *gen_parts = NULL;
    arrsetlen(gen_parts, arrlenu(map->tile_types));
    for (int i = 0, ie = arrlenu(map->tile_types); i != ie; ++i) {
        gen_parts[i] = map->tile_types[i].gen_part;
    }

    struct perlin2d perlin;
    perlin2d_init(&perlin, mt);
    for (int i = 0, y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x, ++i) {
            float r = .5 + perlin2d_noise_x(&perlin, (float)x/64., (float)y/64., 5, .7);
            struct tile tile = { individual_distribute(gen_parts, r * 100.), { ID_NOTHING } };
            map->tiles[i] = tile;
        }
    }

    arrfree(gen_parts);
}

static void 
gen_units(struct world *w) {
    float *probs = NULL;
    w->units = NULL;
    arrsetlen(probs, arrlenu(w->unit_types));

    for (int y = 0, ye = w->map.size.y; y != ye; ++y) {
        for (int i = y*w->map.size.x, x = 0, xe = w->map.size.x; x != xe; ++i, ++x) {
            int tile_type = w->map.tiles[i].type;
            float prob_sum = .0;
            for (int i = 0, ie = arrlenu(w->unit_types); i != ie; ++i) {
                float prob = w->unit_types[i].probs[tile_type];
                probs[i] = prob;
                prob_sum += prob;
            }

            float r = (float)mt_random_uint32(w->mt) / (float)0xffffffff;
            if (r < prob_sum) {
                struct unit u = { individual_distribute(probs, r), UF_NONE, { x*64, y*64 }, { 0, 0 }, { 0, 0 } };
                w->map.tiles[i].units[0] = arrlen(w->units);
                arrput(w->units, u);
            }
        }
    }

    arrfree(probs);
}

static void
gen_unit_flags(struct world *w) {
    int i = (int)lerp(0, arrlenu(w->units), (float)mt_random_uint32(w->mt) / (float)0xffffffff);
    w->units[i].flags |= UF_PLAYER;
    w->player = &w->units[i];
}

static void
gen_unit_ais(struct world *w) {
    w->ais = NULL;
    arrsetlen(w->ais, arrlenu(w->units));
    for (int i = 0, ie = arrlenu(w->ais); i != ie; ++i) {
        w->units[i].flags == UF_PLAYER ? ai_player_init(&w->ais[i]) : ai_human_init(&w->ais[i]);
        w->ais[i].unit = &w->units[i];
    }
}

void gen_world(struct world *w, struct vec2 size, uint32_t seed) {
    gen_map(&w->map, w->mt, size);
    gen_units(w);
    gen_unit_flags(w);
    gen_unit_ais(w);
    /*struct map map;
    struct resource *recources;
    struct unit *units;
    struct building *buildings;
    struct asset *assets;
    struct tool *tools;
    struct receipt *receipts;*/
}

