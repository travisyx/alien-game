#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include <stdbool.h>
#include "color.h"
#include "list.h"
// #include "map.h"
#include "scene.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
// for text rendering
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
// https://www.libsdl.org/projects/SDL_ttf/

// Values passed to a key handler when the given arrow key is pressed
#define LEFT_ARROW 1
#define UP_ARROW 2
#define RIGHT_ARROW 3
#define DOWN_ARROW 4

void sdl_update_zoom(double zoom_n);

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(scene_t *scene, char key, key_event_type_t type,
                              double held_time, void *aux);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(scene_t *scene, void *aux);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

void sdl_update_center(vector_t center);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

// create rectangle for text rendering
void make_rect(int x, int y, int w, int h);

// initialize TTF and fonts
void message_init();

void update_bullets(int bullets);

// renders the player image at a given angle
void render_player_image(double angle);

// updates text when money is collected
void update_money(int money);

// updates stamina when the player's stamina changes
void update_stamina(int stamina);

// updates the old text when money or stamina changes
void replace_text();

// adds texture to renderer to display text
void render_text(int x, int y);

// frees or closes surfaces, textures, fonts, and TTF
void message_free();

// create image surfaces and textures
void images_init();

// create surface, texture, and rectangle for player image
void create_image();

// adds image to renderer and rotates image clockwise based on angle
// void render_image(double angle);
void render_image();

// adds image of alien
void render_alien_image(vector_t location);

// adds image of lockers
void render_locker_image(vector_t location);

// adds image of dumpster
void render_dumpster_image(vector_t location);

// adds image of coin
void render_coin_image(vector_t location);

// adds image of wall
void render_wall_image(vector_t location);

// adds image of door
void render_door_image(vector_t location);

// frees image texture
void image_free();

// prints win message. yay!
void win_message();

// prints lose message. try again!
void lose_message();

#endif // #ifndef __SDL_WRAPPER_H__
