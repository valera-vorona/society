#include "gen.h"
#include "rand.h"
#include "ai.h"
#include "tileset.h"
#include "stb_ds.h"
#include <malloc.h>
#include <math.h>
#include <assert.h>

#define between(a, b, e) ((a) >= (b) && (a) < (e))

/*
 * Height
 */

static void
gen_height(struct map *map, struct mt_state *mt, struct vec2 size) {
    struct perlin2d perlin;

    int default_type = 0;
    for (int i = 0, ie = arrlenu(map->tile_types); i != ie; ++i) {
        if (map->tile_types[i].is_default) {
            default_type = i;
            break;
        }
    }

    perlin2d_init(&perlin, mt);
    map->size = size;
    map->tiles = malloc(sizeof(struct tile) * size.x * size.y);
    for (int i = 0, y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x, ++i) {
            float r = perlin2d_noise_x(&perlin, (float)x/64., (float)y/64., 5, .7);
            struct tile tile = {
                .type = default_type,
                .tileset_index = 0,
                .transit_index = 0,
                .height = r,
                .humidity = .0,
                .resource = ID_NOTHING,
                .units = { ID_NOTHING }
            };

            map->tiles[i] = tile;
        }
    }
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

/*
 * Humidity
 * returns evaporation in within .0 1. for water and 0. for land (if height >= .0)
 * the closer the tile to the equator is the heigher evaporation it has
 */

static float
tile_get_evaporation(struct map *m, struct tile *t, int x, int y) {
    if (t->height >= .0) {
        return .0;
    } else {
        if (y > 512)
            y = 1024 - y;
        assert(y >= 0);
        return (float)y / 512.;
    }
}

#define PI 3.14

/*
 * returns humidity within .0 .1 for land and 1. for water (if height <= .0)
 */

static void
gen_tile_humidity(struct map *m, int jet_latitude, int dest_latitude, float persistence, int x, int y) {
    struct tile *tile = map_get_tile(m, x, y);

    if (tile->height <= .0) {
        tile->humidity = 0.;
    } else {
        struct vec2 size = m->size;
        float angle = .0;
        float influence = 1.;
        float hum = .0;
        int inc;
        float angle_step = PI/2/(jet_latitude - dest_latitude);
        int height = jet_latitude - y;

        if (height == 0) {
            tile->humidity = .0;
            return;
        } else if (height > 0) {
            inc = 1;
            angle_step = -angle_step;
        } else {
            inc = -1;
        }

        for (int i = 0, ie = height; i != ie; i += inc, angle += angle_step, influence *= persistence) {
            float t = tan(angle);
            if (abs(t) > abs(height))
                break;

            struct vec2 src_coo = { trim(0, size.x - 1, x + i), trim(0, size.y - 1, y + t) };
            struct tile *src = map_get_tile(m, src_coo.x, src_coo.y);
            if (src->height <= .0)
                hum += influence * tile_get_evaporation(m, src, src_coo.x, src_coo.y);
        }

        tile->humidity = hum / (float)abs(height);
    }
}

static void
gen_humidity_from_wind(struct world *w, struct wind *wind) {
    struct map *map = &w->map;
    struct vec2 size = map->size;
    /* from grad to map coords */
    int jet_latitude = trim(0, size.y - 1, (90. - wind->jet_latitude) * size.y / 180);
    int dest_latitude = trim(0, size.y - 1, (90. - wind->dest_latitude) * size.y / 180);
    int inc = dest_latitude < jet_latitude ? 1 : -1;

    for (int y = dest_latitude, ye = jet_latitude; y != ye; y += inc) {
        for (int x = 0; x != size.x; ++x) {
            gen_tile_humidity(map, jet_latitude, dest_latitude, wind->persistence, x, y);
        }
    }
}

static void
gen_humidity(struct world *w) {
    for (int i = 0, ie = arrlenu(w->winds); i != ie; ++i) {
        gen_humidity_from_wind(w, &w->winds[i]);
    }
}

/*
 * Generators
 */

static void
standard_generator(struct world *w, struct generator *g) {
    struct map *m = &w->map;

    for (int i = 0, ie = m->size.x * m->size.y; i != ie; ++i) {
        struct tile *t = m->tiles + i;
        float r = (float)mt_random_uint32(w->mt) / (float)0xffffffff;
        if (r < g->in.prob &&
            between(t->height, g->in.height.min, g->in.height.max) &&
            between(t->humidity, g->in.humidity.min, g->in.humidity.max)) {
            int found = 0;

            switch (g->out.type) {
            case GT_TILE:
                for (int i = 0, ie = arrlenu(m->tile_types); i != ie; ++i) {
                    if (!strcmp(g->out.name, m->tile_types[i].name)) {
                        t->type = i;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    /* TODO: handle it somehow, actually types are checked during world load in world_init function,
                        so here it is not necessary to check them */
                }
                break;

            case GT_RESOURCE:
                for (int i = 0, ie = arrlenu(w->resource_types); i != ie; ++i) {
                    if (!strcmp(g->out.name, w->resource_types[i].name)) {
                        t->resource = i;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    /* TODO: handle it somehow, actually types are checked during world load in world_init function,
                        so here it is not necessary to check them */
                }
                break;

            default:
                break;
            }
        }
    }

}

static void
apply_generator(struct world *w, struct generator *g) {
    standard_generator(w, g);
}

static void
apply_generators(struct world *w) {
    for (int i = 0, ie = arrlenu(w->generators); i != ie; ++i) {
        apply_generator(w, &w->generators[i]);
    }
}

/*
 * Units
 */

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

/*
 * Main gen function
 */

void gen_world(struct world *w, struct vec2 size, uint32_t seed) {
    gen_height(&w->map, w->mt, size);
    gen_humidity(w);
    apply_generators(w);
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

