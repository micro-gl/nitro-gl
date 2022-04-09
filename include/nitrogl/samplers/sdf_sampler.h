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

    struct sdf_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
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
    vec2 uv1 = uv.xy - vec2(0.5f);
    float r = 0.5;
    float d = length(uv1) - r;
    vec4 col = (d>0.0) ? vec4(0.,0.,0., 0.) : vec4(0.65,0.85,0.0,1.);
    return vec4(col);
//    return vec4(uv1, 0.0, 1.0);
    return vec4(vec3(uv1.y), 1.0);
//    return data.color;
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
//            GLint loc = get_uniform_location(program, "color");
//            glUniform4f(loc, color.r, color.g, color.b, color.a);
        }

    public:
        color_t color;
        float radius;
        sdf_sampler() : radius(0.5), color{0.0, 1.0, 1.0, 1.0}, base() {

        }
    };
}