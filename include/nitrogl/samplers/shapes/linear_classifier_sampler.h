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
     * A Linear Classifier sampler
     * 1. Define a line of seperation
     * 2. Define two samplers (left/right)
     * 3. Define A boundary width for mixing
     *
     */
    struct linear_classifier_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "rounded_rect_sampler"; }
        const char * uniforms() const override {
            return R"(
{
    // p0_x, p0_y, p1_x, p1_y, boundary_band
    float inputs[5];
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
    float boundary_width = data.inputs[4];
    vec2 p = uv.xy;

    /////////////
    // SDF function
    /////////////
    vec2 pa = p-a, ba = b-a;
    float h = dot(pa, ba)/dot(ba, ba);
    float d = sign(dot(pa, vec2(ba.y, -ba.x)))*length(pa - ba*h);
    d = max(0.0, d);
    // negative distances will become zero
    float alpha = smoothstep(0.0, boundary_width, d);
    vec4 col = mix(sampler_00(uv), sampler_01(uv), alpha);
    return col;
}
)";
        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
            float inputs[5] = { p0.x, p0.y, p1.x, p1.y, boundary_width };
            GLint loc_inputs = get_uniform_location(program, "inputs");
            glUniform1fv(loc_inputs, 5, inputs);
        }

    public:
        vec2f p0, p1;
        float boundary_width;

        /**
         *
         * @param sampler_left Left sampler
         * @param sampler_right Right sampler
         * @param p0 [0..1]x[0..1], 1st point
         * @param p1 [0..1]x[0..1], 2nd point
         * @param boundary_width [0..1], boundary window for mixing a to b
         */
        linear_classifier_sampler(sampler_t * sampler_left, sampler_t * sampler_right,
                                  const vec2f & p0, const vec2f & p1,
                                  float boundary_width=0.1f) :
                        p0(p0), p1(p1), boundary_width(boundary_width), base(sampler_left, sampler_right) {
        }
    };
}