/// deprecated :)
#include "sdl_wrapper.h"
#include "body.h"
#include "list.h"
#include "vector.h"
#include "forces.h"
#include "collision.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/////////////// CMDS ///////////////////////
// 'q' quit, exit game (o/w resets at win/loss)
// 'n' normal
// 'f' fast ball, 2x speed
// 's' slow ball, .5x speed
// 'm' multiple balls, 10 total -- rolled back, in development
// 'b' big paddle, paddle is 2x longer
// other ideas: changing colors, bouncy walls, space mode (can space to make new ball), mirror paddle movements, mult. paddles, etc.
////////////////////////////////////////
// frame bounds, should be 4x:3y
const double WIN_DIM_X = 5000.0;
const double WIN_DIM_Y = 5000.0;
const vector_t MIN = (vector_t){-WIN_DIM_X, -WIN_DIM_X};
const vector_t MAX = (vector_t){WIN_DIM_X, WIN_DIM_Y};
// base parameters of player, dots. mass = 0 bc no gravity here
const vector_t PLAYER_DIM = (vector_t){20, 2};
const double RADIUS_BALL = 125;
const double BRICK_GAP = 2;
const size_t BRICK_COLS = 8; // DO NOT CHANGE
const size_t BRICK_ROWS = 5; // DO CHANGE DESIRED
const size_t BALL_SIDES = 20;
const size_t BRICK_HEIGHT = 45;
const size_t BRICK_X = (2*WIN_DIM_X-BRICK_GAP*(BRICK_COLS-1))/BRICK_COLS;
const size_t BRICK_Y = (BRICK_HEIGHT-BRICK_GAP*(BRICK_ROWS-1))/BRICK_ROWS;
const vector_t BRICK_DIM = (vector_t){BRICK_X, BRICK_Y};
const double ELAS = 1.0;
// const double ENEMY_DISTANCE = RADIUS_ENEMY / 2;
// const double BUFFER_TOP = 3 * RADIUS_ENEMY / 2;
const rgb_color_t GREEN = {0, 128, 0};
const rgb_color_t GREY = {128, 128, 128};
const rgb_color_t RED = {255, 0, 0};
const rgb_color_t PURPLE = {128, 15, 128};
const rgb_color_t BLACK = {0,0,0};
const int COLORS_IN_RAINBOW = 7;
const rgb_color_t RAINBOW[] = {{255, 0, 0}, {255, 127, 0}, {255, 255, 0},
{0, 255, 0}, {0, 0, 255}, {46, 43, 95}, {139, 0, 255}}; // can add {0,0,0} for black
const vector_t PLAYER_VEL = (vector_t){500, 0};
const vector_t BALL_VEL = (vector_t){600, 600};
const size_t NUM_RECT = 4;
const double MASS = INFINITY;
const double MASS_BALL = 20;
const char *PLAYER = "player";
const char *BRICK = "brick";
const char *BALL = "ball";
const char *RANDOM_NAME = "random";
const vector_t BUFFER_BOUND = (vector_t){2, 0.0};
const double BUFF = 5.0;
const double MAX_SCALE = 2.0;
const double MIN_SCALE = 0.25;
// for modes
const double FAST_M = 1.5;
const double SLOW_M = 0.5;
// const size_t MULT_BALLS = 10;

typedef struct info{
  char mode;

}info_t;

info_t *init_info(char mode){
  info_t *info = malloc(sizeof(info_t));
  info->mode = mode;
  return info;
}

void end_game(scene_t * scene){
  scene_free(scene);
  exit(0);
}

// creates a rectangle with given dimensions, color, position, and body info name
body_t *make_brick(vector_t dim, rgb_color_t color, vector_t position, char *name){
  list_t *points = list_init(NUM_RECT, vec_free);
  vector_t *temp1 = malloc(sizeof(vector_t));
  assert(temp1 != NULL);
  *temp1 = (vector_t){dim.x/2,(-1)*dim.y/2}; // lower right
  list_add(points, temp1);
  vector_t *temp2 = malloc(sizeof(vector_t));
  assert(temp2 != NULL);
  *temp2 = vec_multiply(.5, dim); // upper right
  list_add(points, temp2);
  vector_t *temp3 = malloc(sizeof(vector_t));
  assert(temp3 != NULL);
  *temp3 = (vector_t){(-1)*dim.x/2, dim.y/2}; // upper left
  list_add(points, temp3);
  vector_t *temp4 = malloc(sizeof(vector_t));
  assert(temp4 != NULL);
  *temp4 = (vector_t){(-1)*dim.x /2, (-1)*dim.y/2}; // lower left
  list_add(points, temp4);
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, name);
  body_t *brick = body_init_with_info(points, MASS, color, obj_name, free);
  body_set_centroid(brick, position);
  return brick;
}

// set the bricks to hit and the player paddle
void set_bricks(scene_t *scene){
  for(size_t i = 0; i < BRICK_ROWS * BRICK_COLS; i++){
    rgb_color_t cur_color = RAINBOW[((int) i % (int) (BRICK_COLS)) % 7];
    double x_coor = MIN.x + BRICK_DIM.x/2 + ((i % BRICK_COLS) * (BRICK_DIM.x + BRICK_GAP));
    double y_coor = MAX.y - BRICK_DIM.y/2 - ((i % BRICK_ROWS) * (BRICK_DIM.y + BRICK_GAP));
    char *obj_name = malloc(10*sizeof(char));
    strcpy(obj_name, BRICK);
    body_t *brick = make_brick(BRICK_DIM, cur_color, (vector_t){x_coor, y_coor}, obj_name);
    scene_add_body(scene, brick);
    free(obj_name);
  }
}

void make_player(scene_t *scene, vector_t dim){
  char *obj_name = malloc(10*sizeof(char));
  strcpy(obj_name, PLAYER);
  body_t *player = make_brick(dim, BLACK, (vector_t){0, MIN.y + (dim.y / 2)}, obj_name);
  scene_add_body(scene, player);
  free(obj_name);
}

// create red ball on the player's paddle
body_t *make_ball(scene_t *scene){
  list_t *points = list_init(BALL_SIDES + 1, vec_free);
  vector_t general_pt = (vector_t){RADIUS_BALL, 0};
  double rot = (2 * M_PI) / BALL_SIDES;
  for(size_t i = 0; i < BALL_SIDES; i++){
    vector_t *temp = malloc(sizeof(vector_t));
    assert(temp != NULL);
    *temp = vec_rotate(general_pt, (rot * i));
    list_add(points, temp);
  }
  char *ball_name = malloc(10*sizeof(char));
  strcpy(ball_name, BALL);
  body_t *ball = body_init_with_info(points, MASS_BALL, RED, ball_name, free);
  body_set_centroid(ball, (vector_t){0, MIN.y + PLAYER_DIM.y + RADIUS_BALL + BUFF});
  body_set_velocity(ball, BALL_VEL);
  scene_add_body(scene, ball);
  return ball;
}

// remove bodies that are oob -- ONLY BALLS
void out_of_bounds(scene_t *scene){
  int n = 0;
  while(n < scene_bodies(scene)){
    body_t *body = (body_t *)scene_get_body(scene, n);
    if(strcmp(body_get_info(body), BALL) == 0){
      list_t *points = body_get_shape(body);
      for(size_t i = 0; i < list_size(points); i++){
        double y = ((vector_t *)list_get(points, i))->y;
        if(y < MIN.y) {
          scene_remove_body(scene, n);
          n--;
          break;
        }
      }
      list_free(points);
    }
    else if(strcmp(body_get_info(body), PLAYER) == 0){
      list_t *points = body_get_shape(body);
      for(size_t i = 0; i < list_size(points); i++){
        double x = ((vector_t *)list_get(points, i))->x;
        if(x < MIN.x) {
          vector_t player_center = body_get_centroid(body);
          body_set_velocity(body, VEC_ZERO);
          body_set_centroid(body, vec_add(player_center, BUFFER_BOUND));
          break;
        }
        if(x > MAX.x) {
          vector_t player_center = body_get_centroid(body);
          body_set_velocity(body, VEC_ZERO);
          body_set_centroid(body, vec_subtract(player_center, BUFFER_BOUND));
          break;
        }
      }
      list_free(points);
    }
    n++;
  }
}

// reverse x-velocity only when ball reaches edge of screen...could make wall body w/ inf mass
void bounce_wall(scene_t *scene, bool colliding){
  if(!colliding){
    for(size_t i = 0; i < scene_bodies(scene); i++){
      body_t *ball = scene_get_body(scene, i);
      if(strcmp((char *)body_get_info(ball), BALL) == 0){
        vector_t center = body_get_centroid(ball);
        vector_t ball_vel = body_get_velocity(ball);
        if((center.x - RADIUS_BALL) <= MIN.x) {
          body_set_velocity(ball, (vector_t){fabs(ball_vel.x), ball_vel.y});
        }
        if((center.x + RADIUS_BALL) >= MAX.x){
          body_set_velocity(ball, (vector_t){(-1)*fabs(ball_vel.x), ball_vel.y});
        }
        if((center.y + RADIUS_BALL) >= MAX.y) {
          body_set_velocity(ball, (vector_t){ball_vel.x, (-1)*fabs(ball_vel.y)});
        }
      }
    }
  }
}

// have to add impulse btwn ball and all bodies too
// should be added between each brick and the ball
void add_handlers(scene_t *scene){
  // create collisions between ball and bricks, player for impulse--for each ball, check all others for player, bricks--or make bounce off of everything, incl. other balls?
  list_t *bodies = scene_get_all_bodies(scene);
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *ball = (body_t *)list_get(bodies, i);
    if(strcmp(body_get_info(ball), BALL) == 0){
      for(size_t j = 0; j < scene_bodies(scene) && j != i; j++){
        body_t *not_ball = (body_t *)list_get(bodies, j);
        if(strcmp(body_get_info(not_ball), PLAYER) == 0){
          create_physics_collision(scene, ELAS, ball, not_ball, 1);
        }
        if(strcmp(body_get_info(not_ball), BRICK) == 0){
          create_physics_collision(scene, ELAS, ball, not_ball, 2);;
        }
      }
    }
  }
}

// If no bricks OR no balls, return true
bool is_over(scene_t *scene){
  int balls = 0;
  int bricks = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++){
    if(strcmp(body_get_info(scene_get_body(scene, i)), BALL) == 0)
      balls++;
    else if(strcmp(body_get_info(scene_get_body(scene, i)), BRICK) == 0)
      bricks++;
  }
  if(balls == 0 || bricks == 0)
    return true;
  return false;
}

void make_hiding_places(scene_t *scene){
  body_t *player = make_brick((vector_t) {50, 50}, PURPLE, (vector_t) {23, 23}, RANDOM_NAME);
  scene_add_body(scene, player);

  body_t *player1 = make_brick((vector_t) {50, 50}, PURPLE, (vector_t) {123, -23}, RANDOM_NAME);
  scene_add_body(scene, player1);

  body_t *player2 = make_brick((vector_t) {50, 50}, PURPLE, (vector_t) {293, 223}, RANDOM_NAME);
  scene_add_body(scene, player2);

  body_t *player3 = make_brick((vector_t) {50, 50}, PURPLE, (vector_t) {623, -423}, RANDOM_NAME);
  scene_add_body(scene, player3);

  // body_t *make_brick(vector_t dim, rgb_color_t color, vector_t position, char *name){

}

scene_t *set_game(scene_t *scene, char mode){
  scene_free(scene);
  scene_t *n_scene = scene_init();
  if(mode == 'n'){
    make_player(n_scene, PLAYER_DIM);
    set_bricks(n_scene);
    make_ball(n_scene);
  }
  if(mode == 'f'){
    make_player(n_scene, PLAYER_DIM);
    set_bricks(n_scene);
    body_t * ball = make_ball(n_scene);
    body_set_velocity(ball, vec_multiply(FAST_M, BALL_VEL));
  }
  if(mode == 's'){
    make_player(n_scene, PLAYER_DIM);
    set_bricks(n_scene);
    body_t * ball = make_ball(n_scene);
    body_set_velocity(ball, vec_multiply(SLOW_M, BALL_VEL));
  }
  // if(mode == 'm'){
  //   make_player(n_scene, PLAYER_DIM);
  //   set_bricks(n_scene);
  //   for(size_t i = 0; i < MULT_BALLS; i++){
  //     body_t *ball = make_ball(n_scene);
  //     body_set_velocity(ball, vec_add(BALL_VEL, vec_multiply(i, (vector_t){1,1})));
  //   }
  // }
  if(mode == 'b'){
    vector_t mod = (vector_t){2*PLAYER_DIM.x, PLAYER_DIM.y};
    make_player(n_scene, mod);
    set_bricks(n_scene);
    make_ball(n_scene);
  }
  make_hiding_places(n_scene);
  add_handlers(n_scene);
  return n_scene;
}

void key_handle(scene_t *scene, char key, key_event_type_t type, double dt, void *aux){
  body_t *player = scene_get_body(scene, 0);
  vector_t player_center = body_get_centroid(player);
  info_t *info = (info_t *)aux;
  if (type == KEY_PRESSED) {
    switch(key) {
      case LEFT_ARROW:
        if(player_center.x > MIN.x + PLAYER_DIM.x / 2){
          body_set_velocity(player, vec_negate(PLAYER_VEL));
        }
        break;
      case RIGHT_ARROW:
        if(player_center.x < MAX.x - PLAYER_DIM.x / 2){
          body_set_velocity(player, PLAYER_VEL);
        }
        break;
      case ' ':
        break;
      case 'q':
        end_game(scene);
        break;
      // reset cases for new game modes
      case 'n': // normal
        info->mode = 'n';
        break;
      case 'f': // super fast ball
        info->mode = 'f';
        break;
      case 's': // super slow ball
        info->mode = 's';
        break;
      // case 'm':
      //   info->mode = 'm';
      //   break;
      case 'b':
        info->mode = 'b';
        break;
    }
  } else{
    body_set_velocity(player, (vector_t){0, 0});
  }
}

void scale_velocity(scene_t *scene){
  list_t *bodies = scene_get_all_bodies(scene);
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *ball = (body_t *)list_get(bodies, i);
    if(strcmp(body_get_info(ball), BALL) == 0){
      vector_t vel = body_get_velocity(ball);
      double ball_vel = pow(pow(vel.x, 2) + pow(vel.y, 2), 0.5);
      double normal_vel = pow(pow(PLAYER_VEL.x, 2) + pow(PLAYER_VEL.y, 2), 0.5);
      double slow_vel = pow(pow(SLOW_M * PLAYER_VEL.x, 2) + pow(SLOW_M * PLAYER_VEL.y, 2), 0.5);
      if(ball_vel > MAX_SCALE * normal_vel){
        body_set_velocity(ball, vec_multiply(normal_vel/ball_vel, vel));
      }
      if(ball_vel < MIN_SCALE * normal_vel){
        body_set_velocity(ball, vec_multiply(slow_vel/ball_vel, vel));
      }
    }
  }
}

body_t *get_body(scene_t *scene){
  list_t *bodies = scene_get_all_bodies(scene);
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t *ball = (body_t *)list_get(bodies, i);
    if(strcmp(body_get_info(ball), BALL) == 0){
      return ball;
    }
  }
  return NULL;
}

// void make_nodes(){
//   int spacing = 10;
//   int num_rows = (2 * WIN_DIM_Y) % spacing;
//   int num_cols = (2 * WIN_DIM_X) % spacing;
//   for(int i = 0; i < num_rows; i++){
//     for(int j = 0; j < num_cols; j++){
//
//     }
//
//   }
// }
int main(){
  char curr = 'n';
  info_t *mode = init_info(curr);
  scene_t *scene = scene_init();
  sdl_init(MIN, MAX);
  sdl_on_key(key_handle);
  SDL_Texture *mess = message_init();
  sdl_update_zoom(2); // larger value zooms out
  // sdl_update_zoom(.1); // smaller value zooms in
  while(true){
    scene = set_game(scene, curr);
    bool colliding = false;
    body_t *ball = get_body(scene);
    while (!sdl_is_done(scene, mode)){
      message(mess);
      double dt = time_since_last_tick();
      bounce_wall(scene, colliding);
      out_of_bounds(scene);
      scale_velocity(scene);
      if(is_over(scene)){
        break;
      }
      if(mode->mode != curr){
        curr = mode->mode;
        break;
      }
      scene_tick(scene, dt);
      sdl_update_center(body_get_centroid(ball));
      sdl_render_scene(scene);
    }
    message_free();
  }
}
