#ifndef _PATH_H_
#define _PATH_H_

#include "types.h"

struct path {
    struct vec2 *step;
};

struct path find_path(struct world *w, struct unit *u, struct vec2 dest);

#endif /* _PATH_H_ */

