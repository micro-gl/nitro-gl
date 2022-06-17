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

    /**
     * A Sampler, that tints another sampler
     */
    struct tint_sampler : public multi_sampler<1> {
        using base = multi_sampler<1>;
        const char * name() const override { return "tint_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    vec4 color;
}
)";
        }

        const char * main() const override {
            return R"(
(in vec3 uv) {
    return sampler_00(uv)*data.color;
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            GLint loc = get_uniform_location(program, "color");
            glUniform4f(loc, color.r, color.g, color.b, color.a);
        }

        color_t color;

        explicit tint_sampler(const color_t & tint_color, sampler_t * sampler) :
            color(tint_color), base(sampler) {}
    };
}