#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Size increase of list when no space left to add into.
const int GROWTH_RATE = 2;

typedef struct list {
    void **data;
    size_t capacity;
    size_t size;
    free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
    list_t *v = malloc(sizeof(list_t));
    assert(v != NULL);
    v->data = malloc(initial_size * sizeof(void *));
    assert(v->data != NULL);
    v->capacity = initial_size;
    v->size = 0;
    v->freer = freer;
    return v;
}

// Checks if freer is NULL as well and does not try to free objects.
void list_free(void *list) {
    if(((list_t *)list)->freer != NULL){
      for (size_t i = 0; i < list_size(list); i++) {
          ((list_t *)list)->freer(list_get(list, i));
      }
    }
    free(((list_t *)list)->data);
    free(list);
}

size_t list_size(list_t *list) {
    return list->size;
}

size_t list_capacity(list_t *list) {
    return list->capacity;
}

void *list_get(list_t *list, size_t index) {
    assert(index < list->size && index >= 0);
    void *item = list->data[index];
    return item;
}

void resize(list_t *list) {
    void **new_data = malloc(GROWTH_RATE * list->capacity * sizeof(void *));
    for (size_t i = 0; i < list->size; i++) {
        new_data[i] = list_get(list, i);
    }
    free(list->data);
    list->data = new_data;
    list->capacity *= GROWTH_RATE;
}

void list_add(list_t *list, void *value) {
    assert(value != NULL);
    if (list->size >= list->capacity) {
        resize(list);
    }
    int next_ind = list->size;
    list->data[next_ind] = value;
    list->size++;
}

// Helper method to collapse list down given index to start from (of rem. item).
void collapse(list_t *list, size_t ind) {
    for (size_t i = ind; i < list->size - 1; i++) {
        list->data[i] = list_get(list, i + 1);
    }
    list->data[list->size - 1] = (void *)NULL;
}

void *list_remove(list_t *list, size_t ind) {
    assert(ind >= 0);
    assert(ind < list_capacity(list));
    void *temp = list->data[ind];
    collapse(list, ind);
    list->size--;
    return temp;
}

void list_clear(list_t *list){
  if(((list_t *)list)->freer != NULL){
    while(list_size(list) > 0){
      list->freer(list_remove(list, 0));
    }
  }
  else{
    while(list_size(list) > 0){
      list_remove(list, 0);
    }
  }
}

void *list_replace(list_t *list, size_t ind, void *new_item){
  assert(ind >= 0);
  assert(ind < list_capacity(list));
  void *temp = list->data[ind];
  list->data[ind] = new_item;
  return temp;
}

void list_swap(list_t *list, int ind1, int ind2){
  void *item1 = list_get(list, ind1);
  void *item2 = list_get(list, ind2);
  list_replace(list, ind1, item2);
  list_replace(list, ind2, item1);
}
