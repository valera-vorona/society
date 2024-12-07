#ifndef _TYPES_H_
#define _TYPES_H_

#include "rand.h"
#ifndef NK_SDL_RENDERER_H_
  #include "nuklear_sdl_renderer.h"
#endif
#include <stdint.h>
#include <limits.h>

#define ID_NOTHING INT_MAX

struct jq_value;
struct world;

/*
 * enums 
 */

enum resource_t {
    RT_UNKNOWN = 0,
    RT_WOOD
};

enum building_t {
    BT_UNKNOWN = 0,
    BT_HOUSE,
    BT_WINDMILL
};

enum asset_t {
    AT_UNKNOWN = 0,
    AT_WHEAT,
    AT_BREAD
};

enum tool_t {
    TLT_UNKNOWN = 0,
    TLT_SWORD,
    TLT_AXE
};

/*
 * structs
 */

/*
 * common
 */

struct vec2i {
    int x, y;
};

#define vec2 vec2i

struct recti {
    int x, y, w, h;
};

#define rect recti

struct vec2f {
    float x, y;
};

struct characteristics {
    int strength;
    int damage;
};

/*
 * map 
 */

struct resource {
   enum resource_t type;
   struct vec2 coords;
};

struct tile_t {
    int id;
    char *name;         /* not strduped */
    char *description;  /* not strduped */
    float gen_part;
    int is_water_line;     /* boolean */
};

struct cover {
    int id;
    char *name;         /* not strduped */
    char *description;  /* not strduped */
    int *types;
};

struct tile {
    int type;
    int tileset_index;
    int transit_index;
    float height;
    float humidity;
    int units[1];
};

struct map {
    struct vec2 size;
    struct tile_t *tile_types;
    struct cover *covers;
    struct tile *tiles;
};

/*
 * unit
 */

enum unit_flags {
    UF_NONE     = 0,
    UF_PLAYER
};

struct unit_t {
    int id;
    char *name;         /* not strduped */
    float *probs;
    float *pass;
};

struct innate {
    int sociality;
    int leadership;
};

struct unit {
    int type;
    enum unit_flags flags; /* enum unit_flags */
    struct vec2 coords;
    struct innate innate;
    struct characteristics characteristics;
};

/*
 * ai
 */

struct ai;
typedef void (*step)(struct ai *ai);

enum action_t {
    A_NOTHING   = 0,
    A_STAY,
    A_WALK,
    A_DO,
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

struct task {
    int i;                  /* iterator */
    struct action *actions; /* actions array */
};


struct ai {
    step step;
    struct world *world;
    void *data;
    struct unit *unit;
    struct task task;
};

/*
 * building
 */

struct building {
    enum building_t type;
    struct characteristics characteristics;
    struct vec2 coords;
};

/*
 * asset
 */

struct asset {
    enum asset_t type;
    struct characteristics characteristics;
};

/*
 * tool
 */

struct tool {
    enum tool_t type;
    struct characteristics characteristics;
};

/*
 * receipt 
 */

struct receipt {
   enum building_t target;
   struct tool *tools;
   struct asset *assets;
};

/*
 * world 
 */

/*
'jet_latitude' is where the wind starts from,
'dest_latitude' is where it comes to,
90. is North pole, 0. is Equator, -90. is South pole",
*/

struct wind {
    float jet_latitude;
    float dest_latitude;
    float persistence;
};

struct tileset {
    struct vec2 margin;
    struct vec2 padding;
    struct vec2 tile_size;
    struct vec2 tileset_size;
    struct vec2 quad_size;
    struct nk_image image;
    struct vec2 image_size;
};

struct tileset_hash {
    char *key;
    struct tileset value;
};

struct world {
    struct jq_value *json;
    struct mt_state *mt;
    float fps;
    struct wind *winds;
    struct tileset_hash *tilesets;
    struct ai *player_ai;
    struct map map;
    struct resource *recources;
    struct unit_t *unit_types;
    struct unit *units;
    struct ai *ais;
    struct building *buildings;
    struct asset *assets;
    struct tool *tools;
    struct receipt *receipts;
};

#endif /* _TYPES_H_ */

