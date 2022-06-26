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

#include <nitrogl/samplers/sampler.h>
#include <nitrogl/color.h>
#include <nitrogl/math/vertex2.h>
#include <nitrogl/traits.h>
#include <nitrogl/functions/distance.h>
#include <nitrogl/_internal/string_utils.h>

namespace nitrogl {

    /**
     * A Gradient sampler.
     *
     * Notes:
     * - Can hold up to 10 colors
     * - You can also change rotation
     */
    struct line_gradient : public sampler_t {
        const char * name() const override { return "line_gradient"; }
        const char * uniforms() const override {
            return R"(
{
    float inputs[10*8+3];
}
)";
        }

        const char * main() const override {
            return R"(
(in vec3 uv) {
    //////////////
    // main input
    //////////////
    // how many slots are occupied
    int count = int(data.inputs[0]);
    int window_size = int(data.inputs[1]);
    int offset = int(data.inputs[2]);
    vec2 p = (uv.xy);

#define IDX(a) (offset + (a) * (window_size))

    //
    int pos = 0;
    float distance = 0.0;
    for (pos=0; pos<count; ++pos) {
        int idx = IDX(pos);
        // a*x + b*y + c
        float d = data.inputs[idx+0]*p.x + data.inputs[idx+1]*p.y + data.inputs[idx+2];
        if(d<0) break;
        distance=d;
    }
    if(pos==count) {
        int idx = IDX(count-1);
        return vec4(data.inputs[idx+4], data.inputs[idx+5], data.inputs[idx+6], data.inputs[idx+7]);
    } else if(pos==0) {
        int idx = offset;
        return vec4(data.inputs[idx+4], data.inputs[idx+5], data.inputs[idx+6], data.inputs[idx+7]);
    }

    int l_idx = IDX(pos-1);
    int r_idx = l_idx + window_size;
    float l_inverse= data.inputs[r_idx+3];
//    float factor= distance/l_inverse;
    float factor= distance/l_inverse;
    vec4 col_l = vec4(data.inputs[l_idx+4], data.inputs[l_idx+5], data.inputs[l_idx+6], data.inputs[l_idx+7]);
    vec4 col_r = vec4(data.inputs[r_idx+4], data.inputs[r_idx+5], data.inputs[r_idx+6], data.inputs[r_idx+7]);
    vec4 final = mix(col_l, col_r, factor);
    return final;
//    return vec4(p.y,p.y,p.y,1.0);
}
)";
        }

    private:
        constexpr int window_size() const {
            // minus the where float
            return  8;
        }
        constexpr int offset() const {
            // [0]=index, [1]=window_size, [2]=offset
            return  3;
        }
        constexpr int overall_size() const {
            return  offset() + _index*window_size();
        }
    public:

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            // minus the where float
            float inputs[overall_size()];
            inputs[0] = _index;
            inputs[1] = window_size(); // window size
            inputs[2] = offset(); // window size
            for (int ix = 0; ix < _index; ++ix) {
                const auto & stop = _stops[ix];
                int jx = offset() + ix * window_size();
                inputs[jx + 0] = stop.a;
                inputs[jx + 1] = stop.b;
                inputs[jx + 2] = stop.c;
                inputs[jx + 3] = stop.length_inverse;
                inputs[jx + 4] = stop.color.r;
                inputs[jx + 5] = stop.color.g;
                inputs[jx + 6] = stop.color.b;
                inputs[jx + 7] = stop.color.a;
            }
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, overall_size(), inputs);
        }

    public:

        struct stop_t {
            float where=0.0f;
            // inverse length of the segment from this stop and the next stop
            float length_inverse=0.0f;
            float a=0.0f, b=0.0f, c=0.0f;
            color_t color{};

            void updateLine(const vec2f & p, vec2f n) {
                // normalize the normal
                n = n/nitrogl::functions::length(n);
                a=n.x, b=n.y, c= -(n.dot(p));
            }
        };

        void setNewLine(const vec2f & start, const vec2f & end) {
            _start= start; _end= end;
            const auto how_many = _index;
            reset();
            for (int ix = 0; ix < how_many; ++ix)
                addStop(_stops[ix].where, _stops[ix].color);
        }

        void updateStop(int index, float where, color_t color) {
            if(index>_index) {
#ifndef NITROGL_DISABLE_THROW
                struct out_of_range{};
                throw out_of_range{};
#endif
                return;
            }

            const auto dir = _end-_start;
            const auto p_w = _start + (_end-_start) * where;
            auto & stop = _stops[index];

            stop.updateLine(p_w, dir);
            stop.where = where;
            stop.color = color;

            if(index>0) {
                float l = (stop.where-_stops[index-1].where)*nitrogl::functions::length(dir);
                stop.length_inverse= l;
                //                stop.a *= l;
                //                stop.b *= l;
                //                stop.c *= l;
            }
        }

        void addStop(float where, color_t color) {
            updateStop(_index, where, color);
            ++_index;
        }
        int stops() const { return _index; }
        void rotate(float angle_radians, vec2f center = vec2f(0.5f, 0.5f)) {
            auto a = _start - center;
            auto b = _end - center;

            float s = nitrogl::math::sin(angle_radians);
            float c = nitrogl::math::cos(angle_radians);

            // rotate point
            float arx = a.x * c - a.y * s;
            float ary = a.x * s + a.y * c;

            float brx = b.x * c - b.y * s;
            float bry = b.x * s + b.y * c;

            vec2f a_new = vec2f(arx, ary) + center;
            vec2f b_new = vec2f(brx, bry) + center;
            setNewLine(a_new, b_new);
        }

        void reset() { _index=0; }

    private:
        vec2f _start, _end;
        int _index = 0;
        stop_t _stops[10] {};

    public:
        explicit line_gradient(const vec2f & start = vec2f(0.0f, 0.5f),
                      const vec2f & end = vec2f(1.0f, 0.5f)) :
                    _start(start), _end(end) {
            setNewLine(start, end);
        };

    };
}
