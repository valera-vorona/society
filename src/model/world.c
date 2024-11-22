#include "world.h"
#include "app.h"
#include "serial.h"
#include "stb_ds.h"

int world_init(struct world *w, const char *fname, struct mt_state *mt) {
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

    /* Reading tile types */
    w->map.tile_types = NULL;
    val = jq_find(w->json, "tiles", 0);
    if (val && jq_isarray(val)) {
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
                    app_warning("'description' of tile is not found or not a string");
                    return 1;
                }

                p = jq_find(v, "gen-part", 0);
                if (jq_isnumber(p)) {
                    t.gen_part = p->type == JQ_V_INTEGER ? p->value.integer : p->value.real;
                } else {
                    app_warning("'gen-part' is not a number");
                    return 1;
                }

                arrput(w->map.tile_types, t);
            } else {
                app_warning("'tile' is not an object");
                return 1;
            }
        }
    } else {
        app_warning("'tiles' doesn't exist or is not an array");
        return 1;
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

    return 0;
}

void world_free(struct world *w) {

}

void world_step(struct world *w) {
    for (int i = 0, ie = arrlenu(w->ais); i != ie; ++i) {
        w->ais[i].step(&w->ais[i]);
    }
}

