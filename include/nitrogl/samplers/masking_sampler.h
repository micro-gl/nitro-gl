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
#include <nitrogl/masks.h>

namespace nitrogl {

    /**
     * Masking sampler. Given two samplers, one is a source sampler and another
     * acts as a mask. This will mask the first sampler with the second sampler.
     * One can also specify what channel in the mask to use, the chrome mode:
     * - red/green/blue/alpha channels or even inverted channels
     * @tparam chrome Enum to specify the chrome mode
     */
    template<nitrogl::masks::chrome_mode chrome=nitrogl::masks::chrome_mode::alpha_channel>
    struct masking_sampler : public multi_sampler<2> {
        using base = multi_sampler<2>;
        const char * name() const override { return "masking_sampler"; }
        const char * uniforms() const override {
            return nullptr;
        }

        const char * other_functions() const override {
            return R"(
vec4 other_function(float t) {
    return vec4(t);
}
)";
        }

        const char * main() const override {
            if(chrome==masks::chrome_mode::alpha_channel) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= mask.a;
    return base;
})";
            } else if(chrome==masks::chrome_mode::alpha_channel_inverted) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (1.0 - mask.a);
    return base;
})";
            } else if(chrome==masks::chrome_mode::red_channel) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (mask.r);
    return base;
})";
            } else if(chrome==masks::chrome_mode::red_channel_inverted) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (1.0 - mask.r);
    return base;
})";
            } else if(chrome==masks::chrome_mode::green_channel) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (mask.g);
    return base;
})";
            } else if(chrome==masks::chrome_mode::green_channel_inverted) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (1.0 - mask.g);
    return base;
})";
            } else if(chrome==masks::chrome_mode::blue_channel) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (mask.b);
    return base;
})";
            } else if(chrome==masks::chrome_mode::blue_channel_inverted) {
                return R"(
(vec3 uv) {
    vec4 base = sampler_00(uv);
    vec4 mask = sampler_01(uv);
    base.a *= (1.0 - mask.b);
    return base;
})";
            }

        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
        }

    public:
        /**
         *
         * @param what_to_mask the sampler to mask
         * @param mask the mask sampler
         */
        masking_sampler(sampler_t * what_to_mask, sampler_t * mask) :
                    base(what_to_mask, mask) {
        }
    };
}