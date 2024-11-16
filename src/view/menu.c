#include "menu.h"
#ifndef NK_SDL_RENDERER_H_
  #include "nuklear_sdl_renderer.h"
#endif
#include "app.h"
#include "view.h"
#include <malloc.h>

/*
default nuklear styles
border: 2.000000, group border: 1.000000, spacing: 4.000000-4.000000, padding: 4.000000-4.000000, group padding: 4.000000-4.000000
        printf("border: %f, group border: %f, spacing: %f-%f, padding: %f-%f, group padding: %f-%f\n",
            ctx->style.window.border,
            ctx->style.window.group_border,
            ctx->style.window.spacing.x,
            ctx->style.window.spacing.y,
            ctx->style.window.padding.x,
            ctx->style.window.padding.y,
            ctx->style.window.group_padding.x,
            ctx->style.window.group_padding.y
        );
*/

/*
 *
 * Main menu
 *
 */

struct main_menu {
    struct nk_image bg;
};

void main_menu_init(struct view *view) {
    view->data = malloc(sizeof(struct main_menu));
    view->draw = main_menu_draw;

    struct main_menu *data = (struct main_menu *)view->data;
    data->bg = app_get_image(view->app, "menu_bg");
}

void main_menu_free(struct view *view) {
    free(view->data);
}

void main_menu_draw(struct view *view) {
    struct app *app = view->app;
    struct nk_context *ctx = app->ctx;
    struct nk_style_window *s = &ctx->style.window;
    struct main_menu *data = (struct main_menu *)view->data;
    int win_vert_padding = s->border + s->spacing.y * 2 + s->padding.y * 2;
    int group_vert_padding = win_vert_padding * 2 + s->group_border + s->group_padding.y * 2;
    struct vec2 win_size = get_win_size(app);
    int content_height = win_size.y - win_vert_padding;

    nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_image(data->bg));

    if (nk_begin(ctx, "main_menu", nk_rect(0, 0, win_size.x, win_size.y), 0)) {
        nk_layout_row_dynamic(ctx, content_height, 3);

        nk_spacer(ctx);
        nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nk_rgba(0, 0, 0, 127)));
        nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nk_rgba(0, 0, 0, 191)));
        if (nk_group_begin(ctx, "", 0)) {
            int group_height = content_height - group_vert_padding;
            int items_n = 4;
            int space = group_height / (items_n * 2 + 5);

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "New")) {
                app_set_view(app, "new_menu");
            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            nk_button_label(ctx, "Load");

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "Options")) {
                app_set_view(app, "options_menu");
            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "Quit")) {
                app_quit(app);
            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_group_end(ctx);
        }
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_spacer(ctx);
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
}

/*
 *
 * New menu
 *
 */

#define MAX_NAME_LEN 256

struct new_menu {
    struct nk_image bg;
    char name_buffer[MAX_NAME_LEN];
};

void new_menu_init(struct view *view) {
    view->data = malloc(sizeof(struct new_menu));
    view->draw = new_menu_draw;

    struct new_menu *data = (struct new_menu *)view->data;
    data->bg = app_get_image(view->app, "menu_bg");
    *data->name_buffer = '\0';
}

void new_menu_free(struct view *view) {
    free(view->data);
}

void new_menu_draw(struct view *view) {
    struct app *app = view->app;
    struct nk_context *ctx = app->ctx;
    struct nk_style_window *s = &ctx->style.window;
    struct new_menu *data = (struct new_menu *)view->data;
    int win_vert_padding = s->border + s->spacing.y * 2 + s->padding.y * 2;
    int group_vert_padding = win_vert_padding * 2 + s->group_border + s->group_padding.y * 2;
    struct vec2 win_size = get_win_size(app);
    int content_height = win_size.y - win_vert_padding;

    nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_image(data->bg));

    if (nk_begin(ctx, "new_menu", nk_rect(0, 0, win_size.x, win_size.y), 0)) {
        nk_layout_row_dynamic(ctx, content_height, 3);

        nk_spacer(ctx);
        nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nk_rgba(0, 0, 0, 127)));
        nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nk_rgba(0, 0, 0, 191)));
        nk_style_push_style_item(ctx, &ctx->style.combo.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.combo.hover, nk_style_item_color(nk_rgba(0, 0, 0, 127)));
        nk_style_push_style_item(ctx, &ctx->style.combo.active, nk_style_item_color(nk_rgba(0, 0, 0, 191)));
        if (nk_group_begin(ctx, "", 0)) {
            int group_height = content_height - group_vert_padding;
            int items_n = 4;
            int space = group_height / (items_n * 2 + 5);

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "Name:", NK_TEXT_RIGHT);
            if (nk_edit_string_zero_terminated(ctx,
                NK_EDIT_SIG_ENTER               |
                NK_EDIT_ALLOW_TAB               |
                NK_EDIT_SELECTABLE              |
                NK_EDIT_CLIPBOARD               |
                NK_EDIT_NO_HORIZONTAL_SCROLL    |
                NK_EDIT_ALWAYS_INSERT_MODE
                , data->name_buffer, MAX_NAME_LEN, nk_filter_default) & NK_EDIT_COMMITED) {

            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "Start")) {
                struct vec2 ws = { 1024, 1024 };
                app_gen_world(app, ws); //TODO: it's better to move these 2 lines into app code
                struct vec2 center = { app->cur_world->value.player->coords.x, app->cur_world->value.player->coords.y };
                app_set_view(app, "main_view");
                main_view_center_at(&app->cur_view->value, center);
            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "Main menu")) {
                app_set_view(app, "main_menu");
            }

            nk_group_end(ctx);
        }
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_spacer(ctx);
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
}

/*
 *
 * Options menu
 *
 */

struct options_menu {
    struct nk_image bg;
};

void options_menu_init(struct view *view) {
    view->data = malloc(sizeof(struct options_menu));
    view->draw = options_menu_draw;

    struct options_menu *data = (struct options_menu *)view->data;
    data->bg = app_get_image(view->app, "menu_bg");
}

void options_menu_free(struct view *view) {
    free(view->data);
}

void options_menu_draw(struct view *view) {
    struct app *app = view->app;
    struct nk_context *ctx = app->ctx;
    struct nk_style_window *s = &ctx->style.window;
    struct options_menu *data = (struct options_menu *)view->data;
    int win_vert_padding = s->border + s->spacing.y * 2 + s->padding.y * 2;
    int group_vert_padding = win_vert_padding * 2 + s->group_border + s->group_padding.y * 2;
    struct vec2 win_size = get_win_size(app);
    int content_height = win_size.y - win_vert_padding;

    nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_image(data->bg));

    if (nk_begin(ctx, "options_menu", nk_rect(0, 0, win_size.x, win_size.y), 0)) {
        nk_layout_row_dynamic(ctx, content_height, 3);

        nk_spacer(ctx);
        nk_style_push_style_item(ctx, &s->fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nk_rgba(0, 0, 0, 127)));
        nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nk_rgba(0, 0, 0, 191)));
        nk_style_push_style_item(ctx, &ctx->style.combo.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        nk_style_push_style_item(ctx, &ctx->style.combo.hover, nk_style_item_color(nk_rgba(0, 0, 0, 127)));
        nk_style_push_style_item(ctx, &ctx->style.combo.active, nk_style_item_color(nk_rgba(0, 0, 0, 191)));
        if (nk_group_begin(ctx, "", 0)) {
            int group_height = content_height - group_vert_padding;
            int items_n = 4;
            int space = group_height / (items_n * 2 + 5);

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Video mode:", NK_TEXT_RIGHT);
            if (nk_combo_begin_label(ctx, app->video_mode == WINDOWED ? "Windowed" : "Full screen", nk_vec2(nk_widget_width(ctx), 200))) {
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_combo_item_label(ctx, "Windowed", NK_TEXT_LEFT)) { set_video_mode(app, WINDOWED); }
                if (nk_combo_item_label(ctx, "Full screen", NK_TEXT_LEFT)) { set_video_mode(app, FULLSCREEN); }
                nk_combo_end(ctx);
            }

            nk_layout_row_dynamic(ctx, space, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, space * 2, 1);
            if (nk_button_label(ctx, "Main menu")) {
                app_set_view(app, "main_menu");
            }

            nk_group_end(ctx);
        }
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_style_pop_style_item(ctx);
        nk_spacer(ctx);
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
}

