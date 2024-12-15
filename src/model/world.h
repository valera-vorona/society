#ifndef _WORLD_H_
#define _WORLD_H_

#include "types.h"

int world_init(struct world *w, const char *fname, struct mt_state *mt);
void world_free(struct world *w);
void world_step(struct world *w);
struct receipt *get_possible_harvests(struct world *w, struct unit *u, int resource, struct receipt *receipt);

#endif /* _WORLD_H_ */

