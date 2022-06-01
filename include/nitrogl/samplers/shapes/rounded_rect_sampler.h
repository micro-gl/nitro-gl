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

namespace nitrogl {

    /**
     * A sampler, that uses SDF functions to draw a rounded rectangle at
     * the center of the sample space [0..1]x[0..1] quad.
     * Currently, the drawn onto polygon should be a quad as well to avoid
     * any stretching effects.
     * NOTE:
     * - given normalized w,h,r. it will render a rounded rect_i with base of
     *   dimensions WxH centered and the radius is extended outside of this
     *   base boundary
     * - given, that you need to render it on a perfect polygon quad, it might
     *   be inefficient because of a lot of dead pixels
     */
    struct rounded_rect_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "rounded_rect_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // x1, y1, x2, y2, radius, stroke-width, aa_fill, aa_stroke
    float inputs[6];
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

    /////////////
    // inputs
    /////////////
    vec2 a = vec2(data.inputs[0], data.inputs[1])/2.0;
    float r = data.inputs[2]/1.0;
    // stroke width,  divide by 2
    float sw = data.inputs[3]/2.0;
    // aa fill and stroke, mul by 2 for more beautiful
    float aa_fill = data.inputs[4]*2.0;
    float aa_stroke = data.inputs[5]*2.0;

    /////////////
    // SDF function
    /////////////
    vec2 p = (uv.xy - 0.5f);
    vec2 d2 = abs(p) - a;
    float d = length(max(d2,0.0)) + min(max(d2.x,d2.y),0.0) - r;//+ 0.34;

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
    return vec4(col.rgb/col.a, col.a); // un-multiply alpha
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            float inputs[6] = { w, h, radius, stroke_width, aa_fill, aa_stroke };
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, 6, inputs);
        }

    public:
        float w, h;
        float radius;
        float stroke_width;
        float aa_fill, aa_stroke;

        /**
         *
         * @param fill A fill sampler
         * @param stroke A stroke sampler
         * @param w [0..1], where base rectangle minus radius width
         * @param h [0..1], where base rectangle minus radius height
         * @param radius [0..1], radius extended from base rectangle
         * @param stroke_width stroke width [0..1]
         * @param aa_fill boundary anti-alias band for shape fill [0..1]
         * @param aa_stroke boundary anti-alias band for shape stroke [0..1]
         * @param rest samplers pointers for fill and stroke
         */
        rounded_rect_sampler(sampler_t * fill, sampler_t * stroke,
                             float w, float h,
                             float radius=0.5f, float stroke_width=0.01f,
                             float aa_fill=0.01f, float aa_stroke=0.01f) :
                             w(w), h(h), radius(radius), stroke_width(stroke_width),
                             aa_fill(aa_fill), aa_stroke(aa_stroke), base(fill, stroke) {
        }
    };
}