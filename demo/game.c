#include "ailien.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "list.h"
#include "vector.h"
#include "forces.h"
#include "collision.h"
#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/////////////// CMDS ///////////////////////
// 'q' quit, exit game (o/w resets at win/loss)
//  arrow keys to move
// WASD to shoot
// 'e' change to explosive bullets
// 'g' change to grav gun // deprecated :(
//////////////////////////////PARAMS and CONSTANTS//////////////////////////////
// Frame bounds, should be 4x:3y
const double WIN_DIM_X = 500.0;
const double WIN_DIM_Y = 500.0;
const vector_t MIN = (vector_t){0, 0};
const vector_t MAX = (vector_t){2*WIN_DIM_X, 2*WIN_DIM_Y};
const double VIEW_PLAYER = .1;
const double VIEW_DEV = .5;
const double VIEW_ALL = 1;
const double PLAYER_VEL = 100;
const double PLAYER_ANG = M_PI / 2;
const double STAMINA_RATE = .03;
const int MAX_STAMINA = 100;
const int MIN_STAMINA = 50;
double STAMINA = 100;

int NUM_TOTAL_BULLETS;
int NUM_EXPLOSIVES = 5;
const rgb_color_t C_EXP = {255, 0, 0}; // red
const double B_ELAS = 10;
const double WIDTH_BULLET = 2;
const double LENGTH_BULLET = 4;
const int BULLET_SIDES = 4; // Rectangle
const double B_MASS = 20;
const double B_VEL = 200;
const int STALK_RADIUS_EASY = 20;
const int STALK_RADIUS_MEDIUM = 15;
const int STALK_RADIUS_HARD = 10;
// 'e' for explosive, 'g' for grav gun, .. gets changed.
char *BULLET_TYPE = "e";
body_t *BULLET;
map_t *map;

double PLAYER_ANGLE = 0.0;

// Clean up
void end_game(map_t * map){
  map_free(map);
  exit(0);
}

// Regain stamina faster when hiding, but still regain some when standing still. Otherwise, lose stamina.
void set_stamina(map_t *map){
  body_t *player = map->player->body;
  vector_t cur_vel = body_get_velocity(player);
  if(cur_vel.x == 0 && cur_vel.y == 0 && (STAMINA < MAX_STAMINA)){
    if(is_hiding(map)){
      STAMINA += 1.5 * STAMINA_RATE;
    }
    else{
      STAMINA += STAMINA_RATE / 5;
    }
    update_stamina((int)(100 *(STAMINA - MIN_STAMINA)/MIN_STAMINA));
  }
  else if(STAMINA > MIN_STAMINA){
    STAMINA -= STAMINA_RATE;
    update_stamina((int)(100 *(STAMINA - MIN_STAMINA)/MIN_STAMINA));
  }
}

// Make a bullet and assign to global variable (only shooting one at a time).
// Bullet at center of screen, length by width w/ velocity in dir angle.
void make_bullet(){
  // Make bullet's points
  list_t *points = list_init(BULLET_SIDES, vec_free);
  vector_t *temp1 = malloc(sizeof(vector_t));
  assert(temp1 != NULL);
  *temp1 = (vector_t){LENGTH_BULLET / 2.0, WIDTH_BULLET / 2.0};
  list_add(points, temp1);
  vector_t *temp2 = malloc(sizeof(vector_t));
  assert(temp2 != NULL);
  *temp2 = (vector_t){LENGTH_BULLET / 2.0, - WIDTH_BULLET / 2.0};
  list_add(points, temp2);
  vector_t *temp3 = malloc(sizeof(vector_t));
  assert(temp3 != NULL);
  *temp3 = (vector_t){-LENGTH_BULLET / 2.0, -WIDTH_BULLET / 2.0};
  list_add(points, temp3);
  vector_t *temp4 = malloc(sizeof(vector_t));
  assert(temp4 != NULL);
  *temp4 = (vector_t){-LENGTH_BULLET / 2.0, WIDTH_BULLET / 2.0};
  list_add(points, temp4);
  // Set color, name
  rgb_color_t col;
  char *name = malloc(10*sizeof(char));
  strcpy(name, BULLET_TYPE);
  col = C_EXP;
  BULLET = body_init_with_info(points, B_MASS, col, name, free);
}

// Spawns bullet, moves to correct position/orientation and adds collisions.
void shoot(map_t *map, double angle){
  NUM_TOTAL_BULLETS = NUM_EXPLOSIVES;
  if(NUM_TOTAL_BULLETS > 0){
    make_bullet();
    body_t *bullet = BULLET;
    // Set centroid, rotate, velocity
    body_set_centroid(bullet, body_get_centroid(map->player->body));
    body_set_rotation(bullet, angle);
    vector_t vel = vec_multiply(B_VEL, vec_rotate((vector_t){1,0}, angle));
    body_set_velocity(bullet, vel);
    // Get rid of bullet and bounce alien back if they collide
    NUM_EXPLOSIVES--;
    create_physics_collision(map->scene, B_ELAS, bullet, map->alien->body, 3);
    for(size_t i = 0; i < scene_bodies(map->scene); i++){
      body_t *body = scene_get_body(map->scene, i);
      // Get rid of bullet if it hits something else
      if(strcmp((char *)body_get_info(body), "alien") != 0 && strcmp((char *)body_get_info(body), "player") != 0
      && strcmp((char *)body_get_info(body), "g") != 0 && strcmp((char *)body_get_info(body), "e") != 0 ){
        create_destructive_collision(map->scene, bullet, body, 1);
      }
    }
    // Show it afterwards
    scene_add_body(map->scene, bullet);
    // Update text displaying number of bullets left
    update_bullets(NUM_EXPLOSIVES);
  }
}

// Handles key presses for motion, shooting. Movement velocity is constant and
// then is halved once stamina reaches 0.
void key_handle(scene_t *scene, char key, key_event_type_t type, double dt, void *aux){
  body_t *player = map->player->body;
  vector_t vx, vy;
  if(STAMINA > MIN_STAMINA){
    vx = (vector_t){PLAYER_VEL, 0};
    vy = (vector_t){0, PLAYER_VEL};
  }
  else{
    vx = vec_multiply(.5, (vector_t){PLAYER_VEL, 0});
    vy = vec_multiply(.5, (vector_t){0, PLAYER_VEL});
  }
  // Uncomment below if speed to scale down with stamina is desired
  // vector_t vx = vec_multiply(STAMINA / 100.0, (vector_t){PLAYER_VEL, 0});
  // vector_t vy = vec_multiply(STAMINA / 100.0, (vector_t){0, PLAYER_VEL});
  if (type == KEY_PRESSED) {
    switch(key) {
      case 'q':
        end_game(map);
        break;
      case LEFT_ARROW:
        body_set_velocity(player, vec_negate(vx));
        body_set_rotation(player, -2 * PLAYER_ANG);
        PLAYER_ANGLE = 270.0;
        break;
      case RIGHT_ARROW:
        body_set_velocity(player, vx);
        body_set_rotation(player, 0 * PLAYER_ANG);
        PLAYER_ANGLE = 90.0;
        break;
      case UP_ARROW:
        body_set_velocity(player, vy);
        body_set_rotation(player, PLAYER_ANG);
        PLAYER_ANGLE = 0.0;
        break;
      case DOWN_ARROW:
        body_set_velocity(player, vec_negate(vy));
        body_set_rotation(player, -1 * PLAYER_ANG);
        PLAYER_ANGLE = 180.0;
        break;
      // control weapon shooting with WASD, and have other keys to switch guns.
      case 'w': // up
        shoot(map, M_PI / 2);
        break;
      case 's': // down
        shoot(map, 3 * M_PI / 2);
        break;
      case 'a': // left
        shoot(map, M_PI);
        break;
      case 'd': // right
        shoot(map, 0);
        break;
    }
  } else{
    body_set_velocity(player, (VEC_ZERO));
  }
}

// True if image is in frame of window, false if it is not. To avoid spawning
// images that are not needed.
bool image_in_frame(map_t *map, body_t *image_body){
  vector_t player_centroid = body_get_centroid(map->player->body);
  vector_t image_centroid = body_get_centroid(image_body);
  double delta_x = fabs(image_centroid.x - player_centroid.x);
  double delta_y = fabs(image_centroid.y - player_centroid.y);
  if(delta_x < 260 && delta_y < 260){
    return 1;
  }
  else{
    return 0;
  }
}

// Renders images for hiding spots.
void image_hiding_spots(map_t *map){
  list_t *spots = map->hiding_spots;
  for(size_t i = 0; i < list_size(spots); i++){
    body_t *spots_body = ((object_t *)list_get(spots, i))->body;
    vector_t spots_centroid = body_get_centroid(spots_body);
    if(image_in_frame(map, spots_body) == 1){
      if(strcmp(body_get_info(spots_body), "locker") == 0){
        render_locker_image(spots_centroid);
      }
      else if(strcmp(body_get_info(spots_body), "dumpster") == 0){
        render_dumpster_image(spots_centroid);
      }
    }
  }
}

// Renders images for coins.
void image_coins(map_t *map){
  list_t *coins = map->coins;
  for(size_t i = 0; i < list_size(coins); i++){
    body_t *coins_body = ((object_t *)list_get(coins, i))->body;
    vector_t coins_centroid = body_get_centroid(coins_body);
    if(image_in_frame(map, coins_body) == 1){
      render_coin_image(coins_centroid);
    }
  }
}

// Renders images for walls.
 void image_walls(map_t *map){
   list_t *walls = map->walls;
   for(size_t i = 0; i < list_size(walls); i++){
     body_t *walls_body = ((object_t *)list_get(walls, i))->body;
     vector_t walls_centroid = body_get_centroid(walls_body);
     if(image_in_frame(map, walls_body) == 1){
       render_wall_image(walls_centroid);
     }
   }
 }

// Renders images for doors.
 void image_doors(map_t *map){
   list_t *doors = map->doors;
   for(size_t i = 0; i < list_size(doors); i++){
     body_t *doors_body = ((object_t *)list_get(doors, i))->body;
     vector_t doors_centroid = body_get_centroid(doors_body);
     if(image_in_frame(map, doors_body) == 1){
       render_door_image(doors_centroid);
     }
   }
 }

// Setup and main loop.
int main(){
  // Initialize everything:
  map = map_init();
  sdl_init(MIN, MAX);
  vector_t center = (vector_t){WIN_DIM_X, WIN_DIM_Y};
  body_set_centroid(map->player->body, center);
  body_set_centroid(map->alien->body, vec_add(center, (vector_t){100, 100}));
  alien_t *alien = ai_init_bounds(map);
  sdl_update_zoom(VIEW_PLAYER);
  sdl_on_key(key_handle);
  message_init(); // initialize TTF and font
  images_init(); // initialize image surface and textures
  // While game is still running, update stamina and player velocity, direct
  // the ailien, update sdl frame, check win/loss conditions, render text and
  // images.
  while (!sdl_is_done(map->scene, NULL)){
    double dt = time_since_last_tick();
    set_stamina(map);
    ai_stalk(map, alien, STALK_RADIUS_MEDIUM, dt);
    sdl_update_center(body_get_centroid(map->player->body));
    sdl_clear();
    if(map_lose(map)){
      while (!sdl_is_done(map->scene, NULL)){
        sdl_clear();
        lose_message();
        sdl_render_scene(map->scene);
      }
    }
    else if(map_win(map)){
      while (!sdl_is_done(map->scene, NULL)){
        sdl_clear();
        win_message();
        sdl_render_scene(map->scene);
      }
    }
    map_tick(map, dt);
    render_player_image(PLAYER_ANGLE);
    render_alien_image(body_get_centroid(map->alien->body));
    image_hiding_spots(map);
    image_coins(map);
    image_walls(map);
    image_doors(map);
    render_text(0, 0);
    sdl_render_scene(map->scene);
  }
  // Clean up.
  image_free();
  message_free();
  end_game(map);
  ai_free(alien);
}
