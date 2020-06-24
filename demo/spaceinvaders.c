#include "sdl_wrapper.h"
#include "body.h"
#include "list.h"
#include "vector.h"
#include "forces.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

////////////////////////////////////////
// frame bounds, should be 4x:3y
const vector_t MIN = (vector_t){-100.0, -75.0};
const vector_t MAX = (vector_t){100.0, 75.0};
// base parameters of player, dots. mass = 0 bc no gravity here
const double RADIUS_PLAYER = 5;
const double RADIUS_ENEMY = 8;
const double RADIUS_BULLET = 1;
const double ENEMY_DISTANCE = RADIUS_ENEMY / 2;
const double BUFFER_TOP = 3 * RADIUS_ENEMY / 2;
const rgb_color_t GREEN = {0, 128, 0};
const rgb_color_t GREY = {128, 128, 128};
const rgb_color_t RED = {256, 0, 0};
const rgb_color_t PURPLE = {128, 15, 128};
const double SHIP_ANGLE = 4 * M_PI / 3;
const double PLAYER_VEL = 300;
const double ENEMY_VEL = 50;
const double BULLET_VEL = 700;
const double BULLET_SLOW_FACTOR = 0.75;
const int PLAYER_SIDES = 20;
const int ENEMY_SIDES = 20;
const int BULLET_SIDES = 4; //square
const int ENEMY_COLS = 8;
const int ENEMY_ROWS = 3;
const int NUM_ENEM = ENEMY_COLS * ENEMY_ROWS;
const double MASS = 0;
const char *PLAYER = "player";
const char *ENEMY = "enemy";
const char *PLAYER_BULLET = "player_bullet";
const char *ENEMY_BULLET = "enemy_bullet";
const vector_t BUFFER_BOUND = (vector_t){5.0, 0.0};

void end_game(scene_t * scene){
  scene_free(scene);
  exit(0);
}

// creates player at the bottom center of the screen
body_t *make_player(){
  list_t *points = list_init(PLAYER_SIDES + 1, vec_free);
  vector_t general_pt = (vector_t){RADIUS_PLAYER, 0};
  double rot = (2 * M_PI) / PLAYER_SIDES;
  for(size_t i = 0; i < PLAYER_SIDES; i++){
    vector_t *temp = malloc(sizeof(vector_t));
    assert(temp != NULL);
    *temp = vec_rotate(general_pt, (rot * i));
    list_add(points, temp);
  }
  char *player_name = malloc(10*sizeof(char));
  strcpy(player_name, PLAYER);
  body_t *player = body_init_info(points, MASS, GREEN, player_name);
  body_set_centroid(player, (vector_t){0, MIN.y + RADIUS_PLAYER});
  return player;
}

// creates enemy w/o ref to position--set later
body_t *make_enemy(){
  vector_t *center = malloc(sizeof(vector_t));
  assert(center != NULL);
  *center = VEC_ZERO;
  list_t *points = list_init(ENEMY_SIDES + 1, vec_free);
  vector_t first_pt = (vector_t){0, -RADIUS_ENEMY};
  double rotate = ((2 * M_PI) - SHIP_ANGLE) / ENEMY_SIDES;
  list_add(points, center);
  double ship_angle = SHIP_ANGLE / 2;
  for(size_t i = 0; i < ENEMY_SIDES; i++){
    vector_t *new_pt = malloc(sizeof(vector_t));
    assert(new_pt != NULL);
    *new_pt = vec_rotate(first_pt, ship_angle + (rotate * i));
    list_add(points, new_pt);
  }
  char *enemy_name = malloc(10*sizeof(char));
  strcpy(enemy_name, ENEMY);
  body_t *enemy = body_init_info(points, MASS, GREY, enemy_name);
  return enemy;
}

// creates a bullet at center of screen
body_t *make_bullet(rgb_color_t color, char *body_info){
  list_t *points = list_init(BULLET_SIDES, vec_free);
  vector_t *temp1 = malloc(sizeof(vector_t));
  assert(temp1 != NULL);
  *temp1 = (vector_t){RADIUS_BULLET, RADIUS_BULLET};
  list_add(points, temp1);
  vector_t *temp2 = malloc(sizeof(vector_t));
  assert(temp2 != NULL);
  *temp2 = (vector_t){RADIUS_BULLET, -RADIUS_BULLET};
  list_add(points, temp2);
  vector_t *temp3 = malloc(sizeof(vector_t));
  assert(temp3 != NULL);
  *temp3 = (vector_t){-RADIUS_BULLET, -RADIUS_BULLET};
  list_add(points, temp3);
  vector_t *temp4 = malloc(sizeof(vector_t));
  assert(temp4 != NULL);
  *temp4 = (vector_t){-RADIUS_BULLET,RADIUS_BULLET};
  list_add(points, temp4);
  body_t *bullet = body_init_info(points, MASS, color, body_info);
  return bullet;
}

// makes each enemy at center, moves it, and adds to scene
void set_enemies(scene_t *scene){
  for(int i = 0; i < ENEMY_ROWS * ENEMY_COLS; i++){
    body_t *enemy = make_enemy();
    double x_coor = (MIN.x + ((i + 1) * (ENEMY_DISTANCE + (2 * RADIUS_ENEMY)))) -
            ((i / ENEMY_COLS)*(ENEMY_COLS * (ENEMY_DISTANCE + (2 * RADIUS_ENEMY))));
    double y_coor = MAX.y - (((i / ENEMY_COLS) + 1) * (ENEMY_DISTANCE + RADIUS_ENEMY));
    body_set_centroid(enemy, (vector_t){x_coor, y_coor});
    body_set_velocity(enemy, (vector_t){ENEMY_VEL, 0});
    scene_add_body(scene, enemy);
  }
}

// shifts enemy down to an open row when it goes OOB
void wrap(scene_t *scene){
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *enemy = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(enemy), ENEMY) == 0){
      vector_t center = body_get_centroid(enemy);
      double down_dist = ENEMY_ROWS * (ENEMY_DISTANCE + RADIUS_ENEMY);
      if((center.x - RADIUS_ENEMY) < MIN.x){
        body_set_centroid(enemy, (vector_t){MIN.x + RADIUS_ENEMY , center.y - down_dist});
        body_set_velocity(enemy, (vector_t){ENEMY_VEL, 0});
      }
      if((center.x + RADIUS_ENEMY) > MAX.x){
        body_set_centroid(enemy, (vector_t){MAX.x - RADIUS_ENEMY, center.y - down_dist});
        body_set_velocity(enemy, (vector_t){-ENEMY_VEL, 0});
      }
    }
  }
}

// makes and adds bullet with initial velocity at the player
void player_shoot(scene_t *scene){
  // check to see if already bullet on screen, don't fire
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(body), PLAYER_BULLET) == 0){
      return;
    }
  }
  // get player
  body_t *player = NULL;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(body), PLAYER) == 0){
      player = body;
      break;
    }
  }
  assert(player != NULL);
  vector_t player_center = body_get_centroid(player);
  // make bullet @ centroid
  char *str = malloc(20*sizeof(char));
  strcpy(str, PLAYER_BULLET);
  body_t *player_bullet = make_bullet(GREEN, str);
  body_set_centroid(player_bullet, (vector_t){player_center.x, player_center.y + RADIUS_PLAYER + RADIUS_BULLET});
  body_set_velocity(player_bullet, (vector_t){0, BULLET_VEL});
  scene_add_body(scene, player_bullet);
}

// randomly chooses when and which enemy to shoot bullet
void enemy_shoot(scene_t *scene){
  // check to see if already bullet on screen and don't fire
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(body), ENEMY_BULLET) == 0){
      return;
    }
  }
  int enem_inds[NUM_ENEM];
  int counter = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(body), ENEMY) == 0){
      enem_inds[counter] = i;
      counter++;
    }
  }
  // can replace w ind of enemy closest to player in x dir
  body_t *play = scene_get_body(scene, 0);
  double play_x = body_get_centroid(play).x;
  double min_diff = INFINITY;
  int ind = 0;
  int increment = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    double body_x = body_get_centroid(body).x;
    if(strcmp((char *)body_get_info(body), ENEMY) == 0){
      if(fabs(body_x-play_x) < min_diff){
        ind = increment;
        min_diff = fabs(body_x-play_x);
        increment++;
      } else
        increment++;
    }
  }
  body_t *enemy = scene_get_body(scene, enem_inds[ind]);
  vector_t enemy_center = body_get_centroid(enemy);
  char *str = malloc(20*sizeof(char));
  strcpy(str, ENEMY_BULLET);
  body_t *enemy_bullet = make_bullet(PURPLE, str);
  body_set_centroid(enemy_bullet, (vector_t){enemy_center.x, enemy_center.y});
  body_set_velocity(enemy_bullet, vec_multiply(BULLET_SLOW_FACTOR, (vector_t){0, -BULLET_VEL}));
  scene_add_body(scene, enemy_bullet);
}

// checks enemy bullets; if enemy_bullet contacts player, both are removed and game ends
void remove_player(scene_t *scene){
  body_t *enemy_bullet = NULL;
  body_t *player = NULL;
  size_t ind_eb = 0;
  size_t ind_player = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *body = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(body), ENEMY_BULLET) == 0){
      enemy_bullet = body;
      ind_eb = i;
      // ok bc player added first (@ 0)
      break;
    }
    if(strcmp((char *)body_get_info(body), PLAYER) == 0){
      player = body;
      ind_player = i;
    }
  }
  if(enemy_bullet != NULL && player != NULL){
    if(body_is_colliding(enemy_bullet, player)){
      scene_remove_body(scene, ind_eb);
      scene_remove_body(scene, ind_player);
      end_game(scene);
    }
  }
}

// check all enemies; if player_bullet contacts enemy, both are removed
void remove_enemy(scene_t *scene){
  body_t *player_bullet;
  size_t ind_pb = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *temp = scene_get_body(scene, i);
    if(strcmp((char *)body_get_info(temp), PLAYER_BULLET) == 0){
      player_bullet = temp;
      ind_pb = i;
      size_t x = 0;
      while(x < scene_bodies(scene)){
        body_t *enemy = scene_get_body(scene, x);
        if(enemy != NULL && player_bullet != NULL){
          if((strcmp((char *)body_get_info(enemy), ENEMY) == 0) && body_is_colliding(enemy, player_bullet)){
             scene_remove_body(scene, ind_pb);
             scene_remove_body(scene, x);
             break;
          }
          else{
            x++;
          }
        }
        else{
          x++;
        }
      }
      break;
    }
  }
}

// check if enemy reaches the player and ends game if they collide--could also do based on final line? can't shoot anymore
void reached_end(scene_t *scene){
   body_t *enemy;
   body_t *player;
   size_t ind_enemy = 0;
   size_t ind_player = 0;
   for(size_t i = 0; i < scene_bodies(scene); i++){
     body_t *body = scene_get_body(scene, i);
     if(strcmp((char *)body_get_info(body), PLAYER) == 0){
       player = body;
       ind_player = i;
       break;
     }
   }
   int count = 0;
   for(size_t i = 0; i < scene_bodies(scene); i++){
     body_t *body = scene_get_body(scene, i);
     if(strcmp((char *)body_get_info(body), ENEMY) == 0){
       count++;
       enemy = body;
       ind_enemy = i;
       if(player != NULL && enemy != NULL){
         if(body_is_colliding(enemy, player)){
           scene_remove_body(scene, ind_enemy);
           scene_remove_body(scene, ind_player);
           end_game(scene);
         }
       }
     }
   }
   if(count == 0){
     end_game(scene);
   }
}

// control player in accordance with key presses
void move_player(scene_t *scene, char key, key_event_type_t type, double dt){
  body_t *player = scene_get_body(scene, 0);
  vector_t player_center = body_get_centroid(player);
  if (type == KEY_PRESSED) {
    switch(key) {
      case LEFT_ARROW:
        if(player_center.x > MIN.x + RADIUS_PLAYER){
          body_set_velocity(player, (vector_t){-PLAYER_VEL, 0});
        }
        break;
      case RIGHT_ARROW:
        if(player_center.x < MAX.x - RADIUS_PLAYER){
          body_set_velocity(player, (vector_t){PLAYER_VEL, 0});
        }
        break;
      case ' ':
        player_shoot(scene);
        break;
      case 'q':
        end_game(scene);
    }
  } else{
    body_set_velocity(player, (vector_t){0, 0});
  }
}

// keep player from going off of screen
void bound_player(scene_t *scene){
  body_t *player = scene_get_body(scene, 0);
  vector_t player_center = body_get_centroid(player);
  if(player_center.x + RADIUS_PLAYER >= MAX.x){
    body_set_velocity(player, VEC_ZERO);
    body_set_centroid(player, vec_subtract(player_center, BUFFER_BOUND));
  }
  if(player_center.x - RADIUS_PLAYER <= MIN.x){
    body_set_velocity(player, VEC_ZERO);
    body_set_centroid(player, vec_add(player_center, BUFFER_BOUND));
  }
}

// remove bodies that are oob
void out_of_bounds(scene_t *scene){
  int n = 1;
  while(n < scene_bodies(scene)){
    body_t *body = scene_get_body(scene, n);
    list_t *points = body_get_shape(body);
    for(size_t i = 0; i < list_size(points); i++){
      double x = ((vector_t *)list_get(points, i))->x;
      double y = ((vector_t *)list_get(points, i))->y;
      if(!(x < MAX.x && x > MIN.x && y < MAX.y && y > MIN.y)) {
        scene_remove_body(scene, n);
        n--;
        break;
      }
    }
    list_free(points);
    n++;
  }
}

int main(){
  sdl_init(MIN, MAX);
  scene_t *scene = scene_init();
  body_t *player = make_player();
  scene_add_body(scene, player);
  set_enemies(scene);
  sdl_on_key(move_player);
  while (!sdl_is_done(scene)){
    double dt = time_since_last_tick();
    wrap(scene);
    out_of_bounds(scene);
    enemy_shoot(scene);
    remove_player(scene);
    remove_enemy(scene);
    reached_end(scene);
    bound_player(scene);
    scene_tick(scene, dt);
    sdl_render_scene(scene);
  }
  end_game(scene);
}
