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
     * A 2D Piece-wise Function sampler:
     * 1. Composed out of finite segments
     * 2. up to 100 points or 99 segments
     *
     */
    template<int MAX_POINTS=(6 + 2*100)>
    struct d1_function_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "rounded_rect_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // size, offset, window_size, stroke-width, aa_fill, aa_stroke, x,y,x,y,x,y......
    float inputs[6 + 2*100];
}
)";
        }

        const char * main() const override {
            return R"(
(in vec3 uv) {

    /////////////
    // inputs
    /////////////
    int count = int(data.inputs[0]);
    int window_size = int(data.inputs[1]);
    int offset = int(data.inputs[2]);
    // stroke width,  divide by 2
    float sw = data.inputs[3]/2.0;
    // aa fill and stroke, mul by 2 for more beautiful
    float aa_fill = data.inputs[4]*2.0;
    float aa_stroke = data.inputs[5]*2.0;
    vec2 p = uv.xy;

    /////////////
    // SDF function
    /////////////
    // compute squared SDF for each line segment. Defer sqrt() operation to
    // the end for performance reasons. I wish I could have skipped the branching,
    // BUT SDFs union/intersection ops with min/max dont work well for me.
    float d_squared = 1000000.0;
    float sign_ = 0.0;
    for(int ix=0, jx=offset; ix<count-1; ++ix, jx+=2) {
        vec2 a = vec2(data.inputs[jx], data.inputs[jx+1]);
        vec2 b = vec2(data.inputs[jx+2], data.inputs[jx+3]);
        vec2 pa = p-a, ba = b-a;
        float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0 , 1.0);
        vec2 ot = pa - ba*h; // this is the shortest vector
        float d2_squared = dot(ot, ot);
        if(d2_squared <= d_squared) {
            d_squared=d2_squared; sign_=sign(dot(pa, vec2(-ba.y, ba.x)));
        }
    }
    // defer the sqrt() function to here
    float d = sqrt(d_squared)*sign_;

    /////////////
    // inner fill with AA at the boundary
    /////////////
    vec4 col_base = sampler_00(uv);
    col_base.a *= (1.0 - smoothstep(0.0, 0.0 + aa_fill, d) );

    /////////////
    // stroke on boundary with AA
    /////////////
    vec4 col_src = sampler_01(uv);
    col_src.a *= (1.0 - smoothstep(sw, sw + aa_stroke, abs(d) ));

    /////////////
    // source-over compositing stroke over fill
    /////////////
    vec4 col = __internal_porter_duff(1.0, 1.0-col_src.a, col_src, col_base);
    col.rgb /= col.a;
    return clamp(col, 0.0, 1.0);
}
)";
        }

        constexpr int window_size() const {
            // minus the where float
            return  2;
        }
        constexpr int offset() const {
            // [0]=index, [1]=window_size, [2]=offset
            return  6;
        }
        constexpr int overall_size() const {
            return  offset() + size*window_size();
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            // minus the where float
            static float inputs[MAX_POINTS];
            inputs[0] = float(size);
            inputs[1] = float(window_size()); // window size
            inputs[2] = float(offset()); // window size
            inputs[3] = stroke_width; // window size
            inputs[4] = aa_fill; // window size
            inputs[5] = aa_stroke; // window size
            for (int ix = 0; ix < size; ++ix) {
                int jx = offset() + ix * window_size();
                inputs[jx + 0] = points[ix].x;
                inputs[jx + 1] = points[ix].y;
            }
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, overall_size(), inputs);
        }

        vec2f * points;
        int size;
        float stroke_width;
        float aa_fill, aa_stroke;

        void update_points(vec2f * $points, int $size) {
            points=$points; size=$size;
        }

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
        d1_function_sampler(sampler_t * fill, sampler_t * stroke,
                        vec2f * points=nullptr, int size=0,
                        float stroke_width=0.01f,
                        float aa_fill=0.01f, float aa_stroke=0.01f) :
                        points(points), size(size), stroke_width(stroke_width),
                        aa_fill(aa_fill), aa_stroke(aa_stroke), base(fill, stroke) {
        }
    };
}