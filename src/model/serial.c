#include "serial.h"
#include "app.h"
#include "stb_ds.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

static char *read_file(const char *fname, size_t *rsz);

struct jq_value *read_json(const char *fname) {
    size_t sz;
    struct jq_handler h;
    struct jq_value *rv;

    char *buf = read_file(fname, &sz);
    if (!buf) {
        return NULL;
    }

    jq_init(&h);

    rv = jq_read_buf(&h, buf, sz);

    if (!rv) app_warning("%s in file '%s'", jq_errstr(jq_get_error(&h)), fname);

    free(buf);

    return rv;
}

struct nk_image_hash *
read_images(struct jq_value *json) {
    char buf[4096];
    struct nk_image_hash *rv = NULL;
    struct jq_value *val, *v;
    struct jq_pair *pair;

    if (json && jq_isobject(json)) {
        jq_foreach_object(pair, json) {
            val = &pair->value;
            if (jq_isobject(val)) {
                SDL_TextureAccess access;
                void *tex;
                char *file;
                v = jq_find(val, "file", 0);
                if (!v || !jq_isstring(v)) {
                    app_warning("Image file doesn't exist or is not a string");
                    return NULL;
                } else {
                    file = v->value.string;
                }

                v = jq_find(val, "access", 0);
                if (v) {
                    if (!jq_isstring(v)) {
                        app_warning("Image access doesn't exist or is not a string");
                        return NULL;
                    }

                    if (!strcmp(v->value.string, "static")) {
                        access = SDL_TEXTUREACCESS_STATIC;
                    } else if (!strcmp(v->value.string, "streaming")) {
                        access = SDL_TEXTUREACCESS_STREAMING;
                    } else if (!strcmp(v->value.string, "target")) {
                        access = SDL_TEXTUREACCESS_TARGET;
                    } else {
                        app_warning("'access' %s is unknown, it should be one of 'static', 'streaming' or 'target'", v->value.string);
                        return NULL;
                    }
                } else {
                    /* setting default access */
                    access = SDL_TEXTUREACCESS_STATIC;
                }

                snprintf(buf, sizeof(buf), BINARY_PATH "%s", file);
                tex = nk_sdl_device_upload_image(buf, access);
                if (!tex) return NULL;
                shput(rv, pair->key, nk_image_ptr(tex));

            } else {
                app_warning("Image entry shell be a pair of name and json object");
                return NULL;
            }
        }
    } else {
        app_warning("'images' doesn't exist or is not an object");
        return NULL;
    }

    return rv;
}

static char *read_file(const char *fname, size_t *rsz) {
    size_t sz;
    char *rv = NULL;
    FILE *fp = fopen(fname, "r");

    if (fp == NULL) {
        app_warning("Error opening file '%s'", fname);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rv = (char *)malloc(sz);
    if (rv == NULL) {
        app_warning("Error allocating memory");
    } else {
        *rsz = fread(rv, sizeof(char), sz, fp);
        if (sz != *rsz) {
            app_warning("Error reading file '%s'", fname);
            free(rv);
            rv = NULL;
        }
    }

    fclose(fp);

    return rv;
}

