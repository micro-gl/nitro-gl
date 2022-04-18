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
#include "color_sampler.h"
#include "test_sampler.h"
#include "../traits.h"

namespace nitrogl {

    struct sdf_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "color_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    float r; // radius
    float stroke; // stroke width
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
    // move to origin
    float r = 0.5;
    float pix = 10.0/250.0;
    float sw = 10/2 * pix; // stroke width
    float aa_b = 2.0*pix; // aa boundary

    // SDF function
    vec2 xy = uv.xy - vec2(0.5f);
    float d = length(xy) - r; // sdf, signed distance to shape's boundary

    /////////////
    // inner circle with AA at the boundary
    /////////////
    vec4 col_base = sampler_00(uv);
    col_base.a *= (1.0 - smoothstep(0.0, 0.0 + aa_b, d) );

    /////////////
    // mix stroke
    /////////////
    vec4 col_src = sampler_01(uv);
    col_src.a *= (1.0 - smoothstep(sw, sw + aa_b, abs(d) ));

    /////////////
    // source-over compositing stroke over circle
    /////////////
    vec4 col = __internal_porter_duff(1.0, 1.0-col_src.a, col_src, col_base);
    return vec4(col.rgb/col.a, col.a); // un-multiply alpha
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
//            GLint loc = get_uniform_location(program, "color");
//            glUniform4f(loc, color.r, color.g, color.b, color.a);
        }

        color_sampler _color_void;
        color_sampler _color_void_2;
        test_sampler<> _sampler_test;
    public:
        color_t color;
        float radius;
        sdf_sampler(float r=0.5f) : radius(r),
        _color_void{0.0, 1.0, 0.0, 1.0}, _color_void_2{0.0, 1.0, 0.0, 1.0}, base() {
            add_sub_sampler(&_sampler_test);
            add_sub_sampler(&_color_void_2);
        }
    };
}