#include "world.h"
#include "app.h"
#include "serial.h"
#include "tileset.h"
#include "stb_ds.h"

static int get_vec2(struct jq_value *v, struct vec2 *out);
static int get_range(struct jq_value *v, struct range *out);

int world_init(struct world *w, const char *fname, struct mt_state *mt) {
    char buf[4096];
    struct jq_value *val;
    struct jq_value *v;

    w->mt = mt;

    w->units = NULL;
    w->player_ai = NULL;

    /* Reading world json file */
    w->json = read_json(fname);
    if (!w->json) {
        app_warning("Error in world file");
        return 1;
    }

    /* init fps */
    val = jq_find(w->json, "fps", 0);
    if (val && jq_isnumber(val)) {
        w->fps = jq_isreal(val) ? val->value.real : (float)val->value.integer;
        if (w->fps < .1 || w->fps > 1000.) {
            app_warning("'fps' should be a positive number within 0.1 and 1000.0, set to default (60.0)");
            w->fps = 60.;
        }
    } else {
        app_warning("'fps' is not found or not a number, set to default (60.0)");
        w->fps = 60.;
    }

    /* Reading tilesets */
    w->tilesets = NULL;
    val = jq_find(w->json, "tilesets", 0);
    if (val && jq_isobject(val)) {
        struct jq_pair *p;
        struct jq_value *k;
        jq_foreach_object(p, val) {
            SDL_TextureAccess access;
            struct tileset t;
            tileset_init(&t);
            v = &p->value;

            k = jq_find(v, "access", 0);
            if (k) {
                if (!jq_isstring(k)) {
                    app_warning("Image access doesn't exist or is not a string");
                    return 1;
                }

                if (!strcmp(k->value.string, "static")) {
                    access = SDL_TEXTUREACCESS_STATIC;
                } else if (!strcmp(k->value.string, "streaming")) {
                    access = SDL_TEXTUREACCESS_STREAMING;
                } else if (!strcmp(k->value.string, "target")) {
                    access = SDL_TEXTUREACCESS_TARGET;
                } else {
                    app_warning("'access' %s is unknown, it should be one of 'static', 'streaming' or 'target'", k->value.string);
                    return 1;
                }
            } else {
                /* setting default access */
                access = SDL_TEXTUREACCESS_STATIC;
            }

            k = jq_find(v, "file", 0);
            if (k && jq_isstring(k)) {
                snprintf(buf, sizeof(buf), BINARY_PATH "%s", k->value.string);
                void *tex = nk_sdl_device_upload_image(buf, access);
                if (!tex) return 1;
                t.image = nk_image_ptr(tex);
                if (get_image_size(tex, &t.image_size.x, &t.image_size.y)) {
                    return 1;
                }
            } else {
                app_warning("Image file doesn't exist or is not a string");
                return 1;
            }

            k = jq_find(v, "size", "margin", 0);
            if (k) {
                if (get_vec2(k, &t.margin))
                    return 1;
            } else {
                app_warning("'margin' doesn't exist");
                return 1;
            }

            k = jq_find(v, "size", "padding", 0);
            if (k) {
                if (get_vec2(k, &t.padding))
                    return 1;
            } else {
                app_warning("'padding' doesn't exist");
                return 1;
            }

            k = jq_find(v, "size", "tile", 0);
            if (k) {
                if (get_vec2(k, &t.tile_size))
                    return 1;
            } else {
                app_warning("'size.tile' doesn't exist");
                return 1;
            }

            k = jq_find(v, "size", "tileset", 0);
            if (k) {
                if (get_vec2(k, &t.tileset_size))
                    return 1;
            } else {
                app_warning("'size.tileset' doesn't exist");
                return 1;
            }

            k = jq_find(v, "size", "quad", 0);
            if (k) {
                if (get_vec2(k, &t.quad_size))
                    return 1;
            } else {
                /* setting default quad */
                t.quad_size.x = 1;
                t.quad_size.y = 1;
            }

            shput(w->tilesets, p->key, t);
        }
    }

    /* Reading winds */
    w->winds = NULL;
    val = jq_find(w->json, "winds", 0);
    if (val) {
        if (jq_isarray(val)) {
            jq_foreach_array(v, val) {
                if (jq_isobject(v)) {
                    struct wind wind;
                    struct jq_value *p = jq_find(v, "jet_latitude", 0);
                    if (p && jq_isnumber(p)) {
                        wind.jet_latitude = p->type == JQ_V_INTEGER ? p->value.integer : p->value.real;
                    } else {
                        app_warning("'jet_latitude' is not found or not a number");
                        return 1;
                    }

                    p = jq_find(v, "dest_latitude", 0);
                    if (p && jq_isnumber(p)) {
                        wind.dest_latitude = p->type == JQ_V_INTEGER ? p->value.integer : p->value.real;
                    } else {
                        app_warning("'dest_latitude' is not found or not a number");
                        return 1;
                    }

                    p = jq_find(v, "persistence", 0);
                    if (p && jq_isnumber(p)) {
                        wind.persistence = p->type == JQ_V_INTEGER ? p->value.integer : p->value.real;
                    } else {
                        app_warning("'persistence' is not found or not a number");
                        return 1;
                    }

                    arrput(w->winds, wind);
                } else {
                    app_warning("'wind' is not an object");
                    return 1;
                }
            }
        } else {
            app_warning("'winds' is not an array");
            return 1;
        }
    }

    /* Reading tile types */
    w->map.tile_types = NULL;
    val = jq_find(w->json, "tiles", 0);
    if (val && jq_isarray(val)) {
        int default_found = 0;
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {
                struct tile_t t;
                struct jq_value *p = jq_find(v, "id", 0);
                if (p && jq_isinteger(p)) {
                    t.id = p->value.integer;
                } else {
                    app_warning("'id' of tile is not found or not an integer");
                    return 1;
                }

                p = jq_find(v, "name", 0);
                if (p && jq_isstring(p)) {
                    t.name = p->value.string;
                } else {
                    app_warning("'name' of tile is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "description", 0);
                if (p && jq_isstring(p)) {
                    t.description = p->value.string;
                } else {
                    t.description = "";
                }

                p = jq_find(v, "default", 0);
                if (p) {
                    if (jq_isboolean(p)) {
                        t.is_default = p->type == JQ_V_TRUE;
                        if (t.is_default)
                            ++default_found;
                    } else {
                        app_warning("'default' is not boolean");
                        return 1;
                    }
                } else {
                    /* setting default is_default */
                    t.is_default = 0;
                }

                arrput(w->map.tile_types, t);
            } else {
                app_warning("'tile' is not an object");
                return 1;
            }
        }

        if (default_found != 1) {
            app_warning("Only one of 'tiles' should have key 'default': true");
            return 1;
        }
    } else {
        app_warning("'tiles' doesn't exist or is not an array");
        return 1;
    }

    /* Reading resource types */
    w->resource_types = NULL;
    val = jq_find(w->json, "resources", 0);
    if (val && jq_isarray(val)) {
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {
                struct resource_t r;
                struct jq_value *p = jq_find(v, "id", 0);
                if (p && jq_isinteger(p)) {
                    r.id = p->value.integer;
                } else {
                    app_warning("'id' of resource is not found or not an integer");
                    return 1;
                }

                p = jq_find(v, "name", 0);
                if (p && jq_isstring(p)) {
                    r.name = p->value.string;
                } else {
                    app_warning("'name' of resource is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "description", 0);
                if (p && jq_isstring(p)) {
                    r.description = p->value.string;
                } else {
                    r.description = "";
                }

                arrput(w->resource_types, r);
            } else {
                app_warning("'resource' is not an object");
                return 1;
            }
        }
    } else {

        app_warning("'resources' doesn't exist or is not an array");
        return 1;
    }

    /* Reading covers */
    w->map.covers = NULL;
    val = jq_find(w->json, "covers", 0);
    if (val) {
        if (jq_isarray(val)) {
            jq_foreach_array(v, val) {
                if (jq_isobject(v)) {
                    struct cover c;
                    struct jq_value *p = jq_find(v, "type", 0);
                    if (p && jq_isstring(p)) {
                        int type = -1;
                        for (int i = 0, ie = arrlenu(w->map.tile_types); i != ie; ++i) {
                            if (!strcmp(p->value.string, w->map.tile_types[i].name)) {
                                type = w->map.tile_types[i].id;
                                break;
                            }
                        }
                        if (type == -1) {
                            app_warning("No tile type found with name '%s'", p->value.string);
                            return 1;
                        }
                        c.type = type;
                    } else {
                        app_warning("'type' of cover is not found or not a string");
                        return 1;
                    }

                    p = jq_find(v, "height", 0);
                    if (p) {
                        if (get_range(p, &c.height))
                            return 1;
                    } else {
                        app_warning("'height' of cover doest't exist");
                        return 1;
                    }

                    p = jq_find(v, "humidity", 0);
                    if (p) {
                        if (get_range(p, &c.humidity))
                            return 1;
                    } else {
                        app_warning("'humidity' of cover doest't exist");
                        return 1;
                    }

                    arrput(w->map.covers, c);
                } else {
                    app_warning("'cover' is not an object");
                    return 1;
                }
            }
        } else {
            app_warning("'covers' is not an array");
            return 1;
        }
    }

    /* Reading unit types */
    w->unit_types = NULL;
    val = jq_find(w->json, "units", 0);
    if (val && jq_isarray(val)) {
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {
                struct unit_t t;
                t.probs = NULL;
                t.pass = NULL;
                arrsetlen(t.probs, arrlenu(w->map.tile_types));
                arrsetlen(t.pass, arrlenu(w->map.tile_types));
                for (int i = 0, ie = arrlenu(t.probs); i != ie; ++i) {
                    t.probs[i] = .0;
                    t.pass[i] = .0;
                }

                struct jq_value *p = jq_find(v, "id", 0);
                if (p && jq_isinteger(p)) {
                    t.id = p->value.integer;
                } else {
                    app_warning("'id' of unit is not found or not an integer");
                    return 1;
                }

                p = jq_find(v, "name", 0);
                if (p && jq_isstring(p)) {
                    t.name = p->value.string;
                } else {
                    app_warning("'name' of unit is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "probs", 0);
                if (p && jq_isobject(p)) {
                    struct jq_pair *a;
                    jq_foreach_object(a, p) {
                        if (jq_isreal(&a->value)) {
                            int found = 0;
                            for (int i = 0, ie = arrlenu(w->map.tile_types); i != ie; ++i) {
                                if (!strcmp(a->key, w->map.tile_types[i].name)) {
                                    t.probs[i] = a->value.value.real;
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found) {
                                app_warning("No tile type found with name '%s'", a->key);
                                return 1;
                            }
                        } else {
                            app_warning("values of 'unit.probs' object should be pairs of keys and floats");
                            return 1;
                        }
                    }
                } else {
                    app_warning("'probs' of unit is not found or not an object");
                    return 1;
                }

                p = jq_find(v, "pass", 0);
                if (p && jq_isobject(p)) {
                    struct jq_pair *a;
                    jq_foreach_object(a, p) {
                        if (jq_isreal(&a->value)) {
                            int found = 0;
                            for (int i = 0, ie = arrlenu(w->map.tile_types); i != ie; ++i) {
                                if (!strcmp(a->key, w->map.tile_types[i].name)) {
                                    t.pass[i] = a->value.value.real;
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found) {
                                app_warning("No tile type found with name '%s'", a->key);
                                return 1;
                            }
                        } else {
                            app_warning("values of 'unit.pass' object should be pairs of keys and floats");
                            return 1;
                        }
                    }
                } else {
                    app_warning("'pass' of unit is not found or not an object");
                    return 1;
                }

                arrput(w->unit_types, t);
            } else {
                app_warning("'unit' is not an object");
                return 1;
            }
        }
    } else {
        app_warning("'units' doesn't exist or is not an array");
        return 1;
    }

    /* Reading generators */
    w->generators = NULL;
    val = jq_find(w->json, "generators", 0);
    if (val && jq_isarray(val)) {
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {
                struct generator g;
                struct jq_value *p = jq_find(v, "func", 0);
                if (p && jq_isstring(p)) {
                    if (!strcmp(p->value.string, "standard")) {
                        g.out.type = 1;
                    } else {
                        app_warning("No generator func found with name '%s', it should be 'standard'", p->value.string);
                        return 1;
                    }
                } else {
                    app_warning("'func' of generator is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "in", "height", 0);
                if (p) {
                    if (get_range(p, &g.in.height))
                        return 1;
                } else {
                    /* setting default height */
                    g.in.height.min = .0;
                    g.in.height.max = 1.;
                }

                p = jq_find(v, "in", "humidity", 0);
                if (p) {
                    if (get_range(p, &g.in.humidity))
                        return 1;
                } else {
                    /* setting default humidity */
                    g.in.humidity.min = .0;
                    g.in.humidity.max = 1.;
                }

                p = jq_find(v, "in", "prob", 0);
                if (p && jq_isreal(p)) {
                    g.in.prob = p->value.real;
                } else {
                    /* setting default prob */
                    g.in.prob = 1.;
                }

                p = jq_find(v, "out", "type", 0);
                if (p && jq_isstring(p)) {
                    if (!strcmp(p->value.string, "tile")) {
                        g.out.type = GT_TILE;
                    } else if (!strcmp(p->value.string, "resource")) {
                        g.out.type = GT_RESOURCE;
                    } else {
                        app_warning("No tile type found with name '%s', it should be 'tile' or 'resource'", p->value.string);
                        return 1;
                    }
                } else {
                    app_warning("'out.type' is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "out", "name", 0);
                if (p && jq_isstring(p)) {
                    switch (g.out.type) {
                    case GT_TILE:
                        g.out.name = NULL;
                        for (int i = 0, ie = arrlenu(w->map.tile_types); i != ie; ++i) {
                            if (!strcmp(p->value.string, w->map.tile_types[i].name)) {
                                g.out.name = p->value.string;
                                break;
                            }
                        }
                        if (!g.out.name) {
                            app_warning("No tile type found with name '%s'", p->value.string);
                            return 1;
                        }
                        break;

                    case GT_RESOURCE:
                        g.out.name = NULL;
                        for (int i = 0, ie = arrlenu(w->resource_types); i != ie; ++i) {
                            if (!strcmp(p->value.string, w->resource_types[i].name)) {
                                g.out.name = p->value.string;
                                break;
                            }
                        }
                        if (!g.out.name) {
                            app_warning("No resource type found with name '%s'", p->value.string);
                            return 1;
                        }
                        break;

                    default:
                        app_warning("Unknown tile type");
                        return 1;
                    }
                } else {
                    app_warning("'out.name' is not found or not a string");
                    return 1;
                }

                arrput(w->generators, g);
            } else {
                app_warning("'generator' is not an object");
                return 1;
            }
        }
    } else {
        app_warning("'generators' is not found or not an array");
        return 1;
    }

    /* Reading unit types */
    return 0;
}

void world_free(struct world *w) {

}

void world_step(struct world *w) {
    for (int i = 0, ie = arrlenu(w->ais); i != ie; ++i) {
        w->ais[i].step(&w->ais[i]);
    }
}

static int
get_vec2(struct jq_value *v, struct vec2 *out) {
    struct vec2 rv;
    struct jq_value *k;

    if (jq_isarray(v) && jq_array_length(v) == 2) {
        k = jq_find(v, "0", 0);
        if (k && jq_isinteger(k)) {
            rv.x = k->value.integer;
        } else {
            app_warning("'size.x' is not an integer");
            return 1;
        }

        k = jq_find(v, "1", 0);
        if (k && jq_isinteger(k)) {
            rv.y = k->value.integer;
        } else {
            app_warning("'size.y' is not an integer");
            return 1;
        }
    } else if (jq_isobject(v)) {
        k = jq_find(v, "x");
        if (k && jq_isinteger(k)) {
            rv.x = k->value.integer;
        } else {
            app_warning("'size.x' is not an integer");
            return 1;
        }

        k = jq_find(v, "y");
        if (k && jq_isinteger(k)) {
            rv.y = k->value.integer;
        } else {
            app_warning("'size.y' is not an integer");
            return 1;
        }
    } else {
        app_warning("'size' should be either an array of two elements ('x' and 'y') or an object of two keys ('x' and 'y')");
        return 1;
    }

    *out = rv;
    return 0;
}

static int
get_range(struct jq_value *v, struct range *out) {
    struct range rv;
    struct jq_value *k;

    if (jq_isarray(v) && jq_array_length(v) == 2) {
        k = jq_find(v, "0", 0);
        if (k && jq_isnumber(k)) {
            rv.min = jq_isreal(k) ? k->value.real : (float)k->value.integer;
        } else {
            app_warning("'range[0]' is not a number");
            return 1;
        }

        k = jq_find(v, "1", 0);
        if (k && jq_isnumber(k)) {
            rv.max = jq_isreal(k) ? k->value.real : (float)k->value.integer;
        } else {
            app_warning("'range[1]' is not a number");
            return 1;
        }
    } else if (jq_isobject(v)) {
        k = jq_find(v, "min");
        if (k && jq_isnumber(k)) {
            rv.min = jq_isreal(k) ? k->value.real : (float)k->value.integer;
        } else {
            app_warning("'range.min' is not a number");
            return 1;
        }

        k = jq_find(v, "max");
        if (k && jq_isnumber(k)) {
            rv.max = jq_isreal(k) ? k->value.real : (float)k->value.integer;
        } else {
            app_warning("'range.max' is not a number");
            return 1;
        }
    } else {
        app_warning("'range' should be either an array of two elements or an object of two keys ('min' and 'max')");
        return 1;
    }

    *out = rv;
    return 0;
}

