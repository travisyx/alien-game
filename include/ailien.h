#ifndef __AILIEN_H__
#define __AILIEN_H__

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "map.h"
#include "body.h"
#include "list.h"
#include "object.h"
#include "sorted_list.h"

typedef struct alien{
  object_t *alien;
  object_t *player;
  list_t *path;
  node_t *player_last_seen;
  bool is_chasing_player;
  bool is_moving_toward_node;
  double wait_time;
} alien_t;

/**
 * Setup and initialization. Pulls some items out of map for easy access.
 *
 * @param map the map
 * @return an alien struct.
 */
alien_t *ai_init_bounds(map_t *map);

/**
 * Given alien, frees everything associated.
 *
 * @param alien the alien
 */
void ai_free(alien_t *alien);

/**
 * Main method. Defines alien's behavior based on proximity to player, if can
 * see player, if on a path, etc. Includes the call to A*.
 *
 * @param map the map
 * @param alien alien
 * @param stalk_radius, how far 
 * @param tick (dt)
 */
void ai_stalk(map_t *map, alien_t *alien, int stalk_radius, double tick);

// AI just chases around to player's position. For testing purposes.
/**
 * Setup and initialization. Pulls some items out of map for easy access.
 *
 * @param map the map
 * @return an alien struct.
 */
void basic_follow(map_t *map, double vel);

#endif
