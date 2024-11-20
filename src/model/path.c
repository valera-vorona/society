#include "path.h"
#include "stb_ds.h"
#include <math.h>

#include <stdio.h>

#define max(a, b) (a) > (b) ? (a) : (b)
#define calc_h(src, dest) max(abs(src.x - dest.x), abs(src.y - dest.y))

struct step {
    struct vec2 coo;
    float g;                    /* moving cost from src tile to this tile */
    int h;                      /* estimated cost from this tile to dest tile */
    int neighbors_num;          /* max = 8 (left, up, right, down and diagonals) */
    struct vec2 neighbors[8];   /* neighbors */
};

static float get_passability(struct unit *u, struct tile *t);
static struct step *arrputsorted(struct step *a, struct step v);


/* TODO: function find_path is too inafficient and should be optimized!
 * 1. Structs current_v, current are copyed but shouldn't;
 * 2. Open array is kept ordered, I can use this to find data in it
 *    instead of iterating;
 * 3. close array is not ordered, I should check if I can turn it to a hash
 *    to ease the finding prosess;
 */
struct path
find_path(struct world *w, struct unit *u, struct vec2 dest) {
    struct path rv = { NULL };
    struct map *map = &w->map;
    struct tile *tiles = map->tiles;
    struct step *open = NULL;
    struct step *close = NULL;
    int path_found = 0;

    /* src tile is u->coords / 64, dest tile is dest.
     * we swap src and dest not to reverse the result path
     */
    struct vec2 finish = { u->coords.x / 64, u->coords.y / 64 };
    int h = calc_h(dest, finish);
    struct step start = { dest, .0, h, 0 };

    arrsetcap(open, h * 2);
    arrsetcap(close, h * 2);

    arrput(open, start);

    while (arrlenu(open)) {
        int num = 0;
        /* TODO: I should get rid of copying here */
        struct step current_v = open[ arrlenu(open) - 1];
        struct step *current = &current_v;
        struct vec2 coo = current->coo;

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
                current->neighbors[num].x = coo.x - 1;
                current->neighbors[num++].y = coo.y - 1;
            }

            current->neighbors[num].x = coo.x - 1;
            current->neighbors[num++].y = coo.y;

            if (coo.y + 1 < map->size.y) {
                current->neighbors[num].x = coo.x - 1;
                current->neighbors[num++].y = coo.y + 1;
            }
        }

        if (coo.y > 0) {
            current->neighbors[num].x = coo.x;
            current->neighbors[num++].y = coo.y - 1;
        }

        if (coo.y + 1 < map->size.y) {
            current->neighbors[num].x = coo.x;
            current->neighbors[num++].y = coo.y + 1;
        }

        if (coo.x + 1 < map->size.x) {
            if (coo.y > 0) {
                current->neighbors[num].x = coo.x + 1;
                current->neighbors[num++].y = coo.y - 1;
            }

            current->neighbors[num].x = coo.x + 1;
            current->neighbors[num++].y = coo.y;

            if (coo.y + 1 < map->size.y) {
                current->neighbors[num].x = coo.x + 1;
                current->neighbors[num++].y = coo.y + 1;
            }
        }

        current->neighbors_num = num;

        /* TODO: I should get rid of copying here */
        close[ arrlenu(close) - 1].neighbors_num = current->neighbors_num;
        memcpy(&close[ arrlenu(close) - 1].neighbors, &current->neighbors, sizeof(struct vec2) * 8);

        /* for each neighbor */
        for (int i = 0; i != num; ++i) {
            struct vec2 neighbor = current->neighbors[i];

            /* looking for the neighbor in the close list */
            int found = 0;
            for (int j = 0, je = arrlenu(close); j != je; ++j) {
                if (neighbor.x == close[j].coo.x && neighbor.y == close[j].coo.y) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            /* looking for the neighbor in the open list */
            struct step *s_found = NULL;
            for (int j = 0, je = arrlenu(open); j != je; ++j) {
                if (neighbor.x == open[j].coo.x && neighbor.y == open[j].coo.y) {
                    s_found = &open[j];
                    break;
                }
            }
            if (!s_found) {
                struct step step = { neighbor, current->g + 1, calc_h(neighbor, finish), 0 };
                open = arrputsorted(open, step);
            } else if (current->g + 1 + calc_h(neighbor, finish) < s_found->g + s_found->h) {
                s_found->g = current->g + 1;
            }
        }
    }

found:
    arrfree(open);
    /* Backtracking
     * filling in the path
     * putting neighbors if only their g < the current one and if they are in the close list
     */
    if (path_found) {
        for (int i = arrlenu(close) - 1, ie = 0; i >= ie; --i) {
            for (int j = 0, je = close[i].neighbors_num; j != je; ++j) {
                for (int k = 0, ke = arrlenu(close); k != ke; ++k) {
                    if (close[k].g < close[i].g &&
                        close[i].neighbors[j].x == close[k].coo.x &&
                        close[i].neighbors[j].y == close[k].coo.y) {
                        struct vec2 coo = { close[i].coo.x, close[i].coo.y };
                        arrput(rv.step, coo);
                    }
                }
            }
        }
        arrput(rv.step, dest);
    }

    arrfree(close);

    return rv;
}

static float
get_passability(struct unit *u, struct tile *t) {
    return .5;
}

/* it sorts struct step *s array from more to less f then from less to more g
 * so that the least item lays in the end of array
 */
static struct step *
arrputsorted(struct step *a, struct step v) {
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

