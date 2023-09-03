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
#include <nitrogl/math/vertex2.h>

namespace nitrogl {

    /**
     * A Capsule Shape Sampler, maps perfectly to a [0,1]x[0,1] quad
     *
     */
    struct capsule_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "rounded_rect_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // p0_x, p0_y, p1_x, p1_y, radius, stroke-width, aa_fill, aa_stroke
    float inputs[8];
}
)";
        }

        const char * main() const override {
            return R"(
(in vec3 uv) {

    /////////////
    // inputs
    /////////////
    vec2 a = vec2(data.inputs[0], data.inputs[1]);
    vec2 b = vec2(data.inputs[2], data.inputs[3]);
    float r = data.inputs[4];
    // stroke width,  divide by 2
    float sw = data.inputs[5]/2.0;
    // aa fill and stroke, mul by 2 for more beautiful
    float aa_fill = data.inputs[6]*2.0;
    float aa_stroke = data.inputs[7]*2.0;
    vec2 p = uv.xy;//-0.5f;

    /////////////
    // SDF function
    /////////////
    vec2 pa = p-a, ba = b-a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    float d = length(pa - ba*h) - r;

    /////////////
    // inner circle with AA at the boundary
    /////////////
    vec4 col_base = sampler_00(uv);
    col_base.a *= (1.0 - smoothstep(0.0, 0.0 + aa_fill, d) );

    /////////////
    // mix stroke
    /////////////
    vec4 col_src = sampler_01(uv);
    col_src.a *= (1.0 - smoothstep(sw, sw + aa_stroke, abs(d) ));

    /////////////
    // source-over compositing stroke over circle
    /////////////
    vec4 col = __internal_porter_duff(1.0, 1.0-col_src.a, col_src, col_base);
    col.rgb /= col.a;
    return clamp(col, 0.0, 1.0);
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            float inputs[8] = { p0.x, p0.y, p1.x, p1.y, radius, stroke_width, aa_fill, aa_stroke };
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, 8, inputs);
        }

    public:
        vec2f p0, p1;
        float radius;
        float stroke_width;
        float aa_fill, aa_stroke;

        /**
         *
         * @param fill A fill sampler
         * @param stroke A stroke sampler
         * @param p0 [0..1]x[0..1], 1st point
         * @param p1 [0..1]x[0..1], 2nd point
         * @param radius [0..1], radius extended from base rectangle
         * @param stroke_width stroke width [0..1]
         * @param aa_fill boundary anti-alias band for shape fill [0..1]
         * @param aa_stroke boundary anti-alias band for shape stroke [0..1]
         * @param rest samplers pointers for fill and stroke
         */
        capsule_sampler(sampler_t * fill, sampler_t * stroke,
                        const vec2f & p0, const vec2f & p1,
                        float radius=0.5f, float stroke_width=0.01f,
                        float aa_fill=0.01f, float aa_stroke=0.01f) :
                            p0(p0), p1(p1), radius(radius), stroke_width(stroke_width),
                             aa_fill(aa_fill), aa_stroke(aa_stroke), base(fill, stroke) {
        }
    };
}