#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>


/**
 * A growable array of pointers.
 * Can store values of any pointer type (e.g. vector_t*, body_t*).
 * The list automatically grows its internal array when more capacity is needed.
 */
typedef struct list list_t;

/**
 * A function that can be called on list elements to release their resources.
 * Examples: free, body_free
 */
typedef void (*free_func_t)(void *);

// replace element at index with new item, return old item.
// assuming that items are of same type. if not, you will probably break the list
void *list_replace(list_t *list, size_t index, void *new_item);

/**
 * Allocates memory for a new list with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param initial_size the number of elements to allocate space for
 * @param freer if non-NULL, a function to call on elements in the list
 *   in list_free() when they are no longer in use
 * @return a pointer to the newly allocated list
 */
list_t *list_init(size_t initial_size, free_func_t freer);

/**
 * Releases the memory allocated for a list.
 *
 * @param list a pointer to a list returned from list_init()
 */
void list_free(void *list);

/**
 * Gets the size of a list (the number of occupied elements).
 * Note that this is NOT the list's capacity.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the number of elements in the list
 */
size_t list_size(list_t *list);

/**
 * Gets the capacity of a list (the number of spaces in array).
 * Primarily here for testing purposes, as capacity is relevant when
 * the list must be resized.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the number of spaces in the list
 */
size_t list_capacity(list_t *list);

/**
 * Gets the element at a given index in a list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from list_init()
 * @param index an index in the list (the first element is at 0)
 * @return the element at the given index, as a void*
 */
void *list_get(list_t *list, size_t index);

/**
 * Collapses all elements to the right of ind one to the left, setting the
 * last element equal to null
 *
 * @param list a pointer to a list returned from list_init()
 * @param index being removed
 */

void collapse(list_t *list, size_t ind);

/**
 * Removes the element at a given index in a list and returns it,
 * moving all subsequent elements towards the start of the list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the element at the given index in the list
 */
void *list_remove(list_t *list, size_t index);

/**
 * Doubles the current capacity of list->data to make space for new
 * elements. Copies over elements that were already added.
 *
 * @param list a pointer to a list returned from list_init()
 */
void resize(list_t *list);

/**
 * Appends an element to the end of a list.
 * If the list is filled to capacity, resizes the list to fit more elements
 * and asserts that the resize succeeded.
 * Also asserts that the value being added is non-NULL.
 *
 * @param list a pointer to a list returned from list_init()
 * @param value the element to add to the end of the list
 */
void list_add(list_t *list, void *value);

/**
* Clears all elements of given list. List capacity remains the same.
*
* @param list to be cleared
*/
void list_clear(list_t *list);

// Swaps two elements in a list.
void list_swap(list_t *list, int ind1, int ind2);

#endif // #ifndef __LIST_H__
