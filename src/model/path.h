#ifndef _PATH_H_
#define _PATH_H_

#include "types.h"

struct path {
    struct vec2 *steps;
};

void path_init(struct path *p);
void path_free(struct path *p);
#define path_is_free(p) (p.steps == NULL)
struct path find_path(struct world *w, struct unit *u, struct vec2 dest);

#endif /* _PATH_H_ */

