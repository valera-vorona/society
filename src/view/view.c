#include "view.h"
#ifndef NK_SDL_RENDERER_H_
  #include "nuklear_sdl_renderer.h"
#endif
#include "app.h"
#include "stb_ds.h"
#include <malloc.h>

/*
 *
 * Main view 
 *
 */

struct main_view {
    int zoom;
    struct vec2 mid;            /* pixel that should be shown in the middle of the map */
    struct vec2 dest_size;      /* size of shown tile */
    struct nk_image landset;
    struct nk_image unitset;
};

void main_view_init(struct view *view) {
    view->data = malloc(sizeof(struct main_view));
    view->draw = main_view_draw;

    struct main_view *data = (struct main_view *)view->data;
    data->zoom = data->mid.x = data->mid.y = 0;
    main_view_zoom(view, 0);
    data->landset = app_get_image(view->app, "landset");
    data->unitset = app_get_image(view->app, "unitset");
}

void main_view_free(struct view *view) {
    free(view->data);
}

void main_view_zoom(struct view *view, int delta) {
    struct main_view *v = (struct main_view *)view->data;
    v->zoom = trim(-3, 3, v->zoom + delta);

    switch (v->zoom) {
    case -3: v->dest_size.x = 24; v->dest_size.y = 24; break;
    case -2: v->dest_size.x = 32; v->dest_size.y = 32; break;
    case -1: v->dest_size.x = 48; v->dest_size.y = 48; break;
    case  0: v->dest_size.x = 64; v->dest_size.y = 64; break;
    case  1: v->dest_size.x = 96; v->dest_size.y = 96; break;
    case  2: v->dest_size.x = 128; v->dest_size.y = 128; break;
    case  3: v->dest_size.x = 192; v->dest_size.y = 192; break;
    }
}

void main_view_scroll(struct view *view, struct vec2 delta) {
    struct main_view *v = (struct main_view *)view->data;
    v->mid.x = trim(0, 100000, v->mid.x + delta.x);
    v->mid.y = trim(0, 100000, v->mid.y + delta.y);
}

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

void main_view_draw(struct view *view) {
    struct app *app = view->app;
    struct nk_context *ctx = app->ctx;
    struct nk_style_window *s = &ctx->style.window;
    struct main_view *data = (struct main_view *)view->data;
    struct map *map = &app->cur_world->value.map;
    struct unit *units = app->cur_world->value.units;
    struct vec2 win_size = get_win_size(app);

    struct nk_vec2 *mouse_pos = &ctx->input.mouse.pos;
    struct tile *hovered_tile = NULL;
    struct vec2 hovered_coo;

    if (nk_begin(ctx, "main_view", nk_rect(0, 0, win_size.x, win_size.y), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_rect content = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, content.h - content.y, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        struct nk_rect space;                                                       /* widget space */
        enum nk_widget_layout_states state = nk_widget(&space, ctx);
        if (state) {
            struct nk_rect src = { 16, 16, 64, 64 };                                /* source frame in landset */
            struct nk_rect dest = { 0, 0, data->dest_size.x, data->dest_size.y };   /* destination frame in the widget */

            /* destination from left top corner to the center of the widget */
            struct vec2 half_space = { (space.w - space.x) / 2, (space.h - space.y) / 2 };
            data->mid.x = max(data->mid.x, half_space.x);
            data->mid.y = max(data->mid.y, half_space.y);
            /* frame in the world map to draw */
            struct rect frame  = {
                (data->mid.x  - space.x / 2) / data->dest_size.x,
                (data->mid.y - space.y / 2 ) / data->dest_size.y,
                min((space.w - space.x) / data->dest_size.x + 2, map->size.x - frame.x),
                min((space.h - space.y) / data->dest_size.y + 2, map->size.y - frame.y)
            };

            for (int y = frame.y; y < frame.y + frame.h; ++y) {
                for (int i = y*map->size.x + frame.x, x = frame.x; x < frame.x + frame.w; ++i, ++x) {
                    src.y = 16 + map->tiles[i].type * 72;
                    struct nk_image sub = nk_subimage_handle(data->landset.handle, 1176, 1176, src);
                    dest.x = (x - frame.x) * dest.w - (data->mid.x % data->dest_size.x);
                    dest.y = (y - frame.y) * dest.h - (data->mid.y % data->dest_size.y);
                    nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
                }
            }

            hovered_tile = &map->tiles[(frame.y + (int)mouse_pos->y / data->dest_size.y)*map->size.x + frame.x + (int)mouse_pos->x / data->dest_size.x];
            hovered_coo.x = frame.x + (int)mouse_pos->x / data->dest_size.x;
            hovered_coo.y = frame.y + (int)mouse_pos->y / data->dest_size.y;

            src.y = 16 + 72;
            struct nk_image sub = nk_subimage_handle(data->unitset.handle, 1176, 1176, src);
            for (int i = 0, ie = arrlenu(units); i < ie; ++i) {
                struct unit *u = &units[i];
                dest.x = u->coords.x*data->dest_size.x/64 - data->mid.x - half_space.x;
                dest.y = u->coords.y*data->dest_size.y/64 - data->mid.y - half_space.y;
                nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
            }
        }
    }
    nk_end(ctx);

    if (nk_begin(ctx, "mini_map_view", nk_rect(win_size.x - 200, win_size.y - 200, 200, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE)) {
        struct nk_rect content = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, content.h - content.y, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        struct nk_rect space;                           /* widget space */
        enum nk_widget_layout_states state = nk_widget(&space, ctx);
        if (state) {
            struct nk_rect src = { 16, 0, 64, 64 };     /* source frame in landset */
            struct nk_rect dest = { 0, 0, 1, 1 };       /* destination frame in the widget */
            /* frame in the world map to draw */
            struct nk_rect frame  = { 0, 0, min(120, map->size.x), min(100, map->size.y) };

            for (int y = frame.y; y < frame.h; ++y) {
                for (int i = y*map->size.x, x = frame.x; x < frame.w; ++i, ++x) {
                    src.y = 16 + map->tiles[i].type * 72;
                    struct nk_image sub = nk_subimage_handle(data->landset.handle, 1176, 1176, src);
                    dest.x = space.x + x;
                    dest.y = space.y + y;
                    nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
                }
            }
        }
    }
    nk_end(ctx);

    if (nk_begin(ctx, "info_view", nk_rect(win_size.x - 200, win_size.y - 500, 200, 300), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE)) {
        char str[128];
        struct nk_rect content = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, 20, 1);

        if (hovered_tile) {
            snprintf(str, sizeof(str), "Terrian: %s", map->tile_types[hovered_tile->type].name);
            nk_label(ctx, str, NK_TEXT_LEFT);
            snprintf(str, sizeof(str), "Coordinates: %i:%i", hovered_coo.x, hovered_coo.y);
            nk_label(ctx, str, NK_TEXT_LEFT);
        } else {
            nk_label(ctx, "terrian:" , NK_TEXT_LEFT);
        }

    }
    nk_end(ctx);
}

