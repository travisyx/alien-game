#ifndef __SORTED_LIST_H__
#define __SORTED_LIST_H__

#include "map.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

typedef struct slist{
  list_t *list;
} slist_t;

slist_t *sl_init(int size, free_func_t freer);

void sl_free(slist_t *sl);

// assume that item has double priority in struct
// assume list is full of items
void sl_enqueue(slist_t *slist, node_t *item);

size_t sl_size(slist_t *slist);

// first element w/o removing
void *sl_peek(slist_t *slist);

// rem 0 element
void *sl_dequeue(slist_t *slist);

bool node_compare(node_t *first, node_t *second);

// be sure to put in correct place in list now
void sl_change_priority(slist_t *slist, node_t *item, double priority);

void sl_print(slist_t *slist);

void sl_reset_and_free(slist_t *slist);

#endif
