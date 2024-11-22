#include "nuklear.h"
#include "app.h"
#include <SDL2/SDL.h>

static int
set_dest_image(SDL_Renderer *renderer, struct nk_image *image) {
    if (SDL_SetRenderTarget(renderer, image->handle.ptr)) {
        app_warning(SDL_GetError());
        return 1;
    }

    return 0;
}

static int
dup(
    SDL_Renderer *renderer,
    struct nk_image *src_i,
    struct SDL_Rect *src_r,
    struct SDL_Rect *dst_r,
    double angle,
    struct SDL_Point *center,
    SDL_RendererFlip flip) {
    if (SDL_RenderCopyEx(renderer, src_i->handle.ptr, src_r, dst_r, angle, center, flip)) {
        app_warning(SDL_GetError());
        return 1;
    }

    return 0;
}

int
icon_dup(SDL_Renderer *renderer, struct nk_image image) {
    SDL_Texture *old_texture = SDL_GetRenderTarget(renderer);
    struct SDL_Rect s = { 16, 16 + 72, 64, 64 };
    struct SDL_Rect d = { 0, 16 + 72, 64, 64 };

    if (set_dest_image(renderer, &image))
        return 1;

    /* dup path lines */
    d.x = 16 + 72*2;
    dup(renderer, &image, &s, &d, 90, NULL, SDL_FLIP_NONE); 
    d.x = 16 + 72*4;
    dup(renderer, &image, &s, &d, 180, NULL, SDL_FLIP_NONE); 
    d.x = 16 + 72*6;
    dup(renderer, &image, &s, &d, 270, NULL, SDL_FLIP_NONE); 

    s.x = 16 + 72;
    d.x = 16 + 72*3;
    dup(renderer, &image, &s, &d, 90, NULL, SDL_FLIP_NONE); 
    d.x = 16 + 72*5;
    dup(renderer, &image, &s, &d, 180, NULL, SDL_FLIP_NONE); 
    d.x = 16 + 72*7;
    dup(renderer, &image, &s, &d, 270, NULL, SDL_FLIP_NONE); 

    /* Don't forget to restore the old render target */
    image.handle.ptr = old_texture;
    return set_dest_image(renderer, &image);
}

