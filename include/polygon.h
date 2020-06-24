#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "list.h"
#include "vector.h"

typedef struct polygon polygon_t;

/**
 * initialize new polygon object.
 *
 * @param size_t num_pts
 **/
polygon_t *polygon_init(size_t num_pts);

/**
 * Free pointers of polygons. Is of type free_func_t and can be passed in as arg.
 */
/**
 * Frees a vector pointer.
 *
 * @param v, polygon_t pointer
 */

void polygon_free(void *polygon);

/**
 * Set points of polygon given vec_list_t
 * @param vec_list_t of points for each polygon
 * @param polygon_t * polygon
 */
void polygon_set_points(polygon_t *polygon, list_t *points);

/**
 * Returns list of the points in polygon
 * @param polygon_t *polygon
 */
list_t *polygon_get_points(polygon_t *polygon);

/**
 * Computes the area of a polygon.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon object with the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the polygon
 */
double polygon_area(polygon_t *polygon);

/**
 * Computes the center of mass of a polygon.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon object with the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the polygon
 */
vector_t polygon_centroid(polygon_t *polygon);

/**
 * Translates all vertices in a polygon by a given vector.
 * Note: mutates the original polygon.
 *
 * @param polygon object with the list of vertices that make up the polygon
 * @param translation the vector to add to each vertex's position
 */
void polygon_translate(polygon_t *polygon, vector_t translation);

/**
 * Rotates vertices in a polygon by a given angle about a given point.
 * Note: mutates the original polygon.
 *
 * @param polygon object with the list of vertices that make up the polygon
 * @param angle the angle to rotate the polygon, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */
void polygon_rotate(polygon_t *polygon, double angle, vector_t point);


#endif // #ifndef __POLYGON_H__
