#include "sa_border_topology/sa_border_topology.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#ifndef SA_GROWTH_FACTOR
#   define SA_GROWTH_FACTOR 1.5f
#endif

int topology_append_border(SA_BorderTopology* bt, SA_Border border)
{
    if (false == (bt->borders_count < bt->borders_capacity)) {
        size_t const new_capacity = fmaxf(bt->borders_count+1, bt->borders_capacity * SA_GROWTH_FACTOR);
        SA_Border* new_borders = realloc(bt->borders, new_capacity * sizeof(SA_Border));
        if (new_borders == NULL) {
            return 1;
        }
        bt->borders = new_borders;
    }

    bt->borders[bt->borders_count] = border;
    ++bt->borders_count;

    return 0;
}

int topology_append_border_point(SA_BorderTopology* bt, SA_Border* border, SA_Point point)
{
    if (false == (bt->points_count < bt->points_capacity)) {
        size_t const new_capacity = fmaxf(bt->points_count+1, bt->points_capacity * SA_GROWTH_FACTOR);
        SA_Point* new_points = realloc(bt->points, new_capacity * sizeof(SA_Point));
        if (new_points == NULL) {
            return 1;
        }
        bt->points = new_points;
    }

    assert(bt->points_count == border->first_point+border->points_count);

    bt->points[bt->points_count] = point;
    ++bt->points_count;
    ++border->points_count;

    return 0;
}

int SA_extract_topology(int* image, int image_width, int image_height, SA_BorderTopology* topology)
{
    SA_Point const clockwise[] = {
        {-1, 0},
        {-1, -1},
        {0, -1},
        {1, -1},
        {1, 0},
        {1, 1},
        {0, 1},
        {-1, 1},
    };

    // default parent is always a hole
    {
        SA_Border const image_border = {
            .first_point = 0,
            .points_count = 0,
            .parent = 0,
            .is_hole = true,
        };
        topology_append_border(topology, image_border);
    }

    int nbd = 1;
    int y;
    for (y = 1; y < image_height - 1; ++y) {
        int lnbd = 1;
        int x;
        for (x = 1; x < image_width - 1; ++x) {
            int i2j2_index;
            bool is_hole;
            {
                // (1)
                int const center = image[y * image_width + x];
                if (center == 0) continue;
                int const left = image[y * image_width + x - 1];
                int const right = image[y * image_width + x + 1];

                // (1a)
                if (center == 1 && left == 0) {
                    // outer border start
                    is_hole = false;
                    i2j2_index = 0; // to the left
                }
                // (1b)
                else if (center >= 1 && right == 0) {
                    // hole border start
                    is_hole = true;
                    i2j2_index = 4; // to the right
                    if (center > 1) lnbd = center;
                }
                // (1c)
                else {
                    i2j2_index = -1;
                    // go to (4)
                }
            }

            if (i2j2_index != -1) {
                SA_Border* current_border;
                // (2)
                // register newly found border
                {
                    // NOTE: parenting rule from TABLE 1
                    nbd += 1;
                    SA_Border const* lnbd_data = &topology->borders[lnbd - 1];
                    int const parent = is_hole == lnbd_data->is_hole
                                           ? lnbd_data->parent
                                           : lnbd - 1;
                    SA_Border const new_border = {
                        .first_point = topology->points_count,
                        .points_count = 0,
                        .parent = parent,
                        .is_hole = is_hole,
                    };
                    if (topology_append_border(topology, new_border)) {
                        return 1;
                    }
                    assert((int64_t)nbd == (int64_t)topology->borders_count);
                    current_border = &topology->borders[nbd - 1];

                    SA_Point const new_point = {.x = x, .y = y};
                    if (topology_append_border_point(topology, current_border, new_point)) {
                        return 1;
                    }
                }

                // (3)
                // tracing border
                {
                    // (3.1)
                    // travel clockwise until non-zero value is found
                    SA_Point i1j1;
                    int i1j1_index = -1;
                    int i;
                    for (i = 0; i < 8; ++i) {
                        int const computed_idx = (i2j2_index + i) % 8;
                        SA_Point const coord = {x + clockwise[computed_idx].x, y + clockwise[computed_idx].y};
                        int const value = image[coord.y * image_width + coord.x];

                        if (value != 0) {
                            i1j1_index = computed_idx;
                            i1j1 = coord;
                            break;
                        }
                    }

                    if (i1j1_index == -1) {
                        // no non-zero value found, set to -NBD and go to (4)
                        image[y * image_width + x] = -nbd;
                    }
                    else {
                        // non-zero value found, continue with algorithm

                        // (3.2)
                        // i2j2 = i1j1
                        // i3j3 = i,j
                        i2j2_index = i1j1_index;
                        SA_Point i3j3 = {x, y};

                        // (3.3)
                        // travel counterclockwise from the found non-zero value
                        while (true) {
                            int i4j4_index = -1;
                            SA_Point i4j4;
                            bool did_examine_right = false;
                            for (i = 0; i < 8; ++i) {
                                // NOTE: -i-1 because ccw we start from the next element
                                int const computed_idx = (i2j2_index - i - 1 + 8) % 8;
                                SA_Point const coord = {
                                    i3j3.x + clockwise[computed_idx].x, i3j3.y + clockwise[computed_idx].y
                                };
                                int const value = image[coord.y * image_width + coord.x];

                                if (value != 0) {
                                    i4j4_index = computed_idx;
                                    i4j4 = coord;
                                    break;
                                }
                                if (computed_idx == 4) {
                                    did_examine_right = true;
                                }
                            }
                            // NOTE: this should always find some non-zero value, because at the very least
                            //       it will go all the way around to the one at non_zero_coord_index_cw
                            assert(i4j4_index != -1);

                            // (3.4)
                            // (3.4a)
                            if (did_examine_right && image[i3j3.y * image_width + i3j3.x + 1] == 0) {
                                image[i3j3.y * image_width + i3j3.x] = -nbd;
                            }
                            // (3.4b)
                            else if (image[i3j3.y * image_width + i3j3.x] == 1) {
                                image[i3j3.y * image_width + i3j3.x] = nbd;
                            }
                            // (3.4c) noop
                            // else {}

                            SA_Point const new_point = {.x = i3j3.x, .y = i3j3.y};
                            if (topology_append_border_point(topology, current_border, new_point)) {
                                return 1;
                            }

                            // (3.5)
                            if (i4j4.x == x && i4j4.y == y
                                && i3j3.x == i1j1.x && i3j3.y == i1j1.y) {
                                // coming back to the start point
                                // i4,j4 == x,y && i3,j3 == i1,j1
                                break;
                            }

                            // NOTE: this seems to be just swapping current traversal center and non-zero coordinates around
                            // i2,j2 = i3,j3
                            i2j2_index = (i4j4_index + 4) % 8; // opposite direction
                            // i3,j3 = i4,j4
                            i3j3.x = i3j3.x + clockwise[i4j4_index].x;
                            i3j3.y = i3j3.y + clockwise[i4j4_index].y;
                        }
                    }
                }
            }

            // (4)
            int const center = image[y * image_width + x];
            if (center != 1) {
                lnbd = abs(center);
            }
        }
    }

    return 0;
}

void SA_allocate_topology(SA_BorderTopology* topology, size_t borders_capacity, size_t points_capacity)
{
    topology->borders = malloc(borders_capacity * sizeof(SA_Border));
    topology->borders_count = 0;
    topology->borders_capacity = borders_capacity;
    topology->points = malloc(points_capacity * sizeof(SA_Point));
    topology->points_count = 0;
    topology->points_capacity = points_capacity;
}

void SA_free_topology(SA_BorderTopology* topology)
{
    free(topology->borders);
    free(topology->points);
    topology->borders = NULL;
    topology->borders_count = 0;
    topology->borders_capacity = 0;
    topology->points = NULL;
    topology->points_count = 0;
    topology->points_capacity = 0;
}
