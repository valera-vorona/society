#ifndef _TILESET_H_
#define _TILESET_H_

#include "types.h"
#include "nuklear.h"

#define neighbor_left  0x8
#define neighbor_up    0x4
#define neighbor_right 0x2
#define neighbor_down  0x1

void tileset_init(struct tileset *t);
void tileset_get_rect(struct tileset *t, int x, int y, struct rect *r);
#define tileset_get_rect_by_index(t, n, r) tileset_get_rect((t), (n) % t->tileset_size.x, (n) / t->tileset_size.x, (r))
struct nk_image tileset_get(struct tileset *t, int x, int y);
#define tileset_get_by_index(t, n) tileset_get((t), (n) % t->tileset_size.x, (n) / t->tileset_size.x)
int tileset_quad_get_tile_index(struct tileset *t, int n, int neighbors);

#endif /* _TILESET_H_ */

