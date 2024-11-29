#ifndef _TILESET_H_
#define _TILESET_H_

#include "types.h"
#include "nuklear.h"

#define neighbor_left  0x8
#define neighbor_up    0x4
#define neighbor_right 0x2
#define neighbor_down  0x1

struct tileset {
    struct vec2 margin;
    struct vec2 padding;
    struct vec2 tile_size;
    struct vec2 tileset_size;
    struct nk_image image;
    struct vec2 image_size;
};

void tileset_init(struct tileset *t);
struct nk_image tileset_get(struct tileset *t, struct vec2 n);
int tileset_quad_get_tile_index(struct tileset *t, int n, int neighbors);

#endif /* _TILESET_H_ */

