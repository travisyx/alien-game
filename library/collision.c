#include "collision.h"

double *get_mins_and_maxes(list_t *inp){
  // Index returned of 0 is min x, index 1 is max x, 2 is min y, 3 is max y
  double *ans = malloc(4 * sizeof(double));
  double min_x, min_y, max_x, max_y;
  min_x = min_y = INFINITY;
  max_x = max_y = -INFINITY;
  for(size_t i = 0; i < list_size(inp); i++){
    double x = ((vector_t *)list_get(inp, i))->x;
    double y = ((vector_t *)list_get(inp, i))->y;
    if(x < min_x){
      min_x = x;
    } else if (x > max_x){
      max_x = x;
    }
    if(y < min_y){
      min_y = y;
    } else if (y > max_y){
      max_y = y;
    }
  }
  ans[0] = min_x;
  ans[1] = max_x;
  ans[2] = min_y;
  ans[3] = max_y;
  return ans;
}

bool object_test_bounding_box(object_t *one, object_t *two){
  // Written to get around querying alien and player and other objects every single time...
  double *temp1 = object_get_min_max(one);
  double *temp2 = object_get_min_max(two);
  if((temp1[0] <= temp2[0] && temp2[0] <= temp1[1]) ||
    (temp2[0] <= temp1[0] && temp1[0] <= temp2[1])){
    if((temp1[2] <= temp2[2] && temp2[2] <= temp1[3]) ||
      (temp2[2] <= temp1[2] && temp1[2] <= temp2[3])){
        return true;
    }
  }
  return false;
}

bool body_test_bounding_box(list_t *body_one, list_t *body_two){
  // Tests if two objects' bounding are within each other
  double *temp1 = get_mins_and_maxes(body_one);
  double *temp2 = get_mins_and_maxes(body_two);
  if((temp1[0] <= temp2[0] && temp2[0] <= temp1[1]) ||
    (temp2[0] <= temp1[0] && temp1[0] <= temp2[1])){
    if((temp1[2] <= temp2[2] && temp2[2] <= temp1[3]) ||
      (temp2[2] <= temp1[2] && temp1[2] <= temp2[3])){
        free(temp1);
        free(temp2);
        return true;
    }
  }
  free(temp1);
  free(temp2);
  return false;
}

// Helper method to get all perpendicular vectors to both shapes for the
// separating axis method
list_t *get_perp(list_t *shape1, list_t *shape2){
  // Gets the perpendicular vectors for the separating axis method
  list_t *all_perpendicular = list_init(list_size(shape1)+list_size(shape2), vec_free);
  int counter = 0;
  for(size_t i = 0; i < list_size(shape1); i++){
    vector_t vec_one = (vector_t) {0, 0};
    if(i != 0)
      vec_one = vec_subtract(*(vector_t *) list_get(shape1, i), *(vector_t *) list_get(shape1, i-1));
    else
      vec_one = vec_subtract(*(vector_t *) list_get(shape1, 0), *(vector_t *) list_get(shape1, list_size(shape1)-1));
    vector_t *perp = malloc(sizeof(vector_t));
    *perp = (vector_t){-vec_one.y, vec_one.x};
    double distance = pow(pow(vec_one.x, 2) + pow(vec_one.y, 2), 0.5);
    if(distance != 0)
      *perp = vec_multiply(1/distance, *perp);
    list_add(all_perpendicular, perp);
    counter++;
  }
  for(size_t i = 0; i < list_size(shape2); i++){
    vector_t vec_two = (vector_t) {0, 0};
    if(i != 0)
      vec_two = vec_subtract(*(vector_t *)list_get(shape2, i),*(vector_t *)list_get(shape2, i-1));
    else
      vec_two = vec_subtract(*(vector_t *)list_get(shape2, 0),*(vector_t *)list_get(shape2, list_size(shape2)-1));
    vector_t *perp = malloc(sizeof(vector_t));
    *perp = (vector_t){-vec_two.y, vec_two.x};
    double distance = pow(pow(vec_two.x, 2) + pow(vec_two.y, 2), 0.5);
    if(distance != 0)
      *perp = vec_multiply(1/distance, *perp);
    list_add(all_perpendicular, perp);
  }
  return all_perpendicular;
}


double get_max_scale(list_t *inp, vector_t *perp){
  // Index 0 is min scale, 1 is max scale (project inp onto perp)
  double max_scale = -INFINITY;
  for(size_t j = 0; j < list_size(inp); j++){
    vector_t *vec_one = (vector_t *) list_get(inp, j);
    double scale = vec_dot(*vec_one, *perp);
    if(scale > max_scale)
      max_scale = scale;
  }
  return max_scale;
}

double get_min_scale(list_t *inp, vector_t *perp){
  double min_scale = INFINITY;
  for(size_t j = 0; j < list_size(inp); j++){
    vector_t *vec_one = (vector_t *) list_get(inp, j);
    double scale = vec_dot(*vec_one, *perp);
    if(scale < min_scale)
      min_scale = scale;
  }
  return min_scale;
}

collision_info_t find_collision(list_t *shape1, list_t *shape2){
  collision_info_t info;
  if(!body_test_bounding_box(shape1, shape2)){
    info.collided = false;
    return info;
  }
  else{
    double scale = INFINITY;
    vector_t axis = (vector_t) {0,0};
    list_t *all_perpendicular = get_perp(shape1, shape2);
    // Compare the points of each polygon to the unit vectors (get closest, farthest)
    for(size_t i = 0; i < list_size(all_perpendicular); i++){
      vector_t *side = (vector_t *) list_get(all_perpendicular, i);
      double magnitude = pow(pow(side->x, 2) + pow(side->y, 2), 0.5);

      double val1_min = get_min_scale(shape1, side);
      double val1_max = get_max_scale(shape1, side);
      double val2_min = get_min_scale(shape2, side);
      double val2_max = get_max_scale(shape2, side);

      if(!(val2_min < val1_max && val1_min < val2_max)){
        list_free(all_perpendicular);
        info.collided = false;
        return info;
      } else { // all true cases
        bool one_less_than_two = true;
        if(val2_max < val1_max){
          one_less_than_two = false;
        }
        if(one_less_than_two){
          if(magnitude * (val1_max - val2_min) < scale){
            scale = magnitude * (val1_max - val2_min);
            axis = *side;
          }
        } else
          if(magnitude * (val2_max - val1_min) < scale){
            scale = magnitude * (val2_max - val1_min);
            axis = *side;
          }
      }
    }
    list_free(all_perpendicular);
    info.collided = true;
    info.axis = axis;
  }
  return info;
}


collision_info_t object_find_collision(list_t *shape1, list_t *shape2){
  // For objects only; skips bounding box calc
  collision_info_t info;
  double scale = INFINITY;
  vector_t axis = (vector_t) {0,0};
  list_t *all_perpendicular = get_perp(shape1, shape2);
  // Compare the points of each polygon to the unit vectors (get closest, farthest)
  for(size_t i = 0; i < list_size(all_perpendicular); i++){
    vector_t *side = (vector_t *) list_get(all_perpendicular, i);
    double magnitude = pow(pow(side->x, 2) + pow(side->y, 2), 0.5);

    double val1_min = get_min_scale(shape1, side);
    double val1_max = get_max_scale(shape1, side);
    double val2_min = get_min_scale(shape2, side);
    double val2_max = get_max_scale(shape2, side);

    if(!(val2_min < val1_max && val1_min < val2_max)){
      list_free(all_perpendicular);
      info.collided = false;
      return info;
    } else { // all true cases
      bool one_less_than_two = true;
      if(val2_max < val1_max){
        one_less_than_two = false;
      }
      if(one_less_than_two){
        if(magnitude * (val1_max - val2_min) < scale){
          scale = magnitude * (val1_max - val2_min);
          axis = *side;
        }
      } else
        if(magnitude * (val2_max - val1_min) < scale){
          scale = magnitude * (val2_max - val1_min);
          axis = *side;
        }
    }
  }
  list_free(all_perpendicular);
  info.collided = true;
  info.axis = axis;
  return info;
}
