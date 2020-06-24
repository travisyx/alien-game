#ifndef __MAP_H__
#define __MAP_H__

#include "scene.h"
#include "body.h"
#include "list.h"
#include "object.h"
#include "collision.h"
#include "sdl_wrapper.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// generally will not redo scene methods--to do that, get the scene and then
// access that way! only adding on methods that will be added functionality for
// a map.

/**
 * A scene that is organized into player, alien, walls, doors, hiding spots.
 * Intended to allow for more intuitive use of scene for our game. Declared here
 * because we're not particularly concerned with outside access.
 */
 typedef struct map {
     scene_t *scene;
     // list of lists = 2D array. should not ever be forced to resize.
     // big list = all rows, for each row have list w/ size num cols
     list_t *backing_array;
     object_t *player;
     int purse;
     object_t *alien;
     list_t *walls;
     list_t *doors;
     list_t *coins;
     list_t *hiding_spots;
     list_t *nodes;
     list_t *struct_nodes;
 } map_t;

 typedef struct node {
   object_t *node;
   double priority;
   double traveled;
   struct node *neighbors[8];
   double distances[8];
   bool visited;
   size_t num_neighbors;
 } node_t;

 ////////////////////// some methods to make backing array easier to access--could abstract this out if wanted?

 list_t *arr_init(int height, int width, free_func_t freer);

 vector_t arr_size(list_t *arr);

 void *arr_get(list_t *arr, int r, int c);

 // assumes that array is already full/there is something at this position already
 // returns the old object at this position
 void *arr_put(list_t *arr, int r, int c, void *new_obj);

 // returns body at location and puts a node in its place
// void *arr_remove(list_t *arr, int r, int c);


 // return the center of the square rep by given index
 vector_t map_pos_from_ind(map_t *map, int r, int c);

 // position given in x,y where 0,0 is top left, 0, width is top right
 // returns in row, col
 vector_t map_ind_from_pos(map_t *map, vector_t position);

 //////////////////////////////////////////////////

 node_t *node_init(object_t *node, double priority);

 map_t *map_init();

 // triangle so can tell which direction facing. will start pointing to the left
 body_t *make_player();

 body_t *make_alien();

 void map_free(map_t *map);

 // should only be called at the beginning. assumes that objects at this time are
 // staying in place and not being removed/added i.e. FINAL state of map.
 void populate_lists(map_t *map);

 /////////////////////////////////////////////////

 // when player collides with coin, remove the coin and add value to player's purse
 void map_collect_coin(map_t *map);

 // true if transaction can go through. false if not enough money.
 // if true, will take money out of purse
 bool spend_money(map_t *map, int cost);

 // touch you buy..spends money here
 // will just leave player on top of hiding spot. considered hiding if centroid still in
 // once collides, transports player centroid onto hiding spot centroid
 void map_hide_player(map_t *map);

 void map_unhide_player(map_t *map);

 bool is_hiding(map_t *map);

 void open_door(map_t *map);

 // if player is colliding with door and can spend money, returns yes to win
 bool map_win(map_t *map);

 // checks if player and alien colliding, if yes, then lose.
 bool map_lose(map_t *map);

 /////////////// collisions /////////////////////////
 bool object_collision(object_t *o1, object_t *o2);

 // don't let player go into objects it shouldn't be able to go into
 void bounce(map_t *map);

 ////////////////////////////////////////////////

 // some specialized methods for adding specific types

 // makes box at pos 0, no info. you can set centroid and info yourself. has generic 10*10 size
 body_t *make_box(rgb_color_t color);

 object_t *map_make_node(map_t *map);

 // should fill all spots in 2d array with a node. only to be used at start.
 // nodes are lowest priority -- will be replaced by anything else being placed at spot
 void map_add_nodes(map_t *map);

 // get rid of node at particular spot. only helper func.
 // assumes new item is already in scene. makes into object that belongs to backing array
 // DOES NOT ACCOUNT FOR NODE LIST. SHOULD ONLY BE USED BEFORE THAT LIST IS CREATED.
 void map_replace_node(map_t *map, int r, int c, object_t *new_ob);

 object_t *map_make_wall(map_t *map);

 // create the wall when called in map_add_walls
 void create_walls(map_t *map, size_t r, size_t c);

 // need to add outline first. then some sort of design--way to rand generate or just make array of spots filled?
 void map_add_walls(map_t *map);

 // replace wall..probably only used to place doors.
  void map_replace_wall(map_t *map, int r, int c, object_t *new_ob);

 // add door to spot in 2d array, get rid of wall if already there. do nothing if
 object_t *map_make_door(map_t *map);

 // spawn randomly on perimeter, or at set locations? should also assign one to be winning doors, and assign costs (random or set?)
 // scratch that...just two doors that both are win
 // door 0 is left, 1 is right
 void map_add_doors(map_t *map);

 body_t *map_make_coin(map_t *map);

 // could have respawn at interval? perhaps only spawn outside player radius
 // should NOT replace node
 void map_add_coins(map_t *map);

 // unid hiding spot..to be assigned later
 object_t *map_make_hiding_spot(map_t *map, int ind);

 void map_add_hiding_spots(map_t *map);

 /////////////////////////////////

 // should call scene tick, and also collect_coin, lose, etc.
 void map_tick(map_t *map, double dt);

#endif // #ifndef __SCENE_H__
