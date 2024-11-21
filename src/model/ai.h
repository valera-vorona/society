#ifndef _AI_H_
#define _AI_H_

struct ai;
struct path;

void ai_player_init(struct ai *ai);
void ai_human_init(struct ai *ai);
void ai_add_task_from_path(struct ai* ai, struct path p);

#endif /* _AI_H_ */

