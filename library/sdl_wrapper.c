#include "sdl_wrapper.h"

const char WINDOW_TITLE[] = "CS 3";
// square!!
const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 1500;
const double MS_PER_S = 1e3;
double zoom;
const char *FREE_SANS = "fonts/FreeSans.ttf";
const char *OPEN_SANS_LIGHT = "fonts/open-sans/OpenSans-Light.ttf";
const int FONT_SIZE = 50;
TTF_Font *FONT;
const SDL_Color FONT_BLACK = {0, 0, 0};
const SDL_Color FONT_BLUE = {19,178,214};
const SDL_Color TILT_RED = {255,0,0};
const SDL_Color VICTORY_GREEN = {184,19,214};
const char *PLAYER_IMAGE = "images/adam.jpg";
const int PLAYER_X = 697;
const int PLAYER_Y = 375;
const int PLAYER_W = 45;
const int PLAYER_H = 45;

const char *ALIEN_IMAGE = "images/alien.jpeg";
const double ALIEN_RADIUS = 22.5;
const int ALIEN_W = 45;
const int ALIEN_H = 45;

const char *LOCKER_IMAGE = "images/locker.jpeg";
const double LOCKER_RADIUS = 28.125;

const char *DUMPSTER_IMAGE = "images/dumpster.jpeg";
const double DUMPSTER_RADIUS = 28.125;

const char *COIN_IMAGE = "images/coin.png";
const double COIN_RADIUS = 8.375;

const char *WALL_IMAGE = "images/wall.jpeg";
const double WALL_RADIUS = 28.125;

const char *DOOR_IMAGE = "images/door.jpeg";
const double DOOR_RADIUS = 28.125;

SDL_Surface *SURFACE;
SDL_Texture *TEXTURE;
SDL_Rect RECT;
char MONEY[20];
char STAMINA[10];
char BULLETS[10];

SDL_Surface *SURFACE_PLAYER;
SDL_Texture *TEXTURE_PLAYER;
SDL_Rect RECT_PLAYER;

SDL_Surface *SURFACE_ALIEN;
SDL_Texture *TEXTURE_ALIEN;
SDL_Rect RECT_ALIEN;

SDL_Surface *SURFACE_LOCKER;
SDL_Texture *TEXTURE_LOCKER;
SDL_Rect RECT_LOCKER;

SDL_Surface *SURFACE_DUMPSTER;
SDL_Texture *TEXTURE_DUMPSTER;
SDL_Rect RECT_DUMPSTER;

SDL_Surface *SURFACE_COIN;
SDL_Texture *TEXTURE_COIN;

SDL_Surface *SURFACE_WALL;
SDL_Texture *TEXTURE_WALL;

SDL_Surface *SURFACE_DOOR;
SDL_Texture *TEXTURE_DOOR;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

// ///////

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center() {
    int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x, y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                      // Flip y axis since positive y is down on the screen
                      .y = round(window_center.y - pixel_center_offset.y)};
    return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:
            return LEFT_ARROW;
        case SDLK_UP:
            return UP_ARROW;
        case SDLK_RIGHT:
            return RIGHT_ARROW;
        case SDLK_DOWN:
            return DOWN_ARROW;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode)(char)key ? key : '\0';
    }
}

void sdl_update_center(vector_t center_n){
  center = center_n;
}

void sdl_update_zoom(double zoom_n){
  zoom = zoom_n;
  max_diff = vec_multiply(zoom, max_diff);
}

void sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);
    zoom = 1;
    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_multiply(zoom, vec_subtract(max, center));
    SDL_Init(SDL_INIT_EVERYTHING);
    window =
        SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);
    MONEY[0] = '$'; // *MONEY = "$0";
    MONEY[1] = '0';
    STAMINA[0] = '1'; // *STAMINA = "100";
    STAMINA[1] = '0';
    STAMINA[2] = '0';
    BULLETS[0] = '5'; // *BULLETS = "5";
}

bool sdl_is_done(scene_t *scene, void *aux) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (key_handler == NULL) break;
                char key = get_keycode(event->key.keysym.sym);
                if (key == '\0') break;

                uint32_t timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                key_event_type_t type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(scene, key, type, held_time, aux);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 255);
    assert(0 <= color.g && color.g <= 255);
    assert(0 <= color.b && color.b <= 255);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 1, color.g * 1,
                      color.b * 1, 255);
    free(x_points);
    free(y_points);
}

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

// stop obj rendering here if you want to put img instead
void sdl_render_scene(scene_t *scene) {
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        if(strcmp(body_get_info(body), "player") != 0 && strcmp(body_get_info(body), "alien") != 0 && strcmp(body_get_info(body), "dumpster") != 0 && strcmp(body_get_info(body), "locker") != 0 && strcmp(body_get_info(body), "coin") != 0 && strcmp(body_get_info(body), "wall") != 0 && strcmp(body_get_info(body), "door") != 0){
        // if(strcmp(body_get_info(body), "bullet") == 0){
          list_t *shape = body_get_shape(body);
          sdl_draw_polygon(shape, body_get_color(body));
          list_free(shape);
        }
    }
    sdl_show();
}

void sdl_on_key(key_handler_t handler) {
    key_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock ? (double)(now - last_clock) / CLOCKS_PER_SEC
                                   : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

void make_rect(int x, int y, int w, int h){
  RECT.x = x;  //controls the rect's x coordinate
  RECT.y = y; // controls the rect's y coordinte
  RECT.w = w; // controls the width of the rect
  RECT.h = h; // controls the height of the rect
}

void message_init(){
  TTF_Init(); // initialize TTF
  FONT = TTF_OpenFont(OPEN_SANS_LIGHT, FONT_SIZE); // opens font style and sets size
}

// assuming that rect does not change places or size
void replace_text(){
  char *money_label = "MONEY: ";
  char *stamina_label = " . STAMINA: ";
  char *bullets_label = " . BULLETS: ";
  char text[strlen(money_label) + strlen(MONEY) + strlen(stamina_label) + strlen(STAMINA) + strlen(bullets_label) + strlen(BULLETS) + 1];
  strcpy(text, money_label);
  strcat(text, MONEY);
  strcat(text, stamina_label);
  strcat(text, STAMINA);
  strcat(text, bullets_label);
  strcat(text, BULLETS);
  assert(text != NULL);
  if(SURFACE != NULL){    // free any existing surface
    SDL_FreeSurface(SURFACE);
  }
  SURFACE = TTF_RenderText_Blended(FONT, text, FONT_BLUE); // creates surface from char
  if(TEXTURE != NULL){   // free any existing texture
    SDL_DestroyTexture(TEXTURE);
  }
  TEXTURE = SDL_CreateTextureFromSurface(renderer, SURFACE);  // creates texture from surface
}

// pass in purse money value
void update_money(int money){
  snprintf(MONEY, 20, "$%d", money);
  replace_text();
}

// pass in % stamina left: 100 = 100, 50 = 0
void update_stamina(int stamina){
  snprintf(STAMINA, 10, "%d%%", stamina);
  replace_text();
}

void update_bullets(int bullets){
  snprintf(BULLETS, 10, "%d", bullets);
  replace_text();
}

void render_text(int x, int y){
  int width = 0;
  int height = 0;
  SDL_QueryTexture(TEXTURE, NULL, NULL, &width, &height); // set rectange to dimension of text
  make_rect(x, y, width, height);
  SDL_RenderCopy(renderer, TEXTURE, NULL, &RECT); // show gameloop there is text that needs to be rendered
}

void message_free(){
  TTF_CloseFont(FONT);
  TTF_Quit();
}

void images_init(){
  SURFACE_PLAYER = IMG_Load(PLAYER_IMAGE);
  TEXTURE_PLAYER = SDL_CreateTextureFromSurface(renderer, SURFACE_PLAYER);

  SURFACE_ALIEN = IMG_Load(ALIEN_IMAGE);;
  TEXTURE_ALIEN = SDL_CreateTextureFromSurface(renderer, SURFACE_ALIEN);;

  SURFACE_LOCKER = IMG_Load(LOCKER_IMAGE);
  TEXTURE_LOCKER = SDL_CreateTextureFromSurface(renderer, SURFACE_LOCKER);

  SURFACE_DUMPSTER = IMG_Load(DUMPSTER_IMAGE);
  TEXTURE_DUMPSTER = SDL_CreateTextureFromSurface(renderer, SURFACE_DUMPSTER);

  SURFACE_COIN = IMG_Load(COIN_IMAGE);
  TEXTURE_COIN = SDL_CreateTextureFromSurface(renderer, SURFACE_COIN);

  SURFACE_WALL = IMG_Load(WALL_IMAGE);
  TEXTURE_WALL = SDL_CreateTextureFromSurface(renderer, SURFACE_WALL);

  SURFACE_DOOR = IMG_Load(DOOR_IMAGE);
  TEXTURE_DOOR = SDL_CreateTextureFromSurface(renderer, SURFACE_DOOR);

}

void render_player_image(double angle){
  RECT_PLAYER.x = get_window_center().x - PLAYER_W / 2;
  RECT_PLAYER.y = get_window_center().y - PLAYER_H / 2;
  RECT_PLAYER.w = PLAYER_W;
  RECT_PLAYER.h = PLAYER_H;
  SDL_RenderCopyEx(renderer, TEXTURE_PLAYER, NULL, &RECT_PLAYER, angle, NULL, SDL_FLIP_NONE);
}

void render_alien_image(vector_t location){
  vector_t alien_pos = get_window_position(location, get_window_center());
  RECT_ALIEN.x = (alien_pos.x) - ALIEN_RADIUS;
  RECT_ALIEN.y = (alien_pos.y) - ALIEN_RADIUS;
  RECT_ALIEN.w = ALIEN_W;
  RECT_ALIEN.h = ALIEN_H;
  SDL_RenderCopy(renderer, TEXTURE_ALIEN, NULL, &RECT_ALIEN);
}

void render_locker_image(vector_t location){
  vector_t locker_pos = get_window_position(location, get_window_center());
  RECT_LOCKER.x = (locker_pos.x) - LOCKER_RADIUS;
  RECT_LOCKER.y = (locker_pos.y) - LOCKER_RADIUS;
  RECT_LOCKER.w = LOCKER_RADIUS * 2;
  RECT_LOCKER.h = LOCKER_RADIUS * 2;
  SDL_RenderCopy(renderer, TEXTURE_LOCKER, NULL, &RECT_LOCKER);
}

void render_dumpster_image(vector_t location){
  vector_t dumpster_pos = get_window_position(location, get_window_center());
  RECT_DUMPSTER.x = (dumpster_pos.x) - DUMPSTER_RADIUS;
  RECT_DUMPSTER.y = (dumpster_pos.y) - DUMPSTER_RADIUS;
  RECT_DUMPSTER.w = DUMPSTER_RADIUS * 2;
  RECT_DUMPSTER.h = DUMPSTER_RADIUS * 2;
  SDL_RenderCopy(renderer, TEXTURE_DUMPSTER, NULL, &RECT_DUMPSTER);
}

void render_coin_image(vector_t location){
  vector_t coin_pos = get_window_position(location, get_window_center());
  SDL_Rect rect_coin;
  rect_coin.x = (coin_pos.x) - COIN_RADIUS;
  rect_coin.y = (coin_pos.y) - COIN_RADIUS;
  rect_coin.w = COIN_RADIUS * 2;
  rect_coin.h = COIN_RADIUS * 2;
  SDL_RenderCopy(renderer, TEXTURE_COIN, NULL, &rect_coin);
}

void render_wall_image(vector_t location){
  vector_t wall_pos = get_window_position(location, get_window_center());
  SDL_Rect rect_wall;
  rect_wall.x = (wall_pos.x) - WALL_RADIUS;
  rect_wall.y = (wall_pos.y) - WALL_RADIUS;
  rect_wall.w = WALL_RADIUS * 2;
  rect_wall.h = WALL_RADIUS * 2;
  SDL_RenderCopy(renderer, TEXTURE_WALL, NULL, &rect_wall);
}

void render_door_image(vector_t location){
  vector_t door_pos = get_window_position(location, get_window_center());
  SDL_Rect rect_door;
  rect_door.x = (door_pos.x) - DOOR_RADIUS;
  rect_door.y = (door_pos.y) - DOOR_RADIUS;
  rect_door.w = DOOR_RADIUS * 2;
  rect_door.h = DOOR_RADIUS * 2;
  SDL_RenderCopy(renderer, TEXTURE_DOOR, NULL, &rect_door);
}

void image_free(){
  SDL_FreeSurface(SURFACE_PLAYER);
  SDL_DestroyTexture(TEXTURE_PLAYER);
  SDL_FreeSurface(SURFACE_ALIEN);
  SDL_DestroyTexture(TEXTURE_ALIEN);
  SDL_FreeSurface(SURFACE_LOCKER);
  SDL_DestroyTexture(TEXTURE_LOCKER);
  SDL_FreeSurface(SURFACE_DUMPSTER);
  SDL_DestroyTexture(TEXTURE_DUMPSTER);
  SDL_FreeSurface(SURFACE_COIN);
  SDL_DestroyTexture(TEXTURE_COIN);
  SDL_FreeSurface(SURFACE_WALL);
  SDL_DestroyTexture(TEXTURE_WALL);
  SDL_FreeSurface(SURFACE_DOOR);
  SDL_DestroyTexture(TEXTURE_DOOR);
}

void win_message(){
  char text[30];
  strcpy(text, "YOU SURVIVED!");
  if(SURFACE != NULL){    // free any existing surface
    SDL_FreeSurface(SURFACE);
  }
  SURFACE = TTF_RenderText_Blended(FONT, text, VICTORY_GREEN); // creates surface from char
  if(TEXTURE != NULL){   // free any existing texture
    SDL_DestroyTexture(TEXTURE);
  }
  TEXTURE = SDL_CreateTextureFromSurface(renderer, SURFACE);  // creates texture from surface
  render_text(0,0);
}

void lose_message(){
  char text[30];
  strcpy(text, "You died...get good.");
  if(SURFACE != NULL){    // free any existing surface
    SDL_FreeSurface(SURFACE);
  }
  SURFACE = TTF_RenderText_Blended(FONT, text, TILT_RED); // creates surface from char
  if(TEXTURE != NULL){   // free any existing texture
    SDL_DestroyTexture(TEXTURE);
  }
  TEXTURE = SDL_CreateTextureFromSurface(renderer, SURFACE);  // creates texture from surface
  render_text(0,0);
}
