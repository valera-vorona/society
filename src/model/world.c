#include "world.h"
#include "app.h"
#include "serial.h"
#include "stb_ds.h"

int world_init(struct world *w, const char *fname, struct mt_state *mt) {
    struct jq_value *val;
    struct jq_value *v;

    w->mt = mt;

//    w->units = NULL;

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

    /* Reading tiles */
    val = jq_find(w->json, "tiles", 0);
    w->gen_parts = NULL;
    if (val && jq_isarray(val)) {
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {
                struct jq_value *p = jq_find(v, "gen-part", 0);
                if (jq_isnumber(p)) {
                    arrput(w->gen_parts, p->type == JQ_V_INTEGER ? p->value.integer : p->value.real);
                } else {
                    app_warning("'gen-part' is not a number");
                    return 1;
                }
            } else {
                app_warning("'tile' is not an object");
                return 1;
            }
        }
    } else {
        app_warning("'tiles' doesn't exist or is not an array");
        return 1;
    }

    /* Reading units */
    val = jq_find(w->json, "units", 0);
    if (val && jq_isarray(val)) {
        jq_foreach_array(v, val) {
            if (jq_isobject(v)) {

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

