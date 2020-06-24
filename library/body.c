#include <math.h>
#include "body.h"

const int NUM_FORCES = 50;
const int NUM_IMP = 5;
const double AVG = .5;

typedef struct body {
    polygon_t *shape;
    vector_t velocity;
    rgb_color_t color;
    double orientation;
    double mass;
    list_t *forces;
    list_t *impulses;
    void *info;
    bool has_info;
    free_func_t freer;
    bool rem;
    vector_t centroid;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);
    body->shape = polygon_init(list_size(shape));
    polygon_set_points(body->shape, shape);
    body->velocity = (vector_t){0.0, 0.0};
    body->color = color;
    body->mass = mass;
    body->orientation = 0;
    body->forces = list_init(NUM_FORCES, vec_free);
    body->impulses = list_init(NUM_IMP, vec_free);
    body->rem = false;
    body->centroid = polygon_centroid(body->shape);
    body->has_info = false;
    return body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color, void *aux, free_func_t freer) {
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);
    body->shape = polygon_init(list_size(shape));
    polygon_set_points(body->shape, shape);
    body->velocity = (vector_t){0.0, 0.0};
    body->color = color;
    body->mass = mass;
    body->orientation = 0;
    body->forces = list_init(NUM_FORCES, vec_free);
    body->impulses = list_init(NUM_IMP, vec_free);
    body->rem = false;
    body->has_info = false;
    body_put_info(body, aux, freer);
    body->centroid = polygon_centroid(body->shape);
    return body;
}

void body_put_info(void *body, void *aux, free_func_t freer){
  if(((body_t *)body)->has_info){
    ((body_t *)body)->freer(body_get_info(body));
  }
  ((body_t *)body)->info = aux;
  ((body_t *)body)->has_info = true;
  ((body_t *)body)->freer = freer;
}

void *body_get_info(body_t *body){
  if(body->has_info){
    return body->info;
  }
  return NULL;
}

void body_free(void *body) {
    polygon_free(((body_t *)body)->shape);
    list_free(((body_t *)body)->forces);
    list_free(((body_t *)body)->impulses);
    if(((body_t *)body)->has_info){
      ((body_t *)body)->freer(body_get_info(body));
    }
    free(((body_t *)body));
}

double body_get_mass(body_t *body){
  return body->mass;
}

list_t *body_get_shape(body_t *body) {
    list_t *points = polygon_get_points(body->shape);
    size_t size = list_size(points);
    list_t *copy = list_init(size, vec_free);
    for (size_t i = 0; i < size; i++) {
        vector_t *temp = malloc(sizeof(vector_t));
        *temp = *(vector_t *)list_get(points, i);
        list_add(copy, temp);
    }
    return copy;
}

vector_t body_get_centroid(body_t *body) {
    // return polygon_centroid(body->shape);
    return body->centroid;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void body_set_color(body_t *body, rgb_color_t color) {
    body->color = color;
}

void body_set_centroid(body_t *body, vector_t x) {
    polygon_translate(body->shape, vec_subtract(x, body->centroid));
    body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_rotation(body_t *body, double angle) {
    polygon_rotate(body->shape, angle - body->orientation, body_get_centroid(body));
    body->orientation = angle;
}

// Deprecated
void body_translate(body_t *body, vector_t diff) {
    polygon_translate(body->shape, diff);
}

void body_add_force(body_t *body, vector_t *force) {
  list_add(body->forces, force);
}

void body_add_impulse(body_t *body, vector_t *impulse) {
  list_add(body->impulses, impulse);
}

void body_tick(body_t *body, double dt) {
    vector_t velocity = body->velocity;
    vector_t vel_new = body->velocity;
    // handle forces
    for(size_t i = 0; i < list_size(body->forces); i++){
      vector_t force = *(vector_t *)list_get(body->forces, i);
      // f = ma
      vector_t accel = vec_multiply(1.0 / body->mass, force);
      vector_t dv = vec_multiply(dt, accel);
      vel_new = vec_add(vel_new, dv);
    }
    // handle impulses by just adding imp/mass
    for(size_t i = 0; i < list_size(body->impulses); i++){
      vel_new = vec_add(vel_new, vec_multiply(1 / body->mass, *(vector_t *)list_get(body->impulses, i)));
    }
    vector_t vel_avg = vec_multiply(AVG, vec_add(velocity, vel_new));
    vector_t trans = (vector_t)vec_multiply(dt, vel_avg);
    vector_t new_c = vec_add(body->centroid, trans);
    body_set_centroid(body, new_c);
    body_set_velocity(body, vel_new);
    list_clear(body->forces);
    list_clear(body->impulses);
}

void body_remove(body_t *body){
  body->rem = true;
}

bool body_is_removed(body_t *body){
  return body->rem;
}
