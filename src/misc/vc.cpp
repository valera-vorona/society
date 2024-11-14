/* This file is not currently used but may be helpful for loading images and
 videos if it is decided to refuse stb_image.h in favor for OpenCV library.
 */

#include <opencv2/opencv.hpp>
#include <SDL2/SDL.h>
#include <stdio.h>

extern "C"
SDL_Texture *mat_to_tex(const cv::Mat &im, SDL_Renderer *renderer) {
    using namespace cv;

    int x,y,n,pitch;
    Uint32 fmt;

    x = im.size().width;
    y = im.size().height;
    n = im.elemSize();
    pitch = (x * n) % 4 ? 1 : 4;

    switch (n) {
        case 2:     fmt = SDL_PIXELFORMAT_BGRA4444; break; // not sure
        case 3:     fmt = SDL_PIXELFORMAT_BGR24; break;
        case 4:     fmt = SDL_PIXELFORMAT_BGRA32; break;
        default:    fmt = SDL_PIXELFORMAT_INDEX8; break; // not sure
    }

    SDL_Texture *g_SDLTexture = SDL_CreateTexture(renderer, fmt, SDL_TEXTUREACCESS_STATIC, x, y);
    if (g_SDLTexture == NULL) {
        fprintf(stderr, "Error creating texture: %s", SDL_GetError());
        return NULL;
    }

    SDL_UpdateTexture(g_SDLTexture, NULL, im.ptr(), x*n);

    return g_SDLTexture;
}

extern "C"
SDL_Texture *load_image(const char *fname, SDL_Renderer *renderer) // throw std::runtime_error
{
    using namespace cv;

    auto im = imread(fname, IMREAD_UNCHANGED);
    if (im.empty()) {
        fprintf(stderr, "Can't open image file: '%s'\n", fname);
        return NULL;
    }

    return mat_to_tex(im, renderer);
}

