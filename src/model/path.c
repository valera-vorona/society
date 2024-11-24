#include "path.h"
#include "stb_ds.h"
#include <math.h>

#define max(a, b) (a) > (b) ? (a) : (b)
#define calc_h(src, dest) max(abs(src.x - dest.x), abs(src.y - dest.y))

struct node {
    struct vec2 coo;
    float g;                    /* moving cost from src tile to this tile */
    int h;                      /* estimated cost from this tile to dest tile */
    struct node *prev;          /* previous path node */
};

static float get_passability(struct world *w, struct unit *u, struct tile *t);
#define is_obstacle(pass) (pass == .0)
static struct node *arrputsorted(struct node *a, struct node v);

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

/* TODO: function find_path is too inafficient and should be optimized!
 * 1. Structs current_v, current are copyed but shouldn't;
 * 2. Open array is kept ordered, I can use this to find data in it
 *    instead of iterating;
 * 3. close array is not ordered, I should check if I can turn it to a hash
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
    struct node *open = NULL;
    struct node *close = NULL;
    int path_found = 0;

    /* src tile is u->coords / 64, dest tile is dest.
     * we swap src and dest not to reverse the result path
     */
    struct vec2 finish = { u->coords.x / 64, u->coords.y / 64 };
    int h = calc_h(dest, finish);
    struct node start = { coo: dest, g: .0, h: h, prev: NULL };

    arrsetcap(open, h * 2);
    arrsetcap(close, h * 2);

    arrput(open, start);

    while (arrlenu(open)) {
        int num = 0;
        /* TODO: I should get rid of copying here */
        struct node current_v = open[ arrlenu(open) - 1];
        struct node *current = &current_v;
        struct vec2 coo = current->coo;
        float pass;

        /* moving the step with the least f, then highest g from open to close */
        arrput(close, arrpop(open));

        /* looking for the finish coo in the close list */
        for (int j = 0, je = arrlenu(close); j != je; ++j) {
            if (finish.x == close[j].coo.x && finish.y == close[j].coo.y) {
                /* path is found */
                path_found = 1;
                goto found;
            }
        }

        /* filling in the neighbors array with coords */
        if (coo.x > 0) {
            if (coo.y > 0) {
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num].coo.x = coo.x - 1;
                    neighbors[num].coo.y = coo.y - 1;
                    neighbors[num++].pass = pass * 1.4;
                }
            }

            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num].coo.x = coo.x - 1;
                neighbors[num].coo.y = coo.y;
                neighbors[num++].pass = pass;
            }

            if (coo.y + 1 < map->size.y) {
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num].coo.x = coo.x - 1;
                    neighbors[num].coo.y = coo.y + 1;
                    neighbors[num++].pass = pass * 1.4;
                }
            }
        }

        if (coo.y > 0) {
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num].coo.x = coo.x;
                neighbors[num].coo.y = coo.y - 1;
                neighbors[num++].pass = pass;
            }
        }

        if (coo.y + 1 < map->size.y) {
            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num].coo.x = coo.x;
                neighbors[num].coo.y = coo.y + 1;
                neighbors[num++].pass = pass;
            }
        }

        if (coo.x + 1 < map->size.x) {
            if (coo.y > 0) {
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num].coo.x = coo.x + 1;
                    neighbors[num].coo.y = coo.y - 1;
                    neighbors[num++].pass = pass * 1.4;
                }
            }

            pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
            if (!is_obstacle(pass)) {
                neighbors[num].coo.x = coo.x + 1;
                neighbors[num].coo.y = coo.y;
                neighbors[num++].pass = pass;
            }

            if (coo.y + 1 < map->size.y) {
                pass = get_passability(w, u, &tiles[map->size.x * neighbors[num].coo.y + neighbors[num].coo.x]);
                if (!is_obstacle(pass)) {
                    neighbors[num].coo.x = coo.x + 1;
                    neighbors[num].coo.y = coo.y + 1;
                    neighbors[num++].pass = pass * 1.4;
                }
            }
        }

        /* for each neighbor */
        for (int i = 0; i != num; ++i) {
            struct neighbor *n = &neighbors[i];

            /* looking for the neighbor in the close list */
            int found = 0;
            for (int j = 0, je = arrlenu(close); j != je; ++j) {
                if (n->coo.x == close[j].coo.x && n->coo.y == close[j].coo.y) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            /* looking for the neighbor in the open list */
            struct node *s_found = NULL;
            for (int j = 0, je = arrlenu(open); j != je; ++j) {
                if (n->coo.x == open[j].coo.x && n->coo.y == open[j].coo.y) {
                    s_found = &open[j];
                    break;
                }
            }

            if (!s_found) {
                struct node node = { coo: n->coo, g: current->g + n->pass, h: calc_h(n->coo, finish), prev: current };
                open = arrputsorted(open, node);
            } else if (current->g + n->pass + calc_h(n->coo, finish) < s_found->g + s_found->h) {
                s_found->g = current->g + n->pass;
            }
        }
    }
found:

    arrfree(open);
    /* Backtracking, filling in the path
     */
    if (path_found) {
        for (int i = arrlenu(close) - 1, ie = 0; i >= ie; --i) {
            arrput(rv.steps, close[i].coo);
        }

        arrput(rv.steps, dest);
    }

    arrfree(close);

    return rv;
}

static float
get_passability(struct world *w, struct unit *u, struct tile *t) {
    return w->unit_types[u->type].pass[t->type];
}

/* it sorts struct step *s array from more to less f then from less to more g
 * so that the least item lays in the end of array
 */
static struct node *
arrputsorted(struct node *a, struct node v) {
    size_t b = 0;
    size_t e = arrlenu(a);
    while (b != e) {
        size_t mid = b + (e - b) / 2;
        float f[] = { v.g + v.h, a[mid].g + a[mid].h };
        if (f[0] < f[1] || (f[0] == f[1] && v.g > a[mid].g)) {
            b = mid + 1;
        } else {
            e = mid;
        }
    }

    arrins(a, b, v);

    return a;
}

