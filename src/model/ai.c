#include "types.h"
#include <stddef.h>
#include <malloc.h>

/*
 * actions
 */

enum action_t {
    A_NOTHING   = 0,
    A_STAY,
    A_WALK,
    A_MAX
};

struct walk {
    struct vec2 to;
};

struct stay {
    int cnt;
};

struct action {
    enum action_t type;
    union {
        struct stay stay;
        struct walk walk;
    } act;
};

void action_init(struct action *a) {
    a->type = A_NOTHING;
}

/*
 * task
 */

struct task {
    int i;                  /* iterator */
    struct action *actions; /* actions array */
};

void task_init(struct task *t) {
    t->i = 0;
    t->actions = NULL;
}

/*
 * player ai
 */

void ai_player_init(struct ai *ai);
static void ai_player_step(struct ai *ai);

void
ai_player_init(struct ai *ai) {
    ai->step = ai_player_step;
}

static void
ai_player_step(struct ai *ai) {

}

/*
 * human ai
 */

struct human_ai {
    struct action action;
};

void ai_human_init(struct ai *ai);
static void ai_human_step(struct ai *ai);

void
ai_human_init(struct ai *ai) {
    ai->step = ai_human_step;
    ai->data = malloc(sizeof(struct human_ai));
    struct human_ai *human = (struct human_ai *)ai->data;
    action_init(&human->action);

}

static void
gen_rand_action(struct ai *ai, struct action *a) {
    a->type = mt_random_uint32(ai->world->mt) % (A_MAX - 1) + 1;
    switch (a->type) {
    case A_STAY:    a->act.stay.cnt = mt_random_uint32(ai->world->mt) % 60;
                    break;

    case A_WALK:    a->act.walk.to.x = trim(0, ai->world->map.size.x * 64 - 1, ai->unit->coords.x + ((mt_random_uint32(ai->world->mt) % 3) - 1) * 64);
                    a->act.walk.to.y = trim(0, ai->world->map.size.y * 64 - 1, ai->unit->coords.y + ((mt_random_uint32(ai->world->mt) % 3) - 1) * 64);
                    break;
    }
}

static void
unit_move(struct world *w, struct unit *u, struct vec2 dir) {
    size_t old_offset = w->map.size.x * (u->coords.y / 64) + (u->coords.x / 64);
    size_t new_offset = w->map.size.x * ((u->coords.y + dir.y) / 64) + ((u->coords.x + dir.x) / 64);

    if (old_offset != new_offset) {
        w->map.tiles[old_offset].units[0] = ID_NOTHING;
        w->map.tiles[new_offset].units[0] = u - w->units;
    }

    u->coords.x += dir.x;
    u->coords.y += dir.y;
}

static void
ai_human_step(struct ai *ai) {;
    struct human_ai *human_ai = (struct human_ai *)ai->data;
    struct action *a = &human_ai->action;
    switch (a->type) {
    case A_NOTHING: gen_rand_action(ai, a);
                    break;

    case A_STAY:    if (--a->act.stay.cnt <= 0)
                        a->type = A_NOTHING;
                    break;

    case A_WALK:    if (ai->unit->coords.x == a->act.walk.to.x && ai->unit->coords.y == a->act.walk.to.y) {
                        a->type = A_NOTHING;
                    } else {
                        struct vec2 dir = { 0, 0 };
                        if (ai->unit->coords.x < a->act.walk.to.x) dir.x = 1;
                        else if (ai->unit->coords.x > a->act.walk.to.x) dir.x = -1;

                        if (ai->unit->coords.y < a->act.walk.to.y) dir.y = 1;
                        else if (ai->unit->coords.y > a->act.walk.to.y) dir.y = -1;

                        unit_move(ai->world, ai->unit, dir);
                    }
                    break;
    }
}

