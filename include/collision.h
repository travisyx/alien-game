#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <stdbool.h>
#include "list.h"
#include "object.h"
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Represents the status of a collision between two shapes.
 * The shapes are either not colliding, or they are colliding along some axis.
 */
typedef struct {
    /** Whether the two shapes are colliding */
    bool collided;
    /**
     * If the shapes are colliding, the axis they are colliding on.
     * This is a unit vector pointing from the first shape towards the second.
     * Normal impulses are applied along this axis.
     * If collided is false, this value is undefined.
     */
    vector_t axis;
} collision_info_t;

/**
 * Computes the status of the collision between two convex polygons.
 * The shapes are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from shape1 towards shape2.
 */
collision_info_t find_collision(list_t *shape1, list_t *shape2);

/**
 * Rendition of above method, but for objects. Uses object bounding box test.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from shape1 towards shape2.
 */
collision_info_t object_find_collision(list_t *shape1, list_t *shape2);

/**
 * Tests if two objects are colliding via the bounding box method
 *
 * @param one the first object
 * @param two the second object
 */
bool object_test_bounding_box(object_t *one, object_t *two);

/**
 * Tests if two bodies are colliding via the bounding box method
 *
 * @param body_one the first body
 * @param body_two the second body
 */
bool body_test_bounding_box(list_t *body_one, list_t *body_two);

#endif // #ifndef __COLLISION_H__
