/*
    Implementation of a border finding algorithm from the paper:
    Topological Structural Analysis of Digitized Binary Images by Border Following
    by
        SATOSHI SUZUKI of Graduate School of Electronic Science and Technology, Shizuoka University, Hamamatsu 432, Japan
        KEIICHI ABE of Department of Computer Science, Shizuoka University, Hamamatsu 432, Japan
    The paper: https://www2.ipcku.kansai-u.ac.jp/~yasumuro/M_InfoMedia/paper/Suzuki85.pdf
*/

#ifndef SUZUKI_ABE_H
#define SUZUKI_ABE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SA_Point
{
    int x;
    int y;
} SA_Point;

typedef struct SA_Border
{
    size_t first_point;
    size_t points_count;
    // Index into borders array, or image value (when incremented):
    //  borders[parent]
    //  grid[y*width+x] == parent+1
    int parent;
    bool is_hole;
} SA_Border;

typedef struct SA_Topology
{
    // Borders topology tree.
    // Index 0 is default root item.
    // Index is related to the analyzed image values (when incremented),
    // i.e. to access the border data when traversing the image grid
    //      if(image[y*width+x] != 0) {
    //          SA_Border border = borders[image[y*width+x]-1];
    //      }
    struct SA_Border* borders;
    size_t borders_count;
    size_t borders_capacity;

    struct SA_Point* points;
    size_t points_count;
    size_t points_capacity;
} SA_BorderTopology;

/// Find borders and border topology using Suzuki Abe algorithm.
/// Provided image has to have 0 at its borders and only contain 0s and 1s inside.
///
/// @param image Binary image data. Will be modified by the algorithm.
/// @param image_width image width
/// @param image_height image height
/// @param topology image border topology
/// @return 0 on success
int SA_extract_topology(int* image, int image_width, int image_height, SA_BorderTopology* topology);

/// Allocate SA_BorderTopology for use with SA_extract_topology
void SA_allocate_topology(SA_BorderTopology* topology, size_t borders_capacity, size_t points_capacity);

/// Free the SA_BorderTopology
void SA_free_topology(SA_BorderTopology* topology);

#ifdef __cplusplus
}
#endif

#endif // SUZUKI_ABE_H

/*
Copyright (c) 2026 Michał Ociepa
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
