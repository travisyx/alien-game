#include "forces.h"

const double BUFFER = .01;
const double SPRING_EQUI = 0;
const double SQRT = .5;
const double DRAG_REGEN = .2;

aux_t *aux_init(list_t *bodies, double constant){
  aux_t *ans = malloc(sizeof(aux_t));
  assert(ans != NULL);
  ans->bodies = bodies;
  ans->constant = constant;
  ans->collided = false;
  ans->aux = NULL;
  ans->handler = NULL;
  ans->freer = NULL;
  return ans;
}

void *aux_get_aux(aux_t *aux){
  return aux->aux;
}

void aux_set_handler(aux_t *aux, collision_handler_t func){
  aux->handler = func;
}

void aux_set_aux(aux_t *aux, void *info){
  aux->aux = info;
}

void aux_set_freer(aux_t *aux, free_func_t freer){
  aux->freer = freer;
}

void aux_ception_free(void *aux){
  list_free(((aux_t *)aux)->bodies);
  free(aux);
}

void aux_free(void *ans){
  aux_t *aux = (aux_t *)ans;
  list_free(aux->bodies);
  // Assumes that if no func, don't want anything freed.
  if(aux->aux == NULL){
    free_nothing(aux->aux);
  }
  else if(aux->freer == NULL){
    free_nothing(aux->aux);
  }
  else{
    free_func_t freer = aux->freer;
    freer(aux->aux);
  }
  free(aux);
}

void free_nothing(void *thing){
  return;
}

void gravity(void *aux){
  aux_t *data = (aux_t *) aux;
  body_t *body_1 = (body_t *) list_get(data->bodies, 0);
  body_t *body_2 = (body_t *) list_get(data->bodies, 1);
  vector_t b1 = body_get_centroid(body_1);
  vector_t b2 = body_get_centroid(body_2);
  double G = data->constant;
  double distance = pow(pow(b2.x-b1.x,2) + pow(b2.y-b1.y,2), SQRT);
  if(distance == 0.0){
    return;
  }
  double grav_magnitude = 0.0;
  vector_t diff = vec_subtract(b2, b1);
  diff = vec_multiply(1 / distance, diff);
  if(distance > BUFFER){
    grav_magnitude = G * body_get_mass(body_1) * body_get_mass(body_2) / pow(distance, 2);
  }
  diff = vec_multiply(grav_magnitude, diff);
  vector_t *gravity = malloc(sizeof(vector_t));
  *gravity = diff;
  vector_t *neg_gravity = malloc(sizeof(vector_t));
  *neg_gravity = vec_negate(*gravity);
  body_add_force(body_1, gravity);
  body_add_force(body_2, neg_gravity);
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(1, (free_func_t)(free_nothing));
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(bodies, G);
  scene_add_bodies_force_creator(scene, (force_creator_t)gravity, aux, bodies, (free_func_t)aux_free);
}

void spring(void *aux){
  aux_t *data = (aux_t *) aux;
  body_t *body_1 = (body_t *) list_get(data->bodies, 0);
  body_t *body_2 = (body_t *) list_get(data->bodies, 1);
  vector_t b1 = body_get_centroid(body_1);
  vector_t b2 = body_get_centroid(body_2);
  double k = data->constant;
  // Positive if b1 is higher than b2
  vector_t *force_on_one = malloc(sizeof(vector_t));
  *force_on_one = (vector_t){-k*(b1.x - b2.x), -k*(b1.y - b2.y)};
  vector_t *force_on_two = malloc(sizeof(vector_t));
  *force_on_two = vec_negate(*force_on_one);
  body_add_force(body_1, force_on_one);
  body_add_force(body_2, force_on_two);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(1, (free_func_t)(free_nothing));
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(bodies, k);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring, aux, bodies, (free_func_t)aux_free);
}

void drag(void *aux) {
  // id = 1
  aux_t *data = (aux_t *)aux;
  body_t *body = (body_t *)list_get(data->bodies, 0);
  double gamma = data->constant;
  vector_t velocity = body_get_velocity(body);
  vector_t *force = malloc(sizeof(vector_t));
  *force = vec_multiply(gamma, vec_negate(velocity));
  body_add_force(body, force);
}

void drag_fade(void *aux) {
  // id = 2, drag will fade to gamma = 1
  aux_t *data = (aux_t *)aux;
  if(data->constant > 0){
    data->constant = data->constant - DRAG_REGEN;
  }
  else{
    return;
  }
  body_t *body = (body_t *)list_get(data->bodies, 0);
  double gamma = data->constant;
  vector_t velocity = body_get_velocity(body);
  vector_t *force = malloc(sizeof(vector_t));
  *force = vec_multiply(gamma, vec_negate(velocity));
  body_add_force(body, force);
}

void create_drag(scene_t *scene, double gamma, body_t *body, int id) {
  list_t *bodies = list_init(1, (free_func_t)(free_nothing));
  list_add(bodies, body);
  aux_t *aux = aux_init(bodies, gamma);
  if(id == 1){
    scene_add_bodies_force_creator(scene, (force_creator_t)drag, aux, bodies, (free_func_t)aux_free);
  }
  else if(id == 2){
    scene_add_bodies_force_creator(scene, (force_creator_t)drag_fade, aux, bodies, (free_func_t)aux_free);
  }
}

void collision(void *aux){
  // This is a "force_creator_t", will be called each tick as such
  aux_t *data = (aux_t *) aux; //
  collision_handler_t func = data->handler;
  body_t *body1 = (body_t *) list_get(data->bodies, 0);
  body_t *body2 = (body_t *) list_get(data->bodies, 1);
  list_t *pts1 = body_get_shape(body1);
  list_t *pts2 = body_get_shape(body2);
  collision_info_t coll = find_collision(pts1, pts2);
  list_free(pts1);
  list_free(pts2);
  if(coll.collided){
    if(data->collided == false){
      data->collided = true;
      vector_t axis = coll.axis;
      func(body1, body2, axis, data->aux);
    }
  }
  else{
    data->collided = false;
  }
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2, collision_handler_t handler, void *aux, free_func_t freer){
  list_t *bodies = list_init(2, (free_func_t)(free_nothing));
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *new_aux = aux_init(bodies, 0);
  aux_set_aux(new_aux, aux);
  aux_set_handler(new_aux, handler);
  aux_set_freer(new_aux, freer);
  scene_add_bodies_force_creator(scene, (force_creator_t)collision, new_aux, bodies, (free_func_t)aux_free);
}

void destroy(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // id = 2
  body_remove(body1);
  body_remove(body2);
}

void destroy_one(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // id = 1, destroys first body put in when collides
  body_remove(body1);
}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2, int id){
  list_t *bodies = list_init(2, (free_func_t)(free_nothing));
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(bodies, 0);
  if(id == 1){
    create_collision(scene, body1, body2, (collision_handler_t)destroy_one, aux, aux_ception_free);
  }
  else if(id == 2){
    create_collision(scene, body1, body2, (collision_handler_t)destroy, aux, aux_ception_free);
  }
}

void impulse(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // id = 1
  aux_t *data = (aux_t *)aux;
  double elas = data->constant;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  vector_t vel1 = body_get_velocity(body1);
  vector_t vel2 = body_get_velocity(body2);
  double u1 = vec_dot(vel1, axis);
  double u2 = vec_dot(vel2, axis);
  double reduced_mass = 0;
  if(mass1 != INFINITY && mass2 != INFINITY){
    reduced_mass = mass1*mass2/(mass1+mass2);
  }
  else if(mass1 != INFINITY){
    reduced_mass = mass1;
  }
  else if(mass2 != INFINITY){
    reduced_mass = mass2;
  }
  vector_t *imp = malloc(sizeof(vector_t));
  *imp = vec_multiply(reduced_mass* (1.0 + elas) * (u2 - u1), axis);
  vector_t *neg_imp = malloc(sizeof(vector_t));
  *neg_imp = vec_negate(*imp);
  body_add_impulse(body1, imp);
  body_add_impulse(body2, neg_imp);
}

void destroy_brick(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // Assumes input is a ball and brick and gets rid of whichever one is the brick
  // id = 2
  aux_t *data = (aux_t *)aux;
  double elas = data->constant;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  vector_t vel1 = body_get_velocity(body1);
  vector_t vel2 = body_get_velocity(body2);
  double u1 = vec_dot(vel1, axis);
  double u2 = vec_dot(vel2, axis);
  double reduced_mass = 0;
  if(mass1 != INFINITY && mass2 != INFINITY){
    reduced_mass = mass1*mass2/(mass1+mass2);
  }
  else if(mass1 != INFINITY){
    reduced_mass = mass1;
  }
  else if(mass2 != INFINITY){
    reduced_mass = mass2;
  }
  vector_t *imp = malloc(sizeof(vector_t));
  *imp = vec_multiply(reduced_mass* (1.0 + elas) * (u2 - u1), axis);
  if(strcmp((char *)body_get_info(body1), "brick") == 0){
    body_remove(body1);
    *imp = vec_negate(*imp);
    body_add_impulse(body2, imp);
  }
  else if (strcmp((char *)body_get_info(body2), "brick") == 0){
    body_remove(body2);
    body_add_impulse(body1, imp);
  }
}

void bullet_explosive(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // Assumes input is bullet then alien. Destroys bullet.
  // id = 3
  aux_t *data = (aux_t *)aux;
  double elas = data->constant;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  vector_t vel1 = body_get_velocity(body1);
  vector_t vel2 = body_get_velocity(body2);
  double u1 = vec_dot(vel1, axis);
  double u2 = vec_dot(vel2, axis);
  double reduced_mass = 0;
  if(mass1 != INFINITY && mass2 != INFINITY){
    reduced_mass = mass1*mass2/(mass1+mass2);
  }
  else if(mass1 != INFINITY){
    reduced_mass = mass1;
  }
  else if(mass2 != INFINITY){
    reduced_mass = mass2;
  }
  vector_t *imp = malloc(sizeof(vector_t));
  *imp = vec_multiply(reduced_mass* (1.0 + elas) * (u2 - u1), axis);
  if(strcmp((char *)body_get_info(body1), "e") == 0){
    body_remove(body1);
    *imp = vec_negate(*imp);
    body_add_impulse(body2, imp);
  }
  else if (strcmp((char *)body_get_info(body2), "e") == 0){
    body_remove(body2);
    body_add_impulse(body1, imp);
  }
}

void bullet_gravity(body_t *body1, body_t *body2, vector_t axis, void *aux){
  // Assumes input is bullet then alien. Destroys bullet.
  // id = 4
  aux_t *data = (aux_t *)aux;
  double gamma = data->constant;
  scene_t *scene = data->scene;
  if(strcmp((char *)body_get_info(body1), "g") == 0){
    create_drag(scene, gamma, body2, 2);
    body_remove(body1);
  }
  else if (strcmp((char *)body_get_info(body2), "g") == 0){
    create_drag(scene, gamma, body1, 2);
    body_remove(body2);
  }
}


void create_physics_collision(scene_t *scene, double elasticity, body_t *body1, body_t *body2, int id){
  list_t *bodies = list_init(2, (free_func_t)(free_nothing));
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux_t *aux = aux_init(bodies, elasticity);
  aux->scene = scene;
  if(id == 1){
    create_collision(scene, body1, body2, (collision_handler_t)impulse, aux, aux_ception_free);
  }
  else if(id == 2){
    create_collision(scene, body1, body2, (collision_handler_t)destroy_brick, aux, aux_ception_free);
  }
  else if(id == 3){
    create_collision(scene, body1, body2, (collision_handler_t)bullet_explosive, aux, aux_ception_free);
  }
  else if(id == 4){
    create_collision(scene, body1, body2, (collision_handler_t)bullet_gravity, aux, aux_ception_free);
  }
}
