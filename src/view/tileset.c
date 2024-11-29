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

#define n_left  0x8
#define n_up    0x4
#define n_right 0x2
#define n_down  0x1

struct
nk_image tileset_quad_get(struct tileset *t, int n, int neighbors) {
    int q = t->tileset_size.x / 4;
    struct vec2 quad = { (n % q) * 4, (n / q) * 4 };
    switch (neighbors) {
    case                    0: ++quad.x; ++quad.y; break;
    case        n_left | n_up: break;
    case                 n_up: ++quad.x; break;
    case       n_right | n_up: quad.x += 2; break;

    case               n_left: ++quad.y; break;
    case              n_right: quad.x +=2; ++quad.y; break;

    case      n_left | n_down: quad.y += 2; break;
    case               n_down: ++quad.x; quad.y += 2; break;
    case     n_right | n_down: quad.x += 2; quad.y += 2; break;

    case n_left | n_up | n_right: quad.x += 3; break;
    case     n_left | n_right: quad.x += 3; ++quad.y; break;
    case n_left | n_right | n_down: quad.x += 3; quad.y += 2; break;

    case n_left | n_up | n_down: quad.y += 3; break;
    case        n_up | n_down: ++quad.x; quad.y += 3; break;
    case n_up | n_right | n_down: quad.x += 2; quad.y += 3; break;

    case n_left | n_right | n_up | n_down: quad.x += 3; quad.y += 3; break;
    }

    return tileset_get(t, quad);
}


