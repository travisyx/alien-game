#include "map.h"

// actual size is just height and width * GRID_SIZE (for basic box)
const int HEIGHT = 100;
const int WIDTH = 100;
const int NUM_WALLS = 2 * HEIGHT * WIDTH; // we know, not actual #. just for list size init
const int NUM_COINS = 100;
// Do not change
const int NUM_DOORS = 2;
// this is # of each type! so this * num types = total
const int NUM_HIDING_SPOTS = 40;
const int START_MONEY = 0;
const int NUM_RECT = 4;
// this is side length for square grid, equiv to spot in the backing array
const int GRID_SIZE = 10;
const double MASS = 0;
bool HIDING = false;
object_t *CURR_SPOT;

/////// consts for making bodies here
const char *PLAYER = "player";
const char *ALIEN = "alien";
const char *COIN = "coin";
const char *NODE = "node";
const char *WALL = "wall";
const char *DOOR = "door";
const double R_PLAYER_ALIEN = GRID_SIZE / 2.5;
const int RECT_SIDES = 4;
const double RADIUS_SCALE = 2.0;
const double ANGLE_SCALE = RADIUS_SCALE;
const double M_ALIEN = 1; // need for weapon elas...also add in bullets at some point
const double M_PLAYER = 1; // need to reflect off items?
const double R_VISIBLE = 1000;
const double R_COIN = GRID_SIZE / 6;
const int V_COIN = 100;
// const int V_DOOR = 4000;
const int V_DOOR = 2000;
const int V_HIDE = 500;

const rgb_color_t GREEN = {0, 128, 0};
const rgb_color_t GREY = {128, 128, 128};
const rgb_color_t RED = {255, 0, 0};
const rgb_color_t C_HIDDEN = {128, 15, 128}; // purple

const rgb_color_t C_DOOR = {255,255,0}; // yellow
const rgb_color_t C_COIN = {192,192,192}; // silver
const rgb_color_t C_WALL = {0,0,0}; // black
const rgb_color_t C_PLAYER = {248,75,8}; // bright orange
const rgb_color_t C_NODE = {255,255,255}; // white
const rgb_color_t C_ALIEN = {11, 253, 25}; // bright green

// format "name" // color, cost, # gen (just dist equally)
const char *HIDING_TYPES[] = {
  "locker", // blue // 5*coin // .5
  "dumpster" // brown // 3*coin // .5
  // "restaurant", // reddouble
  // "manhole", // brown
  // "dumpster", // dark green
  // "bathroom", //
  // "car", // blue
  // "house" // tan
};
const int NUM_HIDING_TYPES = 2; // update if you change!!
const rgb_color_t C_HIDING_SPOTS[] = {{0,191,255}, {139,69,19}}; // change this too! should be in order

node_t *node_init(object_t *node, double priority){
  node_t *ans = malloc(sizeof(node_t));
  ans->node = node;
  ans->priority = priority;
  ans->num_neighbors = 0;
  for(int i = 0; i < 8; i++)
    ans->distances[i] = -1;
  ans->visited = false;
  return ans;
}

void node_free(void *node){
  free(((node_t *)node));
}

///////// some methods to make backing array easier to access--could abstract this out if wanted?//////////////
vector_t arr_size(list_t *arr){
  vector_t size;
  size.x = list_size(arr);
  size.y = list_size((list_t *)list_get(arr, 0));
  return size;
}

list_t *arr_init(int height, int width, free_func_t freer){
  list_t *ans = list_init(height, list_free);
  for(size_t i = 0; i < height; i++){
    // these should only take in object_ts, NOT body_ts
    list_t *new_list = list_init(width, freer);
    list_add(ans, new_list);
  }
  return ans;
}

void arr_free(list_t *arr){
  list_free(arr);
}

void *arr_get(list_t *arr, int r, int c){
  void *obj = (void *)list_get((list_t *)list_get(arr, r), c);
  return obj;
}

// assumes that array is already full/there is something at this position already
// will make body into object. objects should only be used for 2d array.
// returns the old object at this position
void *arr_put(list_t *arr, int r, int c, void *new_obj){
  void *old = (void *)list_replace((list_t *)list_get(arr, r), c, new_obj);
  return old;
}

// return the center of the square rep by given index
vector_t map_pos_from_ind(map_t *map, int r, int c){
  vector_t center;
  center.x = (GRID_SIZE / 2.0) + (c * GRID_SIZE);
  center.y = (GRID_SIZE / 2.0) + (r * GRID_SIZE);
  return center;
}

// position given in x,y where 0,0 is top left, 0, width is top right
// returns in row, col
vector_t map_ind_from_pos(map_t *map, vector_t position){
  return (vector_t){(int) position.y/(10), (int) position.x/(10)};
}

// helper.
double shortest_dist(vector_t src, vector_t dest, vector_t obstacle){
  double num = fabs((dest.y-src.y)*obstacle.x - (dest.x-src.x)*obstacle.y + dest.x*src.y - dest.y*src.x);
  double den = vec_distance(dest, src);
  return num/den;
}

//////////////////////////////////////////////////
// check if hiding spot is in straight line btwn src, dest.
bool hiding_in_vec(vector_t src, vector_t dest, list_t *hiding){
  for(size_t i = 0; i < list_size(hiding); i++){
    object_t *hide = (object_t *) list_get(hiding, i);
    vector_t centroid = body_get_centroid(hide->body);
    if(shortest_dist(src, dest, centroid) < R_PLAYER_ALIEN){
      return true;
    }
  }
  return false;
}

void make_node_neighbors(map_t *map, int row, int col){
  node_t *struct_node = (node_t *)arr_get(map->struct_nodes, row, col);
  double straight = 10;
  double diag = 10 * pow(2, 0.5);
  // left
  if(col > 1){
    int r = row;
    int c = col - 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // right
  if(col < WIDTH - 2){
    int r = row;
    int c = col + 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // top
  if(row > 1){
    int r = row - 1;
    int c = col;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // bottom
  if(row < HEIGHT - 2){
    int r = row + 1;
    int c = col;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // TL
  if(row > 1 && col > 1){
    int r = row - 1;
    int c = col - 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // TR
  if(row > 1 && col < WIDTH - 2){
    int r = row - 1;
    int c = col + 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // BL
  if(row < HEIGHT - 2 && col > 1){
    int r = row +  1;
    int c = col - 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  // BR
  if(row < HEIGHT - 2 && col < WIDTH - 2){
    int r = row + 1;
    int c = col + 1;
    node_t *cell = (node_t *)arr_get(map->struct_nodes, r, c);
    if(strcmp(cell->node->type, "wall") != 0){
      struct_node->neighbors[struct_node->num_neighbors] = cell;
      struct_node->num_neighbors++;
      if(r == row || c == col){
        struct_node->distances[struct_node->num_neighbors] = straight;
      } else{
        struct_node->distances[struct_node->num_neighbors] = diag;
      }
    }
  }
  if(struct_node->num_neighbors == 0){
    printf("ALERT: 0 neighbors\n");
  }
}

// ONLY CALL after everything else init
// inits node_ts with actual objects in them after objects in backing array set
void pop_struct_nodes(map_t *map){
  // init nodes
  for(size_t r = 0; r < arr_size(map->backing_array).x; r++){
    list_t *struct_row = (list_t *)list_get(map->struct_nodes, r);
    for(size_t c = 0; c < arr_size(map->backing_array).y; c++){
      object_t *obj = (object_t *)arr_get(map->backing_array, r, c);
      node_t *struct_node = node_init(obj, INFINITY);
      list_add(struct_row, struct_node);
    }
  }
  // neighbors....only for nodes, hiding spots
  for(size_t r = 0; r < arr_size(map->backing_array).x; r++){
    for(size_t c = 0; c < arr_size(map->backing_array).y; c++){
      object_t *obj = (object_t *)arr_get(map->backing_array, r, c);
      if(strcmp(obj->type, WALL) != 0 && strcmp(obj->type, DOOR) != 0){
        make_node_neighbors(map, r, c);
      }
    }
  }
}

map_t *map_init(){
  map_t *map = malloc(sizeof(map_t));
  assert(map != NULL);
  map->scene = scene_init();
  map->backing_array = arr_init(HEIGHT, WIDTH, NULL);
  map->purse = START_MONEY;
  // spawn stat objs
  map->struct_nodes = arr_init(HEIGHT, WIDTH, node_free);
  // important!! nodes must add before walls, and everything else afterward!
  map_add_nodes(map);
  map_add_walls(map);
  map_add_doors(map);
  map_add_hiding_spots(map);
  map_add_coins(map);
  populate_lists(map);
  CURR_SPOT = NULL;
  pop_struct_nodes(map);
  // spawn after to avoid triggering coll bc init @ 0
  map->player = object_init(make_player());
  scene_add_body(map->scene, map->player->body);
  map->alien = object_init(make_alien());
  scene_add_body(map->scene, map->alien->body);
  return map;
}

// triangle so can tell which direction facing. will start pointing to the left
body_t *make_player(){
    list_t *pts = list_init(RECT_SIDES, vec_free);
    double angle = 2 * M_PI / RECT_SIDES;
    // generate vectors for general case points
    vector_t general_pt = (vector_t){R_PLAYER_ALIEN, 0};
    for(size_t i = 0; i < RECT_SIDES; i++){
      double angle_ver = (i) * angle + M_PI / RECT_SIDES;
      vector_t *vert = malloc(sizeof(vector_t));
      assert(vert != NULL);
      *vert = vec_rotate(general_pt, angle_ver);
      list_add(pts, vert);
    }
    char *obj_name = malloc(10*sizeof(char));
    strcpy(obj_name,PLAYER);
    body_t *body = body_init_with_info(pts, M_PLAYER, C_PLAYER,obj_name, free);
    return body;
}

body_t *make_alien(){
  list_t *pts = list_init(RECT_SIDES, vec_free);
  double angle = 2 * M_PI / RECT_SIDES;
  // generate vectors for general case points
  vector_t general_pt = (vector_t){R_PLAYER_ALIEN, 0};
  for(size_t i = 0; i < RECT_SIDES; i++){
    double angle_ver = (i) * angle + M_PI / RECT_SIDES;
    vector_t *vert = malloc(sizeof(vector_t));
    assert(vert != NULL);
    *vert = vec_rotate(general_pt, angle_ver);
    list_add(pts, vert);
  }
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name,ALIEN);
  body_t *body = body_init_with_info(pts, M_ALIEN, C_ALIEN,obj_name, free);
    return body;
}

void map_free(map_t *map){
  scene_free(map->scene);
  list_free(map->walls);
  list_free(map->doors);
  list_free(map->coins);
  // free bodies of nodes bc not put into the scene
  for(size_t i = 0; i < list_size(map->nodes); i++){
    object_t *node = (object_t*)list_get(map->nodes, i);
    body_free(node->body);
  }
  list_free(map->nodes);
  list_free(map->hiding_spots);
  arr_free(map->backing_array);
  arr_free(map->struct_nodes);
  object_free(map->player);
  object_free(map->alien);
  free(map);
}

// should only be called at the beginning. assumes that objects at this time are
// staying in place and not being removed/added i.e. FINAL state of map.
void populate_lists(map_t *map){
  map->walls = list_init(NUM_WALLS, object_free);
  map->doors = list_init(NUM_DOORS, object_free);
  map->coins = list_init(NUM_COINS, object_free);
  map->hiding_spots = list_init(NUM_HIDING_SPOTS * NUM_HIDING_TYPES, object_free);
  map->nodes = list_init(HEIGHT * WIDTH, object_free);
  // get coins first bc not in array
  for(size_t i = 0; i < scene_bodies(map->scene); i++){
    body_t *b_coin = scene_get_body(map->scene, i);
    char *type = body_get_info(b_coin);
    if(strcmp(type, COIN) == 0){
      object_t *o_coin = object_init(b_coin);
      object_calc_min_max(o_coin);
      list_add(map->coins, o_coin);
    }
  }
  // get everything else
  for(size_t r = 0; r < HEIGHT; r++){
    for(size_t c = 0; c < WIDTH; c++){
      object_t *o = (object_t *) arr_get(map->backing_array, r, c);
      char *type = o->type;
      if(strcmp(type, WALL) == 0){
        list_add(map->walls, o);
        scene_add_body(map->scene, o->body);
      }
      else if(strcmp(type, DOOR) == 0){
        list_add(map->doors, o);
        scene_add_body(map->scene, o->body);
      }
      else if(strcmp(type, NODE) == 0){
        list_add(map->nodes, o);
      }
      else if(strcmp(type, PLAYER) != 0 && strcmp(type, ALIEN) != 0){
        list_add(map->hiding_spots, o);
        scene_add_body(map->scene, o->body);
      }
    }
  }
}

/////////////////////////////////////////////////

// when player collides with coin, remove the coin and add value to player's purse
void map_collect_coin(map_t *map){
  object_t *player = map->player;
  list_t *coins = map->coins;
  for(size_t i = 0; i < list_size(coins); i++){
    if(object_collision(player, (object_t *)list_get(coins, i)) == true){
      map->purse += V_COIN;
      update_money(map->purse);
      body_remove(((object_t *)list_get(coins, i))->body);
      object_free(list_remove(coins, i));
      break;
    }
  }
}

// true if transaction can go through. false if not enough money.
// if true, will take money out of purse
bool spend_money(map_t *map, int cost){
  if(map->purse < cost){
    return false;
  }
  else{
    map->purse -= cost;
    update_money(map->purse);
    return true;
  }
}

// touch you buy..spends money here
// will just leave player on top of hiding spot. considered hiding if centroid still in
void map_hide_player(map_t *map){
  if(HIDING == false){
    list_t *spots = map->hiding_spots;
    for(size_t i = 0; i < list_size(spots); i++){
      // assumption is that only coll with one hiding spot...will take first found
      if(object_collision(map->player, (object_t *)list_get(spots, i))){
        CURR_SPOT = (object_t *)list_get(spots, i);
        if(CURR_SPOT->is_purchased || spend_money(map, V_HIDE)){
          HIDING = true;
          CURR_SPOT->is_purchased = true;
          break;
        }
        else{
          CURR_SPOT = NULL;
        }
      }
    }
    if(CURR_SPOT != NULL && CURR_SPOT->is_purchased){
      body_set_centroid(map->player->body, body_get_centroid(CURR_SPOT->body));
      body_set_velocity(map->player->body, VEC_ZERO);
      body_set_color(CURR_SPOT->body, C_HIDDEN);
    }
  }
}

void map_unhide_player(map_t *map){
  if(CURR_SPOT != NULL && HIDING == true){
    // make sure centroid is outside of box-then unhidden
    vector_t center_h = body_get_centroid(CURR_SPOT->body);
    vector_t center_p = body_get_centroid(map->player->body);
    if(center_p.x > center_h.x + GRID_SIZE || center_p.x < center_h.x - GRID_SIZE
    || center_p.y > center_h.y + GRID_SIZE || center_p.y < center_h.y - GRID_SIZE){
      for(size_t i = 0; i < NUM_HIDING_TYPES; i++){
        if(strcmp(CURR_SPOT->type, HIDING_TYPES[i]) == 0){
          body_set_color(CURR_SPOT->body, C_HIDING_SPOTS[i]);
          HIDING = false;
          CURR_SPOT->is_open = true;
          CURR_SPOT = NULL;
          break;
        }
      }
    }
  }
}

bool is_hiding(map_t *map){
  return HIDING;
}

void open_door(map_t *map){
  list_t *doors = map->doors;
  for(size_t i = 0; i < list_size(doors); i++){
    object_t *door = (object_t *)list_get(doors, i);
    if(door->is_open || (object_collision(map->player, door) && spend_money(map, V_DOOR))){
      door->is_purchased = true;
      door->is_open = true;
    }
  }
}

// if player is colliding with door and can spend money, returns yes to win
bool map_win(map_t *map){
  list_t *doors = map->doors;
  for(size_t i = 0; i < list_size(doors); i++){
    object_t *door = (object_t *)list_get(doors, i);
    if(object_collision(map->player, door) && door->is_purchased && door->is_open){
      return true;
    }
  }
  return false;
}

// checks if player and alien colliding, if yes, then lose.
bool map_lose(map_t *map){
  return object_collision(map->player, map->alien);
}

/////////////// collisions /////////////////////////
bool object_collision(object_t *o1, object_t *o2){
  if(object_test_bounding_box(o1, o2) == false){
    return false;
  }
  else{
    list_t *shape1 = body_get_shape(o1->body);
    list_t *shape2 = body_get_shape(o2->body);
    collision_info_t info = object_find_collision(shape1, shape2);
    list_free(shape1);
    list_free(shape2);
    return info.collided;
  }
}

// don't let player go into objects it shouldn't be able to go into
void bounce(map_t *map){
  // walls: always bounce
  for(size_t i = 0; i < list_size(map->walls); i++){
    object_t *curr = (object_t *)list_get(map->walls, i);
    if(object_collision(map->player, curr)){
      vector_t center_p = body_get_centroid(map->player->body);
      vector_t center_h = body_get_centroid(curr->body);
      if((center_p.x - R_PLAYER_ALIEN < center_h.x + GRID_SIZE / 2 && center_p.x + R_PLAYER_ALIEN > center_h.x - GRID_SIZE / 2)
    && (center_p.y - R_PLAYER_ALIEN < center_h.y + GRID_SIZE / 2 && center_p.y + R_PLAYER_ALIEN > center_h.y - GRID_SIZE / 2)){
      vector_t vel = body_get_velocity(map->player->body);
      if(center_p.x < center_h.x && vel.x > 0){
        // left
        body_set_velocity(map->player->body, vec_negate((vector_t){fabs(vel.x), 0}));
      }
      else if(center_p.y < center_h.y && vel.y > 0){
        // bottom
        body_set_velocity(map->player->body, vec_negate((vector_t){0, fabs(vel.y)}));
      }
      else if(center_p.x > center_h.x && vel.x < 0){
        // right
        body_set_velocity(map->player->body, (vector_t){fabs(vel.x), 0});
      }
      else if(center_p.y > center_h.y && vel.y < 0){
        // top
        body_set_velocity(map->player->body, (vector_t){0, fabs(vel.y)});
      }
      }
    }
  }
  // doors: bounce unless is_purchased. check if purch first
  open_door(map);
  for(size_t i = 0; i < list_size(map->doors); i++){
    object_t *curr = (object_t *)list_get(map->doors, i);
    if(!curr->is_purchased && object_collision(map->player, curr)){
      vector_t center_p = body_get_centroid(map->player->body);
      vector_t center_h = body_get_centroid(curr->body);
      if((center_p.x - R_PLAYER_ALIEN < center_h.x + GRID_SIZE / 2 && center_p.x + R_PLAYER_ALIEN > center_h.x - GRID_SIZE / 2)
    && (center_p.y - R_PLAYER_ALIEN < center_h.y + GRID_SIZE / 2 && center_p.y + R_PLAYER_ALIEN > center_h.y - GRID_SIZE / 2)){
      vector_t vel = body_get_velocity(map->player->body);
      if(center_p.x < center_h.x && vel.x > 0){
        // left
        body_set_velocity(map->player->body, vec_negate((vector_t){fabs(vel.x), 0}));
      }
      else if(center_p.y < center_h.y && vel.y > 0){
        // bottom
        body_set_velocity(map->player->body, vec_negate((vector_t){0, fabs(vel.y)}));
      }
      else if(center_p.x > center_h.x && vel.x < 0){
        // right
        body_set_velocity(map->player->body, (vector_t){fabs(vel.x), 0});
      }
      else if(center_p.y > center_h.y && vel.y < 0){
        // top
        body_set_velocity(map->player->body, (vector_t){0, fabs(vel.y)});
      }
      }
    }
  }
  // hiding spots: bounce unless HIDING or is_purchased
  map_hide_player(map);
  if(!HIDING){
    for(size_t i = 0; i < list_size(map->hiding_spots); i++){
      object_t *curr = (object_t *)list_get(map->hiding_spots, i);
      if(!curr->is_purchased && object_collision(map->player, curr)){
        vector_t center_p = body_get_centroid(map->player->body);
        vector_t center_h = body_get_centroid(curr->body);
        if((center_p.x - R_PLAYER_ALIEN < center_h.x + GRID_SIZE / 2 && center_p.x + R_PLAYER_ALIEN > center_h.x - GRID_SIZE / 2)
      && (center_p.y - R_PLAYER_ALIEN < center_h.y + GRID_SIZE / 2 && center_p.y + R_PLAYER_ALIEN > center_h.y - GRID_SIZE / 2)){
          vector_t vel = body_get_velocity(map->player->body);
          if(center_p.x < center_h.x && vel.x > 0){
            // left
            body_set_velocity(map->player->body, vec_negate((vector_t){fabs(vel.x), 0}));
          }
          else if(center_p.y < center_h.y && vel.y > 0){
            // bottom
            body_set_velocity(map->player->body, vec_negate((vector_t){0, fabs(vel.y)}));
          }
          else if(center_p.x > center_h.x && vel.x < 0){
            // right
            body_set_velocity(map->player->body, (vector_t){fabs(vel.x), 0});
          }
          else if(center_p.y > center_h.y && vel.y < 0){
            // top
            body_set_velocity(map->player->body, (vector_t){0, fabs(vel.y)});
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////

// some specialized methods for adding specific types

// makes box at pos 0, no info. you can set centroid and info yourself. has generic 10*10 size
body_t *make_box(rgb_color_t color){
  list_t *points = list_init(NUM_RECT, vec_free);
  vector_t *temp1 = malloc(sizeof(vector_t));
  assert(temp1 != NULL);
  *temp1 = (vector_t){GRID_SIZE/2,(-1)*GRID_SIZE/2}; // lower right
  list_add(points, temp1);
  vector_t *temp2 = malloc(sizeof(vector_t));
  assert(temp2 != NULL);
  *temp2 = (vector_t){GRID_SIZE/2,GRID_SIZE/2}; // upper right
  list_add(points, temp2);
  vector_t *temp3 = malloc(sizeof(vector_t));
  assert(temp3 != NULL);
  *temp3 = (vector_t){(-1)*GRID_SIZE/2, GRID_SIZE/2}; // upper left
  list_add(points, temp3);
  vector_t *temp4 = malloc(sizeof(vector_t));
  assert(temp4 != NULL);
  *temp4 = (vector_t){(-1)*GRID_SIZE/2, (-1)*GRID_SIZE/2}; // lower left
  list_add(points, temp4);
  body_t *box = body_init(points, MASS, color);
  return box;
}

object_t *map_make_node(map_t *map){
  body_t *box = make_box(C_NODE);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, NODE);
  body_put_info(box, obj_name, free);
  object_t *node = object_init(box);
  return node;
}

// should fill all spots in 2d array with a node. only to be used at start.
// nodes are lowest priority -- will be replaced by anything else being placed at spot
void map_add_nodes(map_t *map){
  for(size_t r = 0; r < HEIGHT; r++){
    list_t *row = (list_t *)list_get(map->backing_array, r);
    for(size_t c = 0; c < WIDTH; c++){
      object_t *node = map_make_node(map);
      body_set_centroid(node->body, map_pos_from_ind(map, r, c));
      list_add(row, node);
    }
  }
}

// get rid of node at particular spot. only helper func.
// assumes new item is already in scene. makes into object that belongs to backing array
// DOES NOT ACCOUNT FOR NODE LIST. SHOULD ONLY BE USED BEFORE THAT LIST IS CREATED.
void map_replace_node(map_t *map, int r, int c, object_t *new_ob){
  object_t *node = (object_t *)arr_put(map->backing_array, r, c, new_ob);
  body_free(node->body);
  object_free(node);
}

object_t *map_make_wall(map_t *map){
  body_t *box = make_box(C_WALL);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, WALL);
  body_put_info(box, obj_name, free);
  object_t *wall = object_init(box);
  return wall;
}

// creates a wall given coordinates of the 2D-array
void create_walls(map_t *map, size_t r, size_t c){
  object_t *wall = map_make_wall(map);
  body_set_centroid(wall->body, map_pos_from_ind(map, r, c));
  map_replace_node(map, r, c, wall);
  // update bc moved centroid
  object_calc_min_max(wall);
}

// generates walls around the boarder of map and inside of it. hard coded right
// now, could use maze generation algorithm in future.
void map_add_walls(map_t *map){
  for(size_t r = 0; r < HEIGHT; r++){
    for(size_t c = 0; c < WIDTH; c++){
      // creates walls at the edge of the map as boundaries
      if(r == 0 || c == 0 || r == HEIGHT - 1 || c == WIDTH - 1){
        create_walls(map, r, c);
      }
      //create each individual horizontal wall in game
      if (r == 10 && (10 <= c && 25 <= c)){
        create_walls(map, r, c);
      }
      if (r == 10 && (45 <= c &&  c <= 65)){
        create_walls(map, r, c);
      }
      if (r == 25 && (75 <= c && c <= 95)){
        create_walls(map, r, c);
      }
      if (r == 30 && (5 <= c && c <= 20)){
        create_walls(map, r, c);
      }
      if (r == 40 && (20 <= c && c <= 50)){
        create_walls(map, r, c);
      }
      if (r == 45 && (70 <= c && c <= 85)){
        create_walls(map, r, c);
      }
      if (r == 50 && (5 <= c && c <= 15)){
        create_walls(map, r, c);
      }
      if (r == 60 && (35 <= c && c <= 55)){
        create_walls(map, r, c);
      }
      if (r == 65 && ((60 <= c && c <= 75) || (85 <= c && c <= 95))){
        create_walls(map, r, c);
      }
      if (r == 70 && (5 <= c && c <= 50)){
        create_walls(map, r, c);
      }
      if (r == 75 && (85 <= c && c <= 95)){
        create_walls(map, r, c);
      }
      if (r == 80 && (5 <= c && c <= 40)){
        create_walls(map, r, c);
      }
      if (r == 95 && (45 <= c && c <= 60)){
        create_walls(map, r, c);
      }
      // create each individual vertical wall in game
      if (c == 20 && (25 <= r && r <= 60)){
        create_walls(map, r, c);
      }
      if (c == 25 && (85 <= r && r <= HEIGHT - 1)){
        create_walls(map, r, c);
      }
      if (c == 30 && (0 <= r && r <= 30)){
        create_walls(map, r, c);
      }
      if (c == 40 && (0 <= r && r <= 12)){
        create_walls(map, r, c);
      }
      if (c == 45 && (60 <= r && r <= 70)){
        create_walls(map, r, c);
      }
      if (c == 45 && (80 <= r && r <= 95)){
        create_walls(map, r, c);
      }
      if (c == 60 && (65 <= r && r <= 95)){
        create_walls(map, r, c);
      }
      if (c == 65 && (10 <= r && r <= 50)){
        create_walls(map, r, c);
      }
      if (c == 70 && (80 <= r && r <= 95)){
        create_walls(map, r, c);
      }
      if (c == 75 && (65 <= r && r <= 80)){
        create_walls(map, r, c);
      }
      if (c == 80 && (35 <= r && r <= 40)){
        create_walls(map, r, c);
      }
      if (c == 85 && (0 <= r && r <= 15)){
        create_walls(map, r, c);
      }
      if (c == 85 && (75 <= r && r <= 90)){
        create_walls(map, r, c);
      }
      if (c == 90 && (85 <= r && r <= 95)){
        create_walls(map, r, c);
      }
      if (c == 95 && (50 <= r && r <= 65)){
        create_walls(map, r, c);
      }
    }
  }
}

// replace wall..only used to place doors.
 void map_replace_wall(map_t *map, int r, int c, object_t *new_ob){
  object_t *wall = (object_t *)arr_put(map->backing_array, r, c, new_ob);
  body_free(wall->body);
  object_free(wall);
}

// add door to spot in 2d array, get rid of wall if already there.
object_t *map_make_door(map_t *map){
  body_t *box = make_box(C_DOOR);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, DOOR);
  body_put_info(box, obj_name, free);
  object_t *door = object_init(box);
  return door;
}

// door 0 is left, 1 is right. spawned randomly along side walls.
void map_add_doors(map_t *map){
  srand(time(0));
  int val1 = rand() % ((int)((double) HEIGHT - 1)) + 1;
  int val2 = rand() % ((int)((double) HEIGHT - 1)) + 1;
  object_t *door_one = map_make_door(map);
  map_replace_wall(map, val1, 0, door_one);
  object_t *door_two = map_make_door(map);
  map_replace_wall(map, val2, WIDTH - 1, door_two);
  body_set_centroid(door_one->body, map_pos_from_ind(map, val1, 0));
  body_set_centroid(door_two->body, map_pos_from_ind(map, val2, WIDTH - 1));
  // update bc moved centroid
  object_calc_min_max(door_one);
  object_calc_min_max(door_two);
}

body_t *map_make_coin(map_t *map){
  list_t *points = list_init(NUM_RECT, vec_free);
  vector_t *temp1 = malloc(sizeof(vector_t));
  assert(temp1 != NULL);
  *temp1 = (vector_t){R_COIN,(-1)*R_COIN}; // lower right
  list_add(points, temp1);
  vector_t *temp2 = malloc(sizeof(vector_t));
  assert(temp2 != NULL);
  *temp2 = (vector_t){R_COIN,R_COIN}; // upper right
  list_add(points, temp2);
  vector_t *temp3 = malloc(sizeof(vector_t));
  assert(temp3 != NULL);
  *temp3 = (vector_t){(-1)*R_COIN, R_COIN}; // upper left
  list_add(points, temp3);
  vector_t *temp4 = malloc(sizeof(vector_t));
  assert(temp4 != NULL);
  *temp4 = (vector_t){(-1)*R_COIN, (-1)*R_COIN}; // lower left
  list_add(points, temp4);
  body_t *box = body_init(points, MASS, C_COIN);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, COIN);
  body_put_info(box, obj_name, free);
  scene_add_body(map->scene, box);
  return box;
}

// could have respawn at interval? perhaps only spawn outside player radius
// should NOT replace node
void map_add_coins(map_t *map){
  for(int i = 0; i < NUM_COINS; i++){
    bool check = false;
    int x, y;
    while(!check){
      x = rand() % (WIDTH-1) + 1;
      y = rand() % (HEIGHT-1) + 1;
      object_t *curr = (object_t *) arr_get(map->backing_array, x, y);
      if(strcmp(curr->type, NODE) == 0){
        body_t *coin = map_make_coin(map);
        body_set_centroid(coin, map_pos_from_ind(map, x, y));
        check = true;
      }
    }
  }
}

// unid hiding spot..to be assigned later
object_t *map_make_hiding_spot(map_t *map, int ind){
  body_t *box = make_box(C_HIDING_SPOTS[ind]);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, HIDING_TYPES[ind]);
  body_put_info(box, obj_name, free);
  object_t *spot = object_init(box);
  return spot;
}

void map_add_hiding_spots(map_t *map){
  for(int i = 0; i < NUM_HIDING_SPOTS; i++){
    bool check = false;
    int x, y;
    while(!check){
      x = rand() % (WIDTH-1) + 1;
      y = rand() % (HEIGHT-1) + 1;
      object_t *curr = (object_t *) arr_get(map->backing_array, x, y);
      if(strcmp(curr->type, NODE) == 0){
        object_t *hiding = map_make_hiding_spot(map, i%2);
        map_replace_node(map, x, y, hiding);
        body_set_centroid(hiding->body, map_pos_from_ind(map, x, y));
        // update bc moved centroid
        object_calc_min_max(hiding);
        check = true;
      }
    }
  }
}

/////////////////////////////////

// should call scene tick, and also collect_coin, lose, etc.
void map_tick(map_t *map, double dt){
  object_calc_min_max(map->player);
  object_calc_min_max(map->alien);
  map_collect_coin(map);
  bounce(map);
  if(is_hiding(map)){
    map_unhide_player(map);
  }
  scene_tick(map->scene, dt);
}
