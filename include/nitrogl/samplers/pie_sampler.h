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
#include <nitrogl/math.h>

namespace nitrogl {

    /**
     * Arc sampler, that uses SDF functions to draw a rounded Arc at
     * the center of the sample space [0..1]x[0..1] quad.
     * Currently, the drawn onto polygon should be a quad as well to avoid
     * any stretching effects.
     */
    struct pie_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "rounded_rect_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // (ax, ay, bx, by, radius, stroke-width, is_convex, aa_fill, aa_stroke)
    float inputs[9];
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
    vec2 a = vec2(data.inputs[0], data.inputs[1]);
    vec2 b = vec2(data.inputs[2], data.inputs[3]);
    float r = data.inputs[4];
    // stroke width,  divide by 2
    float sw = data.inputs[5]/2.0;
    // aa fill and stroke, mul by 2 for more beautiful
    float convex = data.inputs[6];
    float aa_fill = data.inputs[7]*2.0;
    float aa_stroke = data.inputs[8]*2.0;
    vec2 p = uv.xy - vec2(0.5f);

    /////////////
    // SDF function
    /////////////
//    float PI = 3.1415926538;
//    float th1 = 0.0*PI/180.0;
//    float th2 = 290*PI/180.0;//(200.0+45)*PI/180.0;
//    a = vec2(cos(th1), sin(th1));
//    b = vec2(cos(th2), sin(th2));
//    convex = sign(b.y*a.x-b.x*a.y);

    // circle sdf
    float sdf_circle = length(p) - r;
    // cone
	float m1 = length(p - a*clamp(dot(p,a), 0.0, r) );
	float m2 = length(p - b*clamp(dot(p,b), 0.0, r) );
    float s1 = sign(a.x*p.y - a.y*p.x); // p left of a
    float s2 = sign(b.x*p.y - b.y*p.x); // p left of b
    bool in_cone = ((convex>0.0) && (s1>=0.0 && s2<0.0)) ||
                   ((convex<=0.0) && (s1>=0.0 || s2<0.0));
    float sig = in_cone ? -1.0 : 1.0;
    float sdf_open_cone = min(m1, m2)*sig;

    float d = max(sdf_circle, sdf_open_cone);

//    return vec4(1,0,0, inside ? 1.0:0.0f);
//    return vec4(uv.x, uv.x, uv.x, 1);
//    return vec4(uv.y, uv.y, uv.y, 1);


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
            // normalized angle unit vectors
            const auto two_pi = nitrogl::math::pi<float>()*2.0f;
            from_angle = nitrogl::math::mod(from_angle, two_pi);
            to_angle = nitrogl::math::mod(to_angle, two_pi);

            float ax = nitrogl::math::cos(from_angle);
            float ay = nitrogl::math::sin(from_angle);
            float bx = nitrogl::math::cos(to_angle);
            float by = nitrogl::math::sin(to_angle);

            float is_convex = (ax*by - ay*bx); // b is left-of a
            float inputs[9] = { ax, ay, bx, by, radius, stroke_width,
                                 is_convex, aa_fill, aa_stroke };
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, 9, inputs);
        }

    public:
        float radius;
        float stroke_width;
        float aa_fill, aa_stroke;
        float from_angle, to_angle;

        /**
         *
         * @param fill A fill sampler
         * @param stroke A stroke sampler
         * @param from_angle in radians
         * @param to_angle in radians
         * @param radius radius of the arc from the center
         * @param stroke_width stroke width
         * @param aa_fill AA strength for fill
         * @param aa_stroke  AA strength for stroke
         */
        pie_sampler(sampler_t * fill, sampler_t * stroke,
                    float from_angle, float to_angle,
                    float radius=0.5f, float stroke_width=0.01f,
                    float aa_fill=0.01f, float aa_stroke=0.01f) :
                        from_angle(from_angle), to_angle(to_angle),
                        radius(radius), stroke_width(stroke_width),
                        aa_fill(aa_fill), aa_stroke(aa_stroke), base(fill, stroke) {
        }
    };
}