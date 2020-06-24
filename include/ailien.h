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

alien_t *ai_init_bounds(map_t *map);

void ai_free(alien_t *alien);

double get_angle(vector_t one, vector_t two);

list_t *get_nodes(map_t *map, object_t *player, int stalk_radius);

bool ai_can_see_player(map_t *map, alien_t *alien, int stalk_radius);

// moves alien toward given dest at given speed
void direct_alien(body_t *alien, vector_t dest, double velocity);

void ai_stalk(map_t *map, alien_t *alien, int stalk_radius, double tick);

// map is 100x100, each box is worth 10x10
void ai_create_next_path(map_t *map, alien_t *alien, int stalk_radius);

// AI just chases around to player's position
void basic_follow(map_t *map, double vel);

#endif
