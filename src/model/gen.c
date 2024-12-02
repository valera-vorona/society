#include "gen.h"
#include "rand.h"
#include "ai.h"
#include "tileset.h"
#include "stb_ds.h"
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
            struct tile tile = {
                .type = individual_distribute(gen_parts, r * 100.),
                .tileset_index = 0,
                .transit_index = 0,
                .units = { ID_NOTHING }
            };

            map->tiles[i] = tile;
        }
    }

    arrfree(gen_parts);
}

struct tile *
map_get_tile(struct map *map, int x, int y) {
    return map->tiles + map->size.x * y + x;
}

static void
transit_map(struct world *w) {
    struct map *map = &w->map;
    /* TODO: I should check if tileset 'landset' exists */
    struct tileset *t = &shgetp_null(w->tilesets, "landset")->value;
    struct vec2 size = map->size;

    for (int i = 0, y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x, ++i) {
            int quad = 0;
            struct tile *tile = map_get_tile(map, x, y);
            if (x > 0 && tile->type < map_get_tile(map, x - 1, y)->type) {
                quad |= neighbor_left;
            }
            if (y > 0 && tile->type < map_get_tile(map, x, y - 1)->type) {
                quad |= neighbor_up;
            }
            if (x < size.x - 1 && tile->type < map_get_tile(map, x + 1, y)->type) {
                quad |= neighbor_right;
            }
            if (y < size.y - 1 && tile->type < map_get_tile(map, x, y + 1)->type) {
                quad |= neighbor_down;
            }

            map->tiles[i].tileset_index = tileset_quad_get_tile_index(t, tile->type, 0);
            map->tiles[i].transit_index = tileset_quad_get_tile_index(t, tile->type, quad);
        }
    }
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
}

static void
gen_unit_ais(struct world *w) {
    w->ais = NULL;
    arrsetlen(w->ais, arrlenu(w->units));
    for (int i = 0, ie = arrlenu(w->ais); i != ie; ++i) {
        w->ais[i].world = w;
        if (w->units[i].flags == UF_PLAYER) {
            ai_player_init(&w->ais[i]);
            w->player_ai = &w->ais[i];
        } else {
            ai_human_init(&w->ais[i]);
        }

        w->ais[i].unit = &w->units[i];
    }
}

void gen_world(struct world *w, struct vec2 size, uint32_t seed) {
    gen_map(&w->map, w->mt, size);
    transit_map(w);
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

