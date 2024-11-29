#include "tileset.h"

void
tileset_init(struct tileset *t) {

}

struct
nk_image tileset_get(struct tileset *t, struct vec2 n) {
    struct nk_rect r = {
        t->margin.x + (t->padding.x + t->tile_size.x) * n.x,
        t->margin.y + (t->padding.y + t->tile_size.y) * n.y,
        t->tile_size.x,
        t->tile_size.y
    };

    return nk_subimage_handle(t->image.handle, t->image_size.x, t->image_size.y, r);
}

int
tileset_quad_get_tile_index(struct tileset *t, int n, int neighbors) {
    int q = t->tileset_size.x / 4;
    struct vec2 quad = { (n % q) * 4, (n / q) * 4 };
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

    return quad.y + t->tileset_size.x + quad.x;
}


