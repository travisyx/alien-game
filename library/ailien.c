#include "ailien.h"

const int VISION_RADIUS = 75;
// const double LOOK_TIME = 5.00;
const double LOOK_TIME = 1.00;
const double VEL_STALK = 50;
// const double VEL_STALK = 400;
const double VEL_CHASE = 75;
const int MAX_PATH = 3;
const int MAX_NODES = 10000;

// Computes the diagonal distance heuristic between two nodes based on their centroids.
double diagonal_distance(node_t *start, node_t *end){
  vector_t pos_start = body_get_centroid(start->node->body);
  vector_t pos_end = body_get_centroid(end->node->body);
  double min = fabs(pos_start.x - pos_end.x) < fabs(pos_start.y - pos_end.y) ?
    fabs(pos_start.x - pos_end.x) : fabs(pos_start.y - pos_end.y);
  double max = min == fabs(pos_start.y - pos_end.y) ?
    fabs(pos_start.x - pos_end.x) : fabs(pos_start.y - pos_end.y);
  return min * pow(2, 0.5) + max-min;
}

// A*. Uses priority queue defined by sorted_list. Reverses list ordering at end.
list_t *ai_star(map_t *map, node_t *start, node_t *end){
  // Stores which nodes have been added into the queue so far.
  bool open_arr[100][100];
  // g represents distance from start to current node
  double g_vals[100][100];
  // Equivalent to f, the heuristic combining dist from start and dist to end
  double priorities[100][100];
  // Stores parents; reinit every time so don't have to clear a field in node
  node_t *parents[100][100];
  for(int i = 0; i < 100; i++){
    for(int j = 0; j < 100; j++){
      open_arr[i][j] = false;
      g_vals[i][j] = INFINITY;
      priorities[i][j] = INFINITY;
      parents[i][j] = NULL;
    }
  }
  // Adds first node to the queue and init its g
  slist_t *open = sl_init(MAX_NODES, NULL);
  sl_enqueue(open, start);
  vector_t start_ind = map_ind_from_pos(map, body_get_centroid(start->node->body));
  priorities[(int)start_ind.x][(int)start_ind.y] = diagonal_distance(start, end);
  open_arr[(int)start_ind.x][(int)start_ind.y] = true;
  g_vals[(int)start_ind.x][(int)start_ind.y] = 0;
  bool done = false;
  // Main loop. run until no more nodes in the queue and the end has been found
  while(sl_size(open) > 0 && !done){
    // Get next node (lowest priority/distance) and calc coord in backing arr
    node_t *curr = (node_t *) sl_dequeue(open);
    vector_t inds = map_ind_from_pos(map, body_get_centroid(curr->node->body));
    open_arr[(int)inds.x][(int)inds.y] = false;
    double old_dist = priorities[(int)inds.x][(int)inds.y];
    // If we have end, exit
    if(node_compare(curr, end)){
      done = true;
      break;
    }
    // Check neighbors of current node
    for(size_t i = 0; i < curr->num_neighbors; i++){
      node_t *node = curr->neighbors[i];
      if(node == NULL)
        break;
      vector_t pos = map_ind_from_pos(map, body_get_centroid(node->node->body));
      double g = old_dist + curr->distances[i];
      double f = g + diagonal_distance(node, end);
      // If shorter dist traveled from start, add to queue/replace priority
      if(g < g_vals[(int)pos.x][(int)pos.y]){
        parents[(int)pos.x][(int)pos.y] = curr;
        g_vals[(int)pos.x][(int)pos.y] = g;
        priorities[(int)pos.x][(int)pos.y] = f;
        if(!open_arr[(int)pos.x][(int)pos.y]){
          sl_enqueue(open, node);
          open_arr[(int)pos.x][(int)pos.y] = true;
        } else {
          sl_change_priority(open, node, f);
        }
      }
    }
  }
  list_t *path = list_init(100, NULL);
  node_t *temp = end;
  assert(temp != NULL);
  assert(start != NULL);
  // Reconstruct path from parents; basically a linked list
  while(temp != NULL){
    list_add(path, temp);
    vector_t pos = map_ind_from_pos(map, body_get_centroid(temp->node->body));
    temp = parents[(int)pos.x][(int)pos.y];
  }
  // Reverse path to adjust for fact that parent reconstruction is end->start
  list_t *adj_path = list_init(100, NULL);
  for(size_t i = list_size(path); i > 0; i--){
    node_t *elem = (node_t *)list_get(path, i-1);
    list_add(adj_path, elem);
  }
  list_free(path);
  sl_free(open);
  return adj_path;
}

alien_t *ai_init_bounds(map_t *map){
  alien_t *a = malloc(sizeof(alien_t));
  a->alien = map->alien;
  a->player = map->player;
  a->is_chasing_player = false;
  a->is_moving_toward_node = false;
  a->wait_time = 0;
  a->player_last_seen = NULL;
  a->path = list_init(30, NULL);
  return a;
}

void ai_free(alien_t *alien){
  list_free(alien->path);
  free(alien);
}

// Returns the angle of one relative to two.
double get_angle(vector_t one, vector_t two){
  vector_t other = vec_subtract(one, two);
  double magnitude = pow(pow(other.x, 2) + pow(other.y, 2), 0.5);
  return acos(vec_dot(other, (vector_t){1, 0})/magnitude);
}


// Helper method to extract nodes from map backing array of nodes.
list_t *get_nodes(map_t *map, object_t *player, int stalk_radius){
  vector_t centroid = body_get_centroid(player->body);
  list_t *ans = list_init(stalk_radius * stalk_radius, NULL);
  vector_t arr_ind = map_ind_from_pos(map, centroid);
  list_t *nodets = map->struct_nodes;
  for(int i = arr_ind.x - stalk_radius; i < arr_ind.x + stalk_radius; i++){
    for(int j = arr_ind.y - stalk_radius; j < arr_ind.y + stalk_radius; j++){
      // Buffer of 1 to avoid walls
      object_t *o;
      if(i > 0 && i < 100 -1 && j > 0 && j < 100 -1){
        o = (object_t *)arr_get(map->backing_array, i, j);
      } else
        continue;
      if(strcmp(o->type, "wall") != 0){
        if(i > 0 && i < 100 -1 && j > 0 && j < 100 -1){
            node_t *target = (node_t *)arr_get(nodets, i, j);
            vector_t n_cent = body_get_centroid(target->node->body);
            double dist = vec_distance(n_cent, body_get_centroid(player->body));
            if(dist < stalk_radius*100){
              list_add(ans, arr_get(nodets, i, j));
            }
          }
      }
    }
  }
  return ans;
}

// Helper method to determine if player is withing alien's vision radius AND
// line of vision is not blocked. Can be blocked by walls and hiding spots.
bool ai_can_see_player(map_t *map, alien_t *alien, int stalk_radius){
  if(is_hiding(map) || vec_distance(body_get_centroid(alien->alien->body),
    body_get_centroid(alien->player->body)) > VISION_RADIUS){
      return false;
  } else{
    // Maybe change to only see in one direction?
    list_t *nodes = get_nodes(map, alien->alien, VISION_RADIUS);
    double angle = get_angle(body_get_centroid(alien->player->body),
      body_get_centroid(alien->alien->body));
    // Hiding spots
    for(size_t i = 0; i < list_size(nodes); i++){
      node_t *node = (node_t *) list_get(nodes, i);
      if(strcmp(node->node->type, "dumpster") == 0 || strcmp(node->node->type, "locker") == 0){
        double min_angle = INFINITY;
        double max_angle = -INFINITY;
        list_t *points = body_get_shape(node->node->body);
        for(size_t i = 0; i < list_size(points); i++){
          vector_t *point = (vector_t *) list_get(points, i);
          double ang = get_angle(*point, body_get_centroid(alien->alien->body));
          if(ang < min_angle)
            min_angle = ang;
          if(ang > max_angle)
            max_angle = ang;
        }
        if(min_angle < angle && angle < max_angle){
          if(vec_distance(body_get_centroid(alien->alien->body), body_get_centroid(alien->player->body))
            >= vec_distance(body_get_centroid(alien->alien->body), body_get_centroid(node->node->body))){
              list_free(points);
              list_free(nodes);
              return false;
            }
        }
        list_free(points);
      }
    }
    list_free(nodes);
    // Walls
    angle = get_angle(body_get_centroid(alien->player->body),
      body_get_centroid(alien->alien->body));
    list_t *walls = map->walls;
    for(size_t i = 0; i < list_size(walls); i++){
      object_t *wall = (object_t *) list_get(walls, i);
      double min_angle = INFINITY;
      double max_angle = -INFINITY;
      list_t *points = body_get_shape(wall->body);
      for(size_t i = 0; i < list_size(points); i++){
        vector_t *point = (vector_t *) list_get(points, i);
        double ang = get_angle(*point, body_get_centroid(alien->alien->body));
        if(ang < min_angle)
          min_angle = ang;
        if(ang > max_angle)
          max_angle = ang;
      }
      if(min_angle < angle && angle < max_angle){
        if(vec_distance(body_get_centroid(alien->alien->body), body_get_centroid(alien->player->body))
          >= vec_distance(body_get_centroid(alien->alien->body), body_get_centroid(wall->body))){
            list_free(points);
            return false;
          }
      }
      list_free(points);
    }
    return true;
  }
  return false;
}

// Helper method that moves alien toward a specific destination at input speed.
void direct_alien(body_t *alien, vector_t dest, double velocity){
  vector_t dir = vec_subtract(dest, body_get_centroid(alien));
  dir = vec_multiply(1/vec_distance(body_get_centroid(alien), dest), dir);
  dir = vec_multiply(velocity, dir);
  body_set_velocity(alien, dir);
}

// Shuffles a list. To make alien's stalking more realistic.
list_t *shuffle(list_t *arr){
  srand(time(0));
  for(size_t i = 0; i < list_size(arr); ++i){
    int ind = (int)(rand() % (list_size(arr)-i))+i;
    node_t *new = (node_t *) list_get(arr, ind);
    node_t *old = list_replace(arr, i, new);
    list_replace(arr, ind, old);
  }
  return arr;
}

// Helper creates next path for the alien from the nodes in the given radius.
// Calls A* multiple times to create a path between several chosen destination nodes.
void ai_create_next_path(map_t *map, alien_t *alien, int stalk_radius){
  object_t *player = alien->player;
  list_t *temp_path = list_init(MAX_PATH, NULL);
  list_t *path = alien->path;
  list_t *path_extension = get_nodes(map, player, stalk_radius);
  path_extension = shuffle(path_extension);
  for(size_t i = 0; i < (list_size(path_extension)<MAX_PATH ? list_size(path_extension) : MAX_PATH); i++){
    node_t *path_elem = (node_t *) list_get(path_extension, i);
    list_add(temp_path, path_elem);
  }
  // First connect last node of old path to first node of new path here
  node_t *first = NULL;
  if(list_size(path) == 0){
    vector_t ind = map_ind_from_pos(map, body_get_centroid(alien->alien->body));
    first = (node_t *)arr_get(map->struct_nodes, ind.x, ind.y);
  }
  else if(list_size(path) == 1){
    first = (node_t *)list_get(path, 0);
  }
  list_t *vals = ai_star(map, first, (node_t *)list_get(temp_path, 0));
  for(size_t j = 0; j < list_size(vals); j++){
    node_t *next = (node_t *) list_get(vals, j);
    list_add(path, next);
  }
  list_free(vals);
  // Connect rest of nodes
  for(size_t i = 1; i < list_size(temp_path)-1; i++){
    node_t *one = (node_t *)list_get(temp_path, i - 1);
    node_t *two = (node_t *)list_get(temp_path, i);
    list_t *vals = ai_star(map, one, two);
    for(size_t j = 0; j < list_size(vals); j++){
      node_t *next = (node_t *) list_get(vals, j);
      list_add(path, next);
    }
    list_free(vals);
  }
  list_free(temp_path);
  list_free(path_extension);
}

void ai_stalk(map_t *map, alien_t *alien, int stalk_radius, double tick){
  if(ai_can_see_player(map, alien, stalk_radius)){
    // Actively chasing the player
    basic_follow(map, VEL_CHASE);
    vector_t p_cent = body_get_centroid(map->player->body);
    vector_t inds = map_ind_from_pos(map, p_cent);
    node_t *last = (node_t *)arr_get(map->struct_nodes, inds.x, inds.y);
    alien->is_chasing_player = true;
    alien->player_last_seen = last;
    alien->is_moving_toward_node = true;
    return;
  }
  else if(alien->is_chasing_player){
    // If was chasing but lost sight, go to last seen position
    list_clear(alien->path);
    vector_t a_cent = body_get_centroid(map->alien->body);
    vector_t inds = map_ind_from_pos(map, a_cent);
    node_t *last = (node_t *)arr_get(map->struct_nodes, inds.x, inds.y);
    list_t *vals = ai_star(map, last, alien->player_last_seen);
    for(size_t j = 0; j < list_size(vals); j++){
      node_t *next = (node_t *) list_get(vals, j);
      list_add(alien->path, next);
    }
    list_free(vals);
    alien->is_chasing_player = false;
    alien->is_moving_toward_node = true;
    return;
  }
  else{
    // Normal stalking action
    alien->player_last_seen = NULL;
    alien->is_chasing_player = false;
    // Realistically only at the start of the game
    if(list_size(alien->path) == 0){
      ai_create_next_path(map, alien, stalk_radius);
      return;
    }
    else if(list_size(alien->path) == 1){
      ai_create_next_path(map, alien, stalk_radius);
      return;
    }
    if(alien->is_moving_toward_node){
        // Make sure alien knows when at node and move on to next one in path
        vector_t n_pos = body_get_centroid(((node_t *)list_get(alien->path, 0))->node->body);
        direct_alien(alien->alien->body, n_pos, VEL_STALK);
        vector_t a_pos = body_get_centroid(alien->alien->body);
        if(vec_isclose(5, a_pos, n_pos)){
          list_remove(alien->path, 0);
        }
      }
      else{
        // Buffers to "look around" when at a node...this doesn't really do
        // anything at the moment. game is hard enough without this feature.
        if(alien->wait_time + tick >= LOOK_TIME){
          direct_alien(alien->alien->body, body_get_centroid(((node_t *)list_get(alien->path, 0))->node->body), VEL_STALK);
          alien->wait_time = 0;
          alien->is_moving_toward_node = true;

        } else{
          body_set_velocity(alien->alien->body, VEC_ZERO);
          alien->wait_time += tick;
          alien->is_moving_toward_node = false;
        }
      }
  }
}

// Follows the player.
void basic_follow(map_t *map, double vel){
  direct_alien(map->alien->body, body_get_centroid(map->player->body), vel);
}
