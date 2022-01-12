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

#include "sampler.h"
#include "../traits.h"

namespace nitrogl {

    struct color_sampler_tag : public sampler_t {
        const char * name() override { return "color_sampler"; }
        const char * uniforms() override {
            return R"(
{
    vec4 color; // color
}
)";
        }

        const char * other_functions() override {
            return R"(
vec4 other_function(float t) {
    return vec4(t);
}
)";
        }

        const char * main() override {
            return R"(
(vec3 uv, float time) {
    return color_sampler_color;
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
            _uni_color_loc = glGetUniformLocation(program,
                                    "color_sampler_color");
        }

        void on_upload_uniforms() override {
            glUniform4f(_uni_color_loc, color.r, color.g,
                        color.b, color.a);
        }

    private:
        GLint _uni_color_loc = -1;

    public:
        color_t color;
        color_sampler_tag() : color{1.0, 1.0, 1.0, 1.0} {}
        explicit color_sampler_tag(color_t $color) : color($color) {}
        color_sampler_tag(float r, float g, float b, float a) : color{r, g, b, a} {}};
}