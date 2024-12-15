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

enum wiki_type {
    WT_UNKNOWN = 0,
    WT_TILE,
    WT_RESOURCE
};

enum generator_out_type {
    GT_UNKNOWN = 0,
    GT_TILE,
    GT_RESOURCE
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

struct range {
    float min, max;
};

struct characteristics {
    int strength;
    int damage;
};

/*
 * map 
 */

struct resource_t {
    int id;
    int index;
    char *name;
    char *description;
};

struct resource {
   int type;
};

struct tile_t {
    int id;
    int index;
    char *name;         /* not strduped */
    char *description;  /* not strduped */
    int is_default;
};

struct tile {
    int type;
    int tileset_index;
    int transit_index;
    float height;
    float humidity;
    int resource;
    int units[1];
};

struct map {
    struct vec2 size;
    struct tile_t *tile_types;
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
    int index;
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
 * generator
 */

struct generator {
    int func;
    struct {
        struct range height;
        struct range humidity;
        float prob;
    } in;
    struct {
        enum generator_out_type type;
        char *name;
    } out;
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
    struct resource target;
    int time;
    struct {
        struct resource resource;
    } required;
};

/*
 * wiki
 */

struct wiki_node {
    enum wiki_type type;
    union {
        struct tile_t tile;
        struct resource_t resource;
    } value;
};

struct wiki_hash {
    char *key;
    struct wiki_node value;
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
    struct wiki_hash *wiki;
    struct mt_state *mt;
    float fps;
    struct wind *winds;
    struct tileset_hash *tilesets;
    struct ai *player_ai;
    struct map map;
    struct resource_t *resource_types;
    struct unit_t *unit_types;
    struct unit *units;
    struct generator *generators;
    struct ai *ais;
    struct building *buildings;
    struct asset *assets;
    struct tool *tools;
    struct receipt *receipts;
};

#endif /* _TYPES_H_ */

