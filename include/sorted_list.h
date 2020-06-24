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

/**
 * Initializes a sorted list with a specific size and a free function
 *
 * @param size the size of the sorted list
 * @param freer the free function
 * @return the initialized sorted list
 */
slist_t *sl_init(int size, free_func_t freer);

/**
 * Frees the sorted list
 *
 * @param sl the sorted list
 */
void sl_free(slist_t *sl);

/**
 * Enqueues an item
 *
 * @param slist the sorted list
 * @param item the item to be enqueued
 */
void sl_enqueue(slist_t *slist, node_t *item);

/**
 * Returns the size of occupied elements of a sorted list
 *
 * @param slist the sorted list
 * @return the size of the occupied elements of a sorted list
 */
size_t sl_size(slist_t *slist);

/**
 * Returns the element in the last index of the sorted list
 *
 * @param slist the sorted list
 * @return the element in the peeked position
 */
void *sl_peek(slist_t *slist);

/**
 * Dequeues an element from the sorted list
 * @param slist the sorted list
 * @return the element dequeued
 */
void *sl_dequeue(slist_t *slist);

/**
 * Determines if two nodes are the same
 * @param first the first node to be compared to
 * @param second the second node tp be compared to
 */
bool node_compare(node_t *first, node_t *second);

/**
 * Changes the priority of the item item to the value priority
 *
 * @param slist the sorted list
 * @param item the item to have its priority changed
 * @param priority the priority the item will be changed to
 */
void sl_change_priority(slist_t *slist, node_t *item, double priority);

#endif
