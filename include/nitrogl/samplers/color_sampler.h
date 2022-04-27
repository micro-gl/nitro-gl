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
#include <nitrogl/traits.h>
#include <nitrogl/color.h>

namespace nitrogl {

    struct color_sampler : public sampler_t {
        const char * name() const override { return "color_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    vec4 color; // color
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
    return data.color;
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            GLint loc = get_uniform_location(program, "color");
            glUniform4f(loc, color.r, color.g, color.b, color.a);
        }

    private:

    public:
        color_t color;
        color_sampler() : color{1.0, 1.0, 1.0, 1.0}, sampler_t() {}
        explicit color_sampler(color_t $color) : color($color), sampler_t() {}
        color_sampler(float r, float g, float b, float a) : color{r, g, b, a}, sampler_t() {}
    };
}