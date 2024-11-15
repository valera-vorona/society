#ifndef _TYPES_H_
#define _TYPES_H_

#include "rand.h"
#include <stdint.h>

struct jq_value;

/*
 * enums 
 */
/*
enum tile_t {
    TT_UNKNOWN = 0,
    TT_OCEAN,
    TT_SEA,
    TT_PLAIN,
    TT_MAX
};
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
};

struct tile {
    int type;
};

struct map {
    struct vec2 size;
    struct tile_t *tile_types;
    struct tile *tiles;
};

/*
 * unit
 */

struct innate {
    int sociality;
    int leadership;
};

struct unit {
    struct vec2 coords;
    struct innate innate;
    struct characteristics characteristics;
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

struct world {
    struct jq_value *json;
    struct mt_state *mt;
    float fps;
    struct map map;
    struct resource *recources;
    struct unit *units;
    struct building *buildings;
    struct asset *assets;
    struct tool *tools;
    struct receipt *receipts;
};

#endif /* _TYPES_H_ */

