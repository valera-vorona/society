#include "types.h"
#include "path.h"
#include "ai.h"
#include "stb_ds.h"
#include <stddef.h>
#include <malloc.h>

void action_init(struct action *a) {
    a->type = A_NOTHING;
}

void task_init(struct task *t) {
    t->i = 0;
    t->actions = NULL;
}

void task_free(struct task *t) {
    arrfree(t->actions);
}

void task_append_action(struct task *t, struct action *a) {
    arrput(t->actions, *a);
}

/*
 * general unit ai
 */

void
ai_add_task_from_path(struct ai* ai, struct path p) {
    task_free(&ai->task);
    task_init(&ai->task);
    struct action a = { type: A_WALK };
    for (int i = 0, ie = arrlenu(p.steps); i != ie; ++i) {
        a.act.walk.to.x = p.steps[i].x * 64;
        a.act.walk.to.y = p.steps[i].y * 64;
        task_append_action(&ai->task, &a);
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

static int
ai_unit_step(struct ai *ai, struct action *a) {
    switch (a->type) {
    case A_NOTHING:
                    return 0;

    case A_STAY:    if (--a->act.stay.cnt <= 0) {
                        a->type = A_NOTHING;
                        return 0;
                    }
                    break;

    case A_WALK:    if (ai->unit->coords.x == a->act.walk.to.x && ai->unit->coords.y == a->act.walk.to.y) {
                        a->type = A_NOTHING;
                        return 0;
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

    return 1;
}

/*
 * player ai
 */

void ai_player_init(struct ai *ai);
static void ai_player_step(struct ai *ai);

void
ai_player_init(struct ai *ai) {
    ai->step = ai_player_step;
    task_init(&ai->task);
}

static void
ai_player_step(struct ai *ai) {
    size_t num = arrlenu(ai->task.actions);
    if (num) {
        if (ai->task.i < num) {
            if (!ai_unit_step(ai, &ai->task.actions[ ai->task.i ])) {
                ++ai->task.i;
            }
        } else {
            task_free(&ai->task);
        }
    }
}

/*
 * human ai
 */

static void ai_human_step(struct ai *ai);

void
ai_human_init(struct ai *ai) {
    ai->step = ai_human_step;
    task_init(&ai->task);
    arrsetlen(ai->task.actions, 1);
    action_init(ai->task.actions);
}

static void
ai_human_step(struct ai *ai) {
    if (!ai_unit_step(ai, ai->task.actions))
        gen_rand_action(ai, ai->task.actions);
}

