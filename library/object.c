#include "object.h"

// ONLY USE FOR 2D ARRAY IN MAP

double *get_extrema(list_t *inp){
  double *ans = malloc(4 * sizeof(double));
  double min_x_1, min_y_1, max_x_1, max_y_1;
  min_x_1 = min_y_1 = INFINITY;
  max_x_1 = max_y_1 = -INFINITY;
  for(size_t i = 0; i < list_size(inp); i++){
    double x = ((vector_t *)list_get(inp, i))->x;
    double y = ((vector_t *)list_get(inp, i))->y;
    if(x < min_x_1){
      min_x_1 = x;
    } else if (x > max_x_1){
      max_x_1 = x;
    }
    if(y < min_y_1){
      min_y_1 = y;
    } else if (y > max_y_1){
      max_y_1 = y;
    }
  }
  ans[0] = min_x_1;
  ans[1] = max_x_1;
  ans[2] = min_y_1;
  ans[3] = max_y_1;
  return ans;
}

object_t *object_init(body_t *body){
  // Body should already have info!!
  object_t *o = malloc(sizeof(object_t));
  o->body = body;
  o->is_open = false;
  o->is_purchased = false;
  o->type = (char *)body_get_info(body);
  list_t *shape = body_get_shape(body);
  o->coll_extrema = get_extrema(shape);
  list_free(shape);
  return o;
}

void object_calc_min_max(object_t *o){
  free(o->coll_extrema);
  list_t *shape = body_get_shape(o->body);
  o->coll_extrema = get_extrema(shape);
  list_free(shape);
}


double *object_get_min_max(object_t *o){
  return o->coll_extrema;
}

void object_free(void *o){
  // Assumes that there is coll_extrema...no reason why there should not be upon free, so...
  free(((object_t *)o)->coll_extrema);
  free(o);
}
