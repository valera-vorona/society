#include "app.h"
#include "icon.h"
#include "menu.h"
#include "view.h"
#include "gen.h"
#include "world.h"
#include "stb_ds.h"
#include "serial.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>

int run(struct app *app);

#define DATA_PATH "assets/git-lfs-files/install/"

void run_init(struct app *app);

void app_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

int app_init(struct app *app, uint32_t seed, struct vec2 size) {
    char fbuf[4096];

    /* initializing random number generator*/
    app->seed = seed ? seed : random_device();
    mt_init_state(&app->mt, app->seed);

    /* reading app app info */
    app->json = read_json(DATA_PATH "app.json");
    if (!app->json) return 1;

    /* init app name */
    struct jq_value *val;
    struct jq_pair *pair;
    val = jq_find(app->json, "app", "name", 0);
    app->name = val && jq_isstring(val) ? val->value.string : "society";

    /* init worlds */
    app->worlds = NULL;
    val = jq_find(app->json, "worlds", 0);
    if (val && jq_isobject(val)) {
        jq_foreach_object(pair, val) {
            if (jq_isstring(&pair->value)) {
                struct world w = { };
                strcpy(fbuf, DATA_PATH);
                strcat(fbuf, pair->value.value.string);
                if (world_init(&w, fbuf, &app->mt)) return 1;
                shput(app->worlds, pair->key, w);
            } else {
                app_warning("World entry shell be a pair of name and filename");
                return 1;
            }
        }
    } else {
        app_warning("'worlds' doesn't exist or is not an object");
        return 1;
    }

    /* init cur_world */
    val = jq_find(app->json, "cur_world", 0);
    if (val && jq_isstring(val)) {
        if (jq_find(app->json, "worlds", val->value.string, 0)) {
            app->cur_world = shgetp(app->worlds, val->value.string);
        } else {
            app_warning("World '%s' doest't exist within 'worlds'", val->value.string);
            return 1;
        }
    } else {
        app_warning("'cur_world' doesn't exist or is not a string");
        return 1;
    }

    app->running = 1;
    app->video_mode = WINDOWED;
    app->win_size = size;
    run_init(app);

    /* init images */
    app->images = NULL;
    shput(app->images, "menu_bg", nk_image_ptr(nk_sdl_device_upload_image(DATA_PATH "menu_bg.jpg", SDL_TEXTUREACCESS_STATIC)));
    shput(app->images, "iconset", nk_image_ptr(nk_sdl_device_upload_image(DATA_PATH "iconset.png", SDL_TEXTUREACCESS_TARGET)));
    shput(app->images, "landset", nk_image_ptr(nk_sdl_device_upload_image(DATA_PATH "landset.png", SDL_TEXTUREACCESS_STATIC)));
    shput(app->images, "unitset", nk_image_ptr(nk_sdl_device_upload_image(DATA_PATH "unitset.png", SDL_TEXTUREACCESS_STATIC)));

    /* generate image rotations */
    if (icon_dup(app->renderer, shget(app->images, "iconset"))) {
        return 1;
    }

    /* init views */
    app->views = NULL;
    struct view view = { app };
    main_menu_init(&view);
    shput(app->views, "main_menu", view);

    new_menu_init(&view);
    shput(app->views, "new_menu", view);

    options_menu_init(&view);
    shput(app->views, "options_menu", view);

    main_view_init(&view);
    shput(app->views, "main_view", view);

    app_set_view(app, "main_menu");

    return 0;
}

void app_free(struct app *app) {
    main_view_free(&shget(app->views, "main_view"));
    options_menu_free(&shget(app->views, "options_menu"));
    new_menu_free(&shget(app->views, "new_menu"));
    main_menu_free(&shget(app->views, "main_menu"));

    shfree(app->views);
    shfree(app->images);

    for (int i = 0, ie = shlenu(app->worlds); i != ie; ++i)
        world_free(&app->worlds[i].value);

    shfree(app->worlds);

    jq_free(app->json);
}

void app_run(struct app *app) {
    run(app);
}

void app_step(struct app *app) {
    world_step(&app->cur_world->value);
}

void app_quit(struct app *app) {
    app->running = 0;
}

void set_video_mode(struct app *app, enum video_mode mode) {
    app->video_mode = mode;
    SDL_SetWindowFullscreen(app->win, mode == FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0);
}

void app_set_view(struct app *app, const char *name) {
    app->cur_view = shgetp_null(app->views, name);
}

void app_zoom(void *userdata, int x, int y) {
    struct app *app = (struct app *)userdata;
    if (!strcmp(app->cur_view->key, "main_view")) {
        main_view_zoom(&app->cur_view->value, y); 
    }
}

void app_scroll_map(void *userdata, int x, int y) {
    struct app *app = (struct app *)userdata;
    if (!strcmp(app->cur_view->key, "main_view")) {
        struct vec2 delta = { x, y };
        main_view_scroll(&app->cur_view->value, delta); 
    }
}

struct nk_image app_get_image(struct app *app, const char *name) {
    return shget(app->images, name);
}

struct vec2 get_win_size(struct app *app) {
    struct vec2 rv = { 0, 0 };

    switch (app->video_mode) { 
    case WINDOWED: return app->win_size;
    case FULLSCREEN: SDL_GetWindowSize(app->win, &rv.x, &rv.y); return rv;
    }

    return rv; /* to aviod compiler warning */
}

void app_gen_world(struct app *app, struct vec2 size) {
    gen_world(&app->cur_world->value, size, app->seed);
}

void app_draw(struct app *app) {
    app->cur_view->value.draw(&app->cur_view->value);
}

