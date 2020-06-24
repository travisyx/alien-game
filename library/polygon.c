#include "polygon.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int NUM_COL = 3;
const double CENT1 = 6.0;
const double AREA1 = 2.0;

typedef struct polygon {
    list_t *points;
} polygon_t;

polygon_t *polygon_init(size_t num_pts) {
    polygon_t *p = malloc(sizeof(polygon_t));
    assert(p != NULL);
    p->points = list_init(num_pts, vec_free);
    assert(p->points != NULL);
    return p;
}

void polygon_free(void *polygon) {
    list_free(((polygon_t *)polygon)->points);
    free(((polygon_t *)polygon));
}

void polygon_set_points(polygon_t *polygon, list_t *points) {
    list_free(polygon->points);
    polygon->points = points;
}

list_t *polygon_get_points(polygon_t *polygon) {
    return polygon->points;
}

double polygon_area(polygon_t *polygon) {
    double area = 0.0;
    list_t *points = polygon->points;
    size_t num_vert = list_size(points);
    // j will serve as "previous" vertex
    size_t j = num_vert - 1;
    for (size_t i = 0; i < num_vert; i++) {
        area += ((((vector_t *)list_get(points, j))->x *
                  ((vector_t *)list_get(points, i))->y) -
                 (((vector_t *)list_get(points, j))->y *
                  ((vector_t *)list_get(points, i))->x));
        // keep one behind i
        j = i;
    }
    return fabs(area / AREA1);
}

vector_t polygon_centroid(polygon_t *polygon) {
    double center_x = 0.0;
    double center_y = 0.0;
    list_t *points = polygon->points;
    size_t num_vert = list_size(points);
    // j will serve as "previous" vertex
    size_t j = num_vert - 1;
    for (size_t i = 0; i < num_vert; i++) {
        // calculate this first bc it's common factor for both x and y coordinates
        double comm_f =
            (((vector_t *)list_get(points, j))->x *
             ((vector_t *)list_get(points, i))->y) -
            (((vector_t *)list_get(points, i))->x * ((vector_t *)list_get(points, j))->y);
        center_x += (((vector_t *)list_get(points, j))->x +
                     ((vector_t *)list_get(points, i))->x) *
                    comm_f;
        center_y += (((vector_t *)list_get(points, j))->y +
                     ((vector_t *)list_get(points, i))->y) *
                    comm_f;
        // keep one behind i
        j = i;
    }
    double area = polygon_area(polygon);
    center_x /= (CENT1 * area);
    center_y /= (CENT1 * area);
    vector_t center = {center_x, center_y};
    return center;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
    list_t *points = polygon->points;
    for (size_t i = 0; i < list_size(points); i++) {
        vector_t *vertex = (vector_t *)list_get(points, i);
        *vertex = vec_add(translation, *vertex);
    }
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
    list_t *points = polygon->points;
    for (size_t i = 0; i < list_size(points); i++) {
        vector_t old = *(vector_t *)list_get(points, i);
        // rotate center about point (subtraction gives vector from origin, where
        // origin = point)
        vector_t new = vec_subtract(old, point);
        new = vec_rotate(new, angle);
        // add new vector to point
        new = vec_add(point, new);
        *(vector_t *)list_get(points, i) = new;
    }
}
