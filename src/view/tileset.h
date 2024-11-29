#ifndef _TILESET_H_
#define _TILESET_H_

#include "types.h"
#include "nuklear.h"

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
struct nk_image tileset_quad_get(struct tileset *t, int n, int neighbors);

#endif /* _TILESET_H_ */

