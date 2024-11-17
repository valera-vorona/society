#include "types.h"

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
//puts("p");

}

/*
 * human ai
 */

void ai_human_init(struct ai *ai);
static void ai_human_step(struct ai *ai);

void
ai_human_init(struct ai *ai) {
    ai->step = ai_human_step;
}

static void
ai_human_step(struct ai *ai) {

}

