#ifndef _SERIAL_H_
#define _SERIAL_H_

#define JQ_WITH_DOM
#include "jquick.h"

struct nk_image_hash;

struct jq_value *read_json(const char *fname);
struct nk_image_hash *read_images(struct jq_value *json);

#endif /* _SERIAL_H_ */

