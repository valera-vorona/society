#ifndef _APP_H_
#define _APP_H_

#include "types.h"
#include "rand.h"
#ifndef NK_SDL_RENDERER_H_
  #include "nuklear_sdl_renderer.h"
#endif

struct nk_context;
struct nk_image;
struct app;
struct view;
struct SDL_Window;
struct SDL_Renderer;
struct jq_value;

typedef void (*draw_func)(struct view *view);

enum video_mode {
    WINDOWED,
    FULLSCREEN
};

struct nk_image_hash {
    char *key;
    struct nk_image value;
};

struct view {
    struct app *app;
    void *data;
    draw_func draw;
};

struct view_hash {
    char *key;
    struct view value;
};

struct world_hash {
    char *key;
    struct world value;
};

struct app {
    char *name;
    uint32_t seed;
    struct mt_state mt;
    int running;
    enum video_mode video_mode;
    struct SDL_Window *win;
    struct SDL_Renderer *renderer;
    struct nk_context *ctx;
    struct vec2 win_size;
    struct nk_image_hash *images;
    struct view_hash *views;
    struct view_hash *cur_view; /* this is a hash entry */
    struct world_hash *worlds;
    struct world_hash *cur_world;
    struct jq_value *json;
};

void app_warning(const char *format, ...);
int app_init(struct app *app, uint32_t seed, struct vec2 size);
void app_free(struct app *app);
void app_run(struct app *app);
void app_step(struct app *app);
void app_quit(struct app *app);
void set_video_mode(struct app *app, enum video_mode mode);
void app_set_view(struct app *app, const char *name);
void app_zoom(void *userdata, int x, int y);
void app_scroll_map(void *userdata, int x, int y);
struct nk_image app_get_image(struct app *app, const char *name);
struct vec2 get_win_size(struct app *app);
void app_gen_world(struct app *app, struct vec2 size);
void app_draw(struct app *app);

#endif /* _APP_H_ */

