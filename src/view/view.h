#ifndef _VIEW_H_
#define _VIEW_H_

#include "types.h"

struct view;

void main_view_init(struct view *view);
void main_view_free(struct view *view);
void main_view_draw(struct view *view);
void main_view_zoom(struct view *view, int delta);
void main_view_scroll(struct view *view, struct vec2 delta);

#endif /* _VIEW_H_ */

