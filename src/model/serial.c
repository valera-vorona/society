#include "serial.h"
#include "app.h"
#include <stdlib.h>
#include <stdio.h>

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

