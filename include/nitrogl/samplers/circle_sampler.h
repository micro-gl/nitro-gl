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

    struct circle_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "circle_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // radius, stroke-width, aa_fill, aa_stroke
    float inputs[4];
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
//    // move to origin data.
//    float r = 0.5;
//    //    float pix = 1.0/250.0;
//    float sw = 0.01/2.;//10.0/2 * pix; // stroke width,  divide by 2
//    float aa_b = 0.04;//2.0*pix; // aa boundary, mul by 2 for more beautiful

    /////////////
    // inputs
    /////////////
    // radius
    float r = data.inputs[0];
    // stroke width,  divide by 2
    float sw = data.inputs[1]/2.0;
    // aa fill and stroke, mul by 2 for more beautiful
    float aa_fill = data.inputs[2]*2.0;
    float aa_stroke = data.inputs[3]*2.0;

    /////////////
    // SDF function
    /////////////
    vec2 xy = uv.xy - vec2(0.5f);
    float d = length(xy) - r; // sdf, signed distance to shape's boundary

    /////////////
    // inner circle with AA at the boundary
    /////////////
    vec4 col_base = sampler_00(uv);
//    col_base.a *= (1.0 - smoothstep(0.0, 0.0 + aa_fill, d) );

    /////////////
    // mix stroke
    /////////////
    vec4 col_src = sampler_01(uv);
    col_src.a *= (1.0 - smoothstep(sw, sw + aa_stroke, abs(d) ));

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
            float inputs[4] = { radius, stroke_width, aa_fill, aa_stroke };
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, 4, inputs);
        }

    public:
        float radius;
        float stroke_width;
        float aa_fill, aa_stroke;

        template <class... Ts>
        circle_sampler(float radius=0.5f, float stroke_width=0.01f,
                       float aa_fill=0.01f, float aa_stroke=0.01f, Ts... rest) :
                        radius(radius), stroke_width(stroke_width), aa_fill(aa_fill),
                        aa_stroke(aa_stroke),
                        base(rest...) {
        }
    };
}