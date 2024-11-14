#include "app.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
#undef main
#endif

int main(int argc, char *argv[]) {
    int rv;
    struct app app;
    struct vec2 win_size = { 1600, 900 };

    rv = app_init(&app, 0, win_size);
    if (rv) return rv;

    app_run(&app);
    app_free(&app);

    return 0;
}

