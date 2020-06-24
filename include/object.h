#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "body.h"
// #include "collision.h"
#include "list.h"
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// object to be stored in 2D map array. only suitable for STATIONARY objects
// scratch that. just use for all bodies in the map and ignore values for player/alien
// declared in header so should be able to be accessed externally.
typedef struct object{
  body_t *body;
  bool is_open;
  bool is_purchased;
  char *type;
  double *coll_extrema;
} object_t;

// could pass in char *type
object_t *object_init(body_t *body);

void object_calc_min_max(object_t *o);

double *object_get_min_max(object_t *o);

// should not free the body bc scene will try to free that. or type if same as body info
void object_free(void *o);

#endif // #ifndef __SCENE_H__
