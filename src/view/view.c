#include "view.h"
#ifndef NK_SDL_RENDERER_H_
  #include "nuklear_sdl_renderer.h"
#endif
#include "app.h"
#include "path.h"
#include "ai.h"
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
    enum {
        AM_NONE = 0,
        AM_WALK
    } action_mode;
    struct path path;
    struct vec2 prev_hovered_coo;
    struct nk_image iconset;
    struct nk_image landset;
    struct nk_image unitset;
};

void main_view_init(struct view *view) {
    view->data = malloc(sizeof(struct main_view));
    view->draw = main_view_draw;

    struct main_view *data = (struct main_view *)view->data;
    data->zoom = data->mid.x = data->mid.y = 0;
    main_view_zoom(view, 0);
    data->action_mode = AM_WALK;
    path_init(&data->path);
    data->prev_hovered_coo.x = -1;
    data->prev_hovered_coo.y = -1;
    data->iconset = app_get_image(view->app, "iconset");
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

void main_view_center_at(struct view *view, struct vec2 coo) {
    struct main_view *v = (struct main_view *)view->data;
    v->mid.x = trim(0, 1000000, coo.x);
    v->mid.y = trim(0, 1000000, coo.y);
}

#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)

void main_view_draw(struct view *view) {
    struct app *app = view->app;
    struct world *w = &app->cur_world->value;
    struct nk_context *ctx = app->ctx;
    struct nk_style_window *s = &ctx->style.window;
    struct main_view *data = (struct main_view *)view->data;
    struct map *map = &w->map;
    struct unit *units = w->units;
    struct unit *player = w->player_ai->unit;
    struct vec2 win_size = get_win_size(app);

    enum nk_widget_layout_states state;
    struct nk_vec2 *mouse_pos = &ctx->input.mouse.pos;
    struct nk_rect space;                                                   /* widget space */
    struct nk_rect src = { 16, 16, 64, 64 };                                /* source frame in landset */
    struct nk_rect dest = { 0, 0, data->dest_size.x, data->dest_size.y };   /* destination frame in the widget */
    struct vec2 left_margin;
    struct vec2 half_space;
    struct rect frame;

    struct tile *hovered_tile = NULL;
    struct vec2 hovered_coo;

    if (nk_begin(ctx, "tile_view", nk_rect(0, 0, win_size.x, win_size.y), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_rect content = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, content.h - content.y, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        state = nk_widget(&space, ctx);
        if (state) {
            /* destination from left top corner to the center of the widget */
            half_space.x    = (space.w - space.x) / 2;
            half_space.y    = (space.h - space.y) / 2;
            data->mid.x     = max(data->mid.x, half_space.x);
            data->mid.y     = max(data->mid.y, half_space.y);
            /* partly shown tile size in the top left corner */
            left_margin.x   = (data->mid.x - half_space.x) % data->dest_size.x;
            left_margin.y   = (data->mid.y - half_space.y) % data->dest_size.y;
            /* frame in the world map to draw */
            /* TODO: these two lines zoom correctly but scroll incorrectly */
            //frame.x         = data->mid.x / 64 - half_space.x / data->dest_size.x;
            //frame.y         = data->mid.y / 64 - half_space.y / data->dest_size.y;
            /* these two lines zoom incorrectly but scroll correctly */
            frame.x         = (data->mid.x - half_space.x) / data->dest_size.x;
            frame.y         = (data->mid.y - half_space.y) / data->dest_size.y;
            frame.w         = min((space.w - space.x) / data->dest_size.x + 2, map->size.x - frame.x);
            frame.h         = min((space.h - space.y) / data->dest_size.y + 2, map->size.y - frame.y);
            /* hovered  coords and tile */
            hovered_coo.x = frame.x + ((int)mouse_pos->x + left_margin.x) / data->dest_size.x;
            hovered_coo.y = frame.y + ((int)mouse_pos->y + left_margin.y) / data->dest_size.y;
            hovered_tile = &map->tiles[hovered_coo.y * map->size.x + hovered_coo.x];

            /* drawing map */
            for (int y = frame.y; y < frame.y + frame.h; ++y) {
                for (int i = y * map->size.x + frame.x, x = frame.x; x < frame.x + frame.w; ++i, ++x) {
                    /* drawing tile */
                    struct tile *tile = &map->tiles[i];
                    src.y = 16 + tile->type * 72;
                    struct nk_image sub = nk_subimage_handle(data->landset.handle, 1176, 1176, src);
                    dest.x = (x - frame.x) * dest.w - left_margin.x;
                    dest.y = (y - frame.y) * dest.h - left_margin.y;
                    nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
                }
            }

            /* handling mouse press the map_view */
            if (data->action_mode == AM_WALK && (data->prev_hovered_coo.x != hovered_coo.x || data->prev_hovered_coo.y != hovered_coo.y)) {
                if (data->path.steps)
                    path_free(&data->path);
                data->path = find_path(w, player, hovered_coo);
            }

            if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
                if (!path_is_free(data->path)) {
                    ai_add_task_from_path(w->player_ai, data->path);
                }
            }
        }
    }
    nk_end(ctx);

    nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
    if (nk_begin(ctx, "unit_view", nk_rect(0, 0, win_size.x, win_size.y), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_rect content = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, content.h - content.y, 1);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        enum nk_widget_layout_states state2 = nk_widget(&space, ctx);
        if (state && state2) {
            /* drawing map, units */
            for (int y = frame.y; y < frame.y + frame.h; ++y) {
                for (int i = y * map->size.x + frame.x, x = frame.x; x < frame.x + frame.w; ++i, ++x) {
                    struct tile *tile = &map->tiles[i];
                    struct nk_image sub;
                    dest.x = (x - frame.x) * dest.w - left_margin.x;
                    dest.y = (y - frame.y) * dest.h - left_margin.y;

                    /* drawing unit if exists on current tile */
                    if (tile->units[0] != ID_NOTHING) {
                        struct unit *u = &units[tile->units[0]];
                        dest.x += (u->coords.x % 64) * data->dest_size.x / 64; // it should be x * data->dest_size.x / 64
                        dest.y += (u->coords.y % 64) * data->dest_size.y / 64; // the same but y instead of x
                        struct nk_rect r = { dest.x, dest.y, 16, 24 };
                        nk_fill_rect(canvas, r, 0, nk_rgba(255, 0, 0, 255));

                        /* drawing circle under the player */
                        if (u->flags & UF_PLAYER) {
                            src.y = 16;
                            sub = nk_subimage_handle(data->iconset.handle, 1176, 1176, src);
                            nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
                        }

                        src.y = 16 + 72;
                        sub = nk_subimage_handle(data->unitset.handle, 1176, 1176, src);
                        nk_draw_image(canvas, dest, &sub, nk_rgba(255, 255, 255, 255));
                    }
                }
            }
        }
    }
    nk_end(ctx);
    nk_style_pop_style_item(ctx);

    if (nk_begin(ctx, "mini_map_view", nk_rect(win_size.x - 200, 0, 200, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE)) {
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

            /* handling mouse in the mini_map_view, now it works incorrecnly */
            if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_DOUBLE)) {
                struct vec2 coo = { (frame.x + (int)mouse_pos->x - (win_size.x - 200)) * 64, (frame.y + (int)mouse_pos->y) * 64 };
                main_view_center_at(view, coo);
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
            nk_label(ctx, "Terrian:" , NK_TEXT_LEFT);
        }

    }
    nk_end(ctx);

    data->prev_hovered_coo.x = hovered_coo.x;
    data->prev_hovered_coo.y = hovered_coo.y;
}

