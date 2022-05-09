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
#include <nitrogl/math.h>

namespace nitrogl {

    /**
     * Angular Gradient sampler.
     *
     * Notes:
     * - Can hold up to 10 colors
     */
    struct angular_gradient : public sampler_t {
        const char * name() const override { return "line_gradient"; }
        const char * uniforms() const override {
            return R"(
{
    float inputs[10*6+3];
}
)";
        }

        const char * other_functions() const override {
            return R"(
vec4 other_function(float t) {
    return vec4(t);
}
)";
        }

        const char * main() const override {
            return R"(
(vec3 uv) {
    //////////////
    // main input
    //////////////
    // how many slots are occupied
    int count = int(data.inputs[0]);
    int window_size = int(data.inputs[1]);
    int offset = int(data.inputs[2]);
    vec2 p = uv.xy - 0.5f;

#define IDX(a) (offset + (a) * (window_size))
#define PI 3.14159

    //
    int pos = 0;
    float angle = -atan(p.t, -p.s) + PI;
    float distance_to_closest_stop = 0.0;
    for (pos=0; pos<count; ++pos) {
        int idx = IDX(pos);
        // distance to this angular stop
        float d = angle - data.inputs[idx+0];
        if(d<0) break;
        distance_to_closest_stop=d;
    }

    // left and right extremes
    if(pos==count) {
        int idx = IDX(count-1);
        return vec4(data.inputs[idx+2], data.inputs[idx+3], data.inputs[idx+4], data.inputs[idx+5]);
    } else if(pos==0) {
        int idx = offset;
        return vec4(data.inputs[idx+2], data.inputs[idx+3], data.inputs[idx+4], data.inputs[idx+5]);
    }

    int l_idx = IDX(pos-1);
    int r_idx = l_idx + window_size;
    float segment_length= data.inputs[r_idx+1];
    float factor= distance_to_closest_stop/segment_length;
    vec4 col_l = vec4(data.inputs[l_idx+2], data.inputs[l_idx+3], data.inputs[l_idx+4], data.inputs[l_idx+5]);
    vec4 col_r = vec4(data.inputs[r_idx+2], data.inputs[r_idx+3], data.inputs[r_idx+4], data.inputs[r_idx+5]);
    vec4 final = mix(col_l, col_r, factor);
    return final;
}
)";
        }

    private:
        static constexpr int window_size() {
            return  6;
        }
        static constexpr int offset() {
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
            inputs[0] = float(_index);
            inputs[1] = float(window_size()); // window size
            inputs[2] = float(offset()); // window size
            for (int ix = 0; ix < _index; ++ix) {
                const auto & stop = _stops[ix];
                int jx = offset() + ix * window_size();
                inputs[jx + 0] = stop.angle;
                inputs[jx + 1] = stop.segment_length;
                inputs[jx + 2] = stop.color.r;
                inputs[jx + 3] = stop.color.g;
                inputs[jx + 4] = stop.color.b;
                inputs[jx + 5] = stop.color.a;
            }
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, overall_size(), inputs);
        }

        struct stop_t {
            float angle=0.0f;
            float segment_length=0.0f;
            color_t color{};
        };

        void setNewInterval(const vec2f & interval) {
            _interval= interval;
            const auto how_many = _index;
            reset();
            for (int ix = 0; ix < how_many; ++ix)
                addStop(_stops[ix].angle, _stops[ix].color);
        }

        void updateStop(int index, float where, color_t color) {
            if(index>_index) {
#ifdef NITROGL_ENABLE_THROW
                struct out_of_range{};
                throw out_of_range{};
#endif
                return;
            }

            auto & stop = _stops[index];

            stop.angle = where * (_interval.y-_interval.x) * nitrogl::math::pi<float>()/180.0f;
            stop.color = color;

            if(index>0) {
                stop.segment_length = (stop.angle-_stops[index-1].angle);
            }
        }

        void addStop(float where, color_t color) {
            updateStop(_index, where, color);
            ++_index;
        }
        int stops() const { return _index; }

        void reset() { _index=0; }

    private:
        vec2f _interval;
        int _index = 0;
        stop_t _stops[10] {};

    public:
        explicit angular_gradient(float from_deg=0.0f, float to_deg=360.0f) :
                _interval(from_deg, to_deg) {
            setNewInterval(_interval);
        };

    };
}
