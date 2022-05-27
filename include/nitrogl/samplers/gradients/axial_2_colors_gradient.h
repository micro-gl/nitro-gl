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
#include <nitrogl/traits.h>

namespace nitrogl {

    enum class axial_degree {
        _0, _45, _90, _135, _180, _225, _270, _315, _360
    };

    /**
     * A Fast Axial Gradient sampler with 2 colors.
     *
     * Notes:
     * - supports 8 angles {0, 45, 90, 135, 180, 225, 270, 315, 360}
     * - Very fast
     */
    struct axial_2_colors_gradient : public sampler_t {
        const char * name() const override { return "axial_2_colors_gradient"; }
        const char * uniforms() const override {
            return R"(
{
    vec4 colors[2];
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
            if(degree==axial_degree::_0 || degree==axial_degree::_360) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], uv.x);
})";
            } else if (degree==axial_degree::_45) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], (uv.x+uv.y)/2.0);
})";
            } else if (degree==axial_degree::_90) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], uv.y);
})";
            } else if (degree==axial_degree::_135) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], 1.0 - ((uv.x-uv.y)/2.0 + 0.5f));
})";
            } else if (degree==axial_degree::_180) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], 1.0 - uv.x);
})";
            } else if (degree==axial_degree::_225) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], 1.0 - (uv.x+uv.y)/2.0);
})";
            } else if (degree==axial_degree::_270) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], 1.0 - uv.y);
})";
            } else if (degree==axial_degree::_315) {
                return R"(
(in vec3 uv) {
    return mix(data.colors[0], data.colors[1], 0.5f + (uv.x-uv.y)/2.0 );
})";
            }
            return nullptr;
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            float inputs[8] = { color_1.r, color_1.g, color_1.b, color_1.a,
                                color_2.r, color_2.g, color_2.b, color_2.a };
            GLint loc_inputs = get_uniform_location(program, "colors");
            glUniform4fv(loc_inputs, 8, inputs);
        }

    public:

        axial_degree degree;
        color_t color_1, color_2;

        explicit axial_2_colors_gradient(const color_t & color_1 = {1.0, 0.0, 0.0, 1.0},
                                const color_t & color_2 = {0.0, 1.0, 0.0, 1.0},
                                axial_degree degree = axial_degree::_45) :
                color_1(color_1), color_2(color_2), degree(degree) {}
    };
}
