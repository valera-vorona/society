#include "path.h"
#include "stb_ds.h"
#include <math.h>
#include <assert.h>

#include <stdio.h>

struct node {
    struct vec2 coo;
    float g;                    /* moving cost from src tile to this tile */
    int h;                      /* estimated cost from this tile to dest tile */
    int prev;                   /* previous path node */
};

float calc_h(struct vec2 src, struct vec2 dest);
static float get_passability(struct world *w, struct unit *u, struct tile *t);
#define is_obstacle(pass) (pass == .0)
static size_t *arrputsorted(struct node *data, size_t *a, struct node *v);

void
path_init(struct path *p) {
    p->steps = NULL;
}

void
path_free(struct path *p) {
    if (p->steps) {
        arrfree(p->steps);
        p->steps = NULL;
    }
}

/* TODO:
 * 1. Open array is kept ordered, I can use this to find data in it
 *    instead of iterating;
 * 2. close array is not ordered, I should check if I can turn it to a hash
 *    to ease the finding prosess;
 */
struct path
find_path(struct world *w, struct unit *u, struct vec2 dest) {
    struct neighbor {
        struct vec2 coo;
        float pass;
    } neighbors[8];

    struct path rv = { NULL };
    struct map *map = &w->map;
    struct tile *tiles = map->tiles;
    struct node *data = NULL;           /* path node storage */
    size_t *open = NULL;                /* open list */
    size_t *close = NULL;               /* close list */
    int path_found = 0;

    /* src tile is u->coords / 64, dest tile is dest.
     * we swap src and dest not to reverse the result path
     * as said in A* algorithm instruction
     */
    struct vec2 finish = { u->coords.x / 64, u->coords.y / 64 };
    struct node start = { coo: dest, g: .0, h: calc_h(dest, finish), prev: -1 };

    if (!is_obstacle(get_passability(w, u, &tiles[map->size.x * dest.y + dest.x]))) {
        arrsetcap(data, (int)start.h * 2);
        arrsetcap(open, (int)start.h * 2);
        arrsetcap(close, (int)start.h * 2);

        arrput(data, start);
        arrput(open, 0);
    }

    while (arrlenu(open)) {
        int num = 0;
        size_t current_index = open[arrlenu(open) - 1];
        struct node *current = &data[current_index];
        struct vec2 coo = current->coo;
        float pass;

        /* moving the node with the least f, then highest g from open to close */
        arrput(close, arrpop(open));

        if (finish.x == current->coo.x && finish.y == current->coo.y) {
            path_found = 1;
            break;
        }

        /* filling in the neighbors array with coords */
        if (coo.x > 0) {
            if (coo.y > 0) {
                neighbors[num].coo.x = coo.x - 1;
                neighbors[num].coo.y = coo.y - 1;
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num++].pass = pass * 1.4;
                }
            }

            neighbors[num].coo.x = coo.x - 1;
            neighbors[num].coo.y = coo.y;
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num++].pass = pass;
            }

            if (coo.y + 1 < map->size.y) {
                neighbors[num].coo.x = coo.x - 1;
                neighbors[num].coo.y = coo.y + 1;
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num++].pass = pass * 1.4;
                }
            }
        }

        if (coo.y > 0) {
            neighbors[num].coo.x = coo.x;
            neighbors[num].coo.y = coo.y - 1;
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num++].pass = pass;
            }
        }

        if (coo.y + 1 < map->size.y) {
            neighbors[num].coo.x = coo.x;
            neighbors[num].coo.y = coo.y + 1;
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num++].pass = pass;
            }
        }

        if (coo.x + 1 < map->size.x) {
            if (coo.y > 0) {
                neighbors[num].coo.x = coo.x + 1;
                neighbors[num].coo.y = coo.y - 1;
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num++].pass = pass * 1.4;
                }
            }

            neighbors[num].coo.x = coo.x + 1;
            neighbors[num].coo.y = coo.y;
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num++].pass = pass;
            }

            if (coo.y + 1 < map->size.y) {
                neighbors[num].coo.x = coo.x + 1;
                neighbors[num].coo.y = coo.y + 1;
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num++].pass = pass * 1.4;
                }
            }
        }
        assert(num <= 8);

        /* for each neighbor */
        for (int i = 0; i != num; ++i) {
            struct neighbor *n = &neighbors[i];

            /* looking for the neighbor in the close list, TODO: to be optimized */
            int found = 0;
            for (int j = 0, je = arrlenu(close); j != je; ++j) {
                if (n->coo.x == data[ close[j] ].coo.x && n->coo.y == data[ close[j] ].coo.y) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            /* looking for the neighbor in the open list, TODO: to be optimized */
            struct node *s_found = NULL;
            for (int j = 0, je = arrlenu(open); j != je; ++j) {
                if (n->coo.x == data[ open[j] ].coo.x && n->coo.y == data[ open[j] ].coo.y) {
                    s_found = &data[ open[j] ];
                    break;
                }
            }

            if (!s_found) {
                struct node node = { coo: n->coo, g: current->g + n->pass, h: calc_h(n->coo, finish), prev: current_index };
                arrput(data, node);
                open = arrputsorted(data, open, &data[ arrlenu(data) - 1 ]);
            } else if (current->g + n->pass + calc_h(n->coo, finish) < s_found->g + s_found->h) {
                s_found->g = current->g + n->pass;
                s_found->prev = current_index;
            }
        }
    }

    arrfree(open);
    /* Backtracking, filling in the path */
    if (path_found) {
        int i = close[ arrlenu(close) - 1 ];        /* index in the data array */
        i = data[i].prev;                           /* ignoring the first step */

        while (i != -1) {
            arrput(rv.steps, data[i].coo);
            i = data[i].prev;
        }
    }

    arrfree(close);
    arrfree(data);

    return rv;
}

float
calc_h(struct vec2 src, struct vec2 dest) {
    int min;
    int max;
    struct vec2 delta = { abs(src.x - dest.x), abs(src.y - dest.y) };
    if (delta.x < delta.y) {
        min = delta.x;
        max = delta.y;
    } else {
        min = delta.y;
        max = delta.x;
    }

    return min * 1.4 + max - min;
}

static float
get_passability(struct world *w, struct unit *u, struct tile *t) {
    return w->unit_types[u->type].pass[t->type];
}

/* it inserts struct node *node to array so that the later is sorted
 * from more to less f then from less to more g and that the least item
 * lays in the end of the array
 */
static size_t *
arrputsorted(struct node *data, size_t *a, struct node *v) {
    size_t b = 0;
    size_t e = arrlenu(a);
    while (b != e) {
        size_t mid = b + (e - b) / 2;
        assert(mid < e);
        float f[] = { v->g + v->h, data[ a[mid] ].g + data[ a[mid] ].h };
        if (f[0] < f[1] || (f[0] == f[1] && v->g > data[ a[mid] ].g)) {
            b = mid + 1;
        } else {
            e = mid;
        }
    }

    arrins(a, b, v - data);

    return a;
}

