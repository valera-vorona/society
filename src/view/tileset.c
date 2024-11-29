#include "tileset.h"

void
tileset_init(struct tileset *t) {

}

struct
nk_image tileset_get(struct tileset *t, int x, int y) {
    struct nk_rect r = {
        t->margin.x + (t->padding.x + t->tile_size.x) * x,
        t->margin.y + (t->padding.y + t->tile_size.y) * y,
        t->tile_size.x,
        t->tile_size.y
    };

    return nk_subimage_handle(t->image.handle, t->image_size.x, t->image_size.y, r);
}

int
tileset_quad_get_tile_index(struct tileset *t, int n, int neighbors) {
    struct vec2 quad = { (n % t->quad_size.x) * t->quad_size.x, (n / t->quad_size.x) * t->quad_size.y };

    switch (neighbors) {
    case                                              0: ++quad.x; ++quad.y; break;
    case                    neighbor_left | neighbor_up: break;
    case                                    neighbor_up: ++quad.x; break;
    case                   neighbor_right | neighbor_up: quad.x += 2; break;

    case                                  neighbor_left: ++quad.y; break;
    case                                 neighbor_right: quad.x +=2; ++quad.y; break;

    case                  neighbor_left | neighbor_down: quad.y += 2; break;
    case                                  neighbor_down: ++quad.x; quad.y += 2; break;
    case                 neighbor_right | neighbor_down: quad.x += 2; quad.y += 2; break;

    case   neighbor_left | neighbor_up | neighbor_right: quad.x += 3; break;
    case                 neighbor_left | neighbor_right: quad.x += 3; ++quad.y; break;
    case neighbor_left | neighbor_right | neighbor_down: quad.x += 3; quad.y += 2; break;

    case    neighbor_left | neighbor_up | neighbor_down: quad.y += 3; break;
    case                    neighbor_up | neighbor_down: ++quad.x; quad.y += 3; break;
    case   neighbor_up | neighbor_right | neighbor_down: quad.x += 2; quad.y += 3; break;

    case neighbor_left | neighbor_right | neighbor_up | neighbor_down: quad.x += 3; quad.y += 3; break;
    }

    return quad.y * t->tileset_size.x + quad.x;
}


