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

// Object to be stored in 2D map array. Builds off of body but has other fields
// for use with map and game.
typedef struct object{
  body_t *body;
  bool is_open;
  bool is_purchased;
  char *type;
  double *coll_extrema;
} object_t;

// Initializes off of body.
object_t *object_init(body_t *body);

// Recalculates min, max in order to keep updated for the moving objects.
void object_calc_min_max(object_t *o);

// Gets min and maxes of bounds, for use in bounding box calculations.
double *object_get_min_max(object_t *o);

// Frees things associated with object that aren't freed elsewhere.
void object_free(void *o);

#endif // #ifndef __SCENE_H__
