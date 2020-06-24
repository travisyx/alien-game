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

// A node that builts off of object_t, for pathfinding purposes.
 typedef struct node {
   object_t *node;
   double priority;
   double traveled;
   struct node *neighbors[8];
   double distances[8];
   bool visited;
   size_t num_neighbors;
 } node_t;

 ////////////////////// Some methods to make backing array easier to access--could abstract this out if wanted?

/**
 * Initializes the backing array with a certain width, height, and free function.
 *
 * @param height the height of the array
 * @param width the width of the array
 * @param freer the free function
 * @return the initialized array
 */
 list_t *arr_init(int height, int width, free_func_t freer);

 /**
  * Returns a vector_t corresponding to the original array size
  *
  * @param arr the backing array
  * @return a vector_t corresponding to the size of the array
  */
 vector_t arr_size(list_t *arr);

 /**
  * Gets a specified element of the backing array (row, column)
  *
  * @param arr the backing array
  * @param r the row of the desired element
  * @param c the column of the desired element
  * @return the element in the backing array
  */
 void *arr_get(list_t *arr, int r, int c);


 /**
  * Replaces the element at the array with new_obj. Assumes an element in the array
  *
  * @param arr the backing array
  * @param r the row of the desired element
  * @param c the column of the desired element
  * @param new_obj the element to be placed in the backing array
  * @return the old element in the backing array
  */
  void *arr_put(list_t *arr, int r, int c, void *new_obj);

 /**
  * Returns the center of the corresponding square (in pixel representation)
  *
  * @param arr the backing array
  * @param r the row of the desired element
  * @param c the column of the desired element
  * @return the coordinates of the pixel representation
  */
 vector_t map_pos_from_ind(map_t *map, int r, int c);

 /**
  * Returns the map index of the pixel
  *
  * @param arr the backing array
  * @param position the position in pixel representation
  * @return the position of the backing array element
  */
vector_t map_ind_from_pos(map_t *map, vector_t position);

  /**
   * Initializes a node with a specified object_t and priority
   *
   * @param node the object
   * @param priority the priority
   * @return the initialized node
   */
 node_t *node_init(object_t *node, double priority);

 /**
  * Initializes the map with its instance variables
  *
  * @return the initialized map
  */
 map_t *map_init();

 /**
  * Initializes the player
  *
  * @return the player
  */
 body_t *make_player();

 /**
  * Initializes the alien
  *
  * @return the alien
  */
 body_t *make_alien();

 /**
  * Frees the map and its instance variables
  *
  * @param map the map to be freed
  */
 void map_free(map_t *map);

  /**
   * Populates the lists with the desired quantities. Should only be called at the beginning
   *
   * @param map the map with the instance variables to be initialized
   */
 void populate_lists(map_t *map);

 /**
  * Collects a coin from the map
  *
  * @param map the map
  */
 void map_collect_coin(map_t *map);

 /**
  * Determines if the player has enough currency to buy the desired item
  *
  * @param map the map
  * @param cost the cost of the desired item
  * @return a boolean that is true if the player has enough coins, false otherwise
  */
 bool spend_money(map_t *map, int cost);

 /**
  * Teleports the player to the center of the hiding spot
  *
  * @param map the map
  */
 void map_hide_player(map_t *map);

 /**
  * Removes the player from the hiding spot
  *
  * @param map the map
  */
 void map_unhide_player(map_t *map);

 /**
  * Determines if the player is hiding or not
  *
  * @param map the map
  * @return a boolean that is true if the player is hiding, false otherwise
  */
 bool is_hiding(map_t *map);

 /**
  * Opens the door if the player has enough coins
  *
  * @param map the map
  */
 void open_door(map_t *map);

 /**
  * Tests if the door is open and the player is colliding with it
  *
  * @param map the map
  * @return a boolean that is true if the above conditions are met, false otherwise
  */
 bool map_win(map_t *map);

 /**
  * Tests if the player and alien are colliding
  *
  * @param map the map
  * @return a boolean that is true if the above condition is met, false otherwise
  */
 bool map_lose(map_t *map);

 /**
  * Tests if two objects are colliding using collision.c framework and
  * precalculated min and maxes for bounding box.
  *
  * @param o1 the first object to be tested
  * @param o2 the second object to be tested
  * @return a boolean true if the objects are colliding, false otherwise
  */
 bool object_collision(object_t *o1, object_t *o2);

 /**
  * Disallows a player from being where they shouldn't be
  *
  * @param map the map
  */
 void bounce(map_t *map);

 /////////////////////////////////

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
