#ifndef _MENU_H_
#define _MENU_H_

struct view;

void main_menu_init(struct view *view);
void main_menu_free(struct view *view);
void main_menu_draw(struct view *view);

void new_menu_init(struct view *view);
void new_menu_free(struct view *view);
void new_menu_draw(struct view *view);

void options_menu_init(struct view *view);
void options_menu_free(struct view *view);
void options_menu_draw(struct view *view);

#endif /* _MENU_H_ */

