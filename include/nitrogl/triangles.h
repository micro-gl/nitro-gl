/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "math.h"
#include "functions/minmax.h"
#ifndef NITROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/triangles.h"
#else
#include <micro-tess/triangles.h>
#endif

namespace nitrogl {
    namespace triangles {
        using index = GLuint;
        enum indices {
            TRIANGLES=GL_TRIANGLES,
            TRIANGLES_FAN=GL_TRIANGLE_FAN,
            TRIANGLES_STRIP=GL_TRIANGLE_STRIP,
            LINE_STRIP=GL_LINE_STRIP,
            LINE_LOOP=GL_LINE_LOOP,
            LINES=GL_LINES,
            POINTS=GL_POINTS
        };

        inline indices microtess_indices_type_to_nitrogl(microtess::triangles::indices type) {
            switch (type) {
                case microtess::triangles::indices::TRIANGLES_WITH_BOUNDARY:
                case microtess::triangles::indices::TRIANGLES:
                {
                    return nitrogl::triangles::indices::TRIANGLES;
                }
                case microtess::triangles::indices::TRIANGLES_FAN:
                case microtess::triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY:
                {
                    return nitrogl::triangles::indices::TRIANGLES_FAN;
                }
                case microtess::triangles::indices::TRIANGLES_STRIP:
                case microtess::triangles::indices::TRIANGLES_STRIP_WITH_BOUNDARY:
                {
                    return nitrogl::triangles::indices::TRIANGLES_STRIP;
                    break;
                }
                default:
                    return nitrogl::triangles::indices::TRIANGLES;
            }
        }

        /**
         * Iterate triangles encoded in a list of indices
         * @tparam iterator_callback a callback struct or lambda
         * @param indices pointer to indices array
         * @param size size of indices list
         * @param type the type of triangles
         * @param callback the callback instance
         */
        template<typename iterator_callback>
        void iterate_triangles(const index *indices,
                               const index &size,
                               const enum triangles::indices &type,
                               const iterator_callback & callback) {
#define IND(a) ((indices) ? indices[(a)] : (a))
            switch (type) {
                case indices::TRIANGLES:
                    for (index ix = 0, idx=0; ix < size; ix+=3,idx++)
                        callback(idx, IND(ix + 0), IND(ix + 1), IND(ix + 2), 0, 1, 2);
                    break;
                case indices::TRIANGLES_FAN:
                    for (index ix = 1; ix < size-1; ++ix)
                        callback(ix-1, IND(0), IND(ix), IND(ix + 1), 0, 1, 2);
                    break;
                case indices::TRIANGLES_STRIP:
                {
                    bool even = true;
                    for (index ix = 0; ix < size-2; ++ix) {
                        // we alternate order inorder to preserve CCW or CW,
                        index first_index = even ?  IND(ix + 0) : IND(ix + 2);
                        index second_index = IND(ix + 1);
                        index third_index = even ?  IND(ix + 2) : IND(ix + 0);
                        callback(ix, first_index, second_index, third_index, even?0:1, even?1:0, 2);
                        even = !even;
                    }
                    break;
                }
                default: {
                    // todo: throw error
                    return;
                }
            }
#undef IND
        }

        /**
         * Compute triangles bbox
         * @param vertices pointer to array of vertices
         * @param size_vertices size of vertices (if not null), or vertices (if indices are null)
         * @param indices (Optional) pointer to array of indices to vertices array
         * @param size_indices (Optional) size of indices (if not null), or vertices (if indices are null)
         * @return bounding box rectangle
         */
        rectf triangles_bbox(const vec2f *vertices,
                             const index size_vertices,
                            const index *indices,
                            const index size_indices) {
            const bool has_indices = indices!=nullptr && size_indices!=0;
            const auto & ref_base = has_indices ? vertices[indices[0]] : vertices[0];
            rectf rect{ ref_base.x, ref_base.y, ref_base.x, ref_base.y };
            const auto size = has_indices ? size_indices : size_vertices;
            for (unsigned ix = 0; ix < size; ++ix) { // compute bounding box
                const auto & pt = has_indices ? vertices[indices[ix]] : vertices[ix];
                rect.left = functions::min(rect.left, pt.x);
                rect.top = functions::min(rect.top, pt.y);
                rect.right = functions::max(rect.right, pt.x);
                rect.bottom = functions::max(rect.bottom, pt.y);
            }
            return rect;
        }

        /**
         * Compute triangles bbox
         * @param attribs pointer to array of vertices
         * @param size_attribs (Optional) will be used if indices is null
         * @param indices (Optional) pointer to array of indices to vertices array
         * @param size_indices size of indices (if not null), or vertices (if indices are null)
         * @return bounding box rectangle
         */
        rectf triangles_bbox_from_attribs(const float *attribs,
                                          const index size_attribs,
                                          const index *indices,
                                          const index size_indices,
                                          index x_idx, index y_idx, index window_size) {
            const bool has_indices = indices!=nullptr && size_indices!=0;
            const auto & ref_base = has_indices ? attribs[indices[0]] : attribs[0];
            rectf rect{ attribs[x_idx], attribs[y_idx], attribs[x_idx], attribs[y_idx] };
            const auto size = !has_indices ? size_attribs : size_indices;
            for (unsigned ix = 0; ix < size; ++ix) { // compute bounding box
                const float * window = attribs + (has_indices ? indices[ix] : ix)*window_size;
                rect.left = functions::min(rect.left, window[x_idx]);
                rect.top = functions::min(rect.top, window[y_idx]);
                rect.right = functions::max(rect.right, window[x_idx]);
                rect.bottom = functions::max(rect.bottom, window[y_idx]);
            }
            return rect;
        }
    };
}