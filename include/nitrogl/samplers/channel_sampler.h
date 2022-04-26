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
#include <nitrogl/channels.h>
#include <nitrogl/traits.h>

namespace nitrogl {

    /**
     * Samples a specific channel and copies it to all channels.
     * - You can sample red/green/blue/alpha and inverted channels
     * - suppose alpha channel is sampled: (1, 0.2, 0, 0.5) -> (0.5, 0.5, 0.5, 0.5)
     */
    struct channel_sampler : public multi_sampler<1> {
        using base = multi_sampler<1>;
        using channel_t = nitrogl::channels::channel;
        const char * name() const override { return "channel_sampler"; }
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
            switch (channel) {

                case channel_t::red_channel:
                    return R"(
(vec3 uv) {
    return vec4(sampler_00(uv).r);
})";

                case channel_t::green_channel:
                    return R"(
(vec3 uv) {
    return vec4(sampler_00(uv).g);
})";
                case channel_t::blue_channel:
                    return R"(
(vec3 uv) {
    return vec4(sampler_00(uv).b);
})";
                case channel_t::alpha_channel:
                    return R"(
(vec3 uv) {
    return vec4(sampler_00(uv).a);
})";
                case channel_t::red_channel_inverted:
                    return R"(
(vec3 uv) {
    return vec4(1.0 - sampler_00(uv).r);
})";
                case channel_t::green_channel_inverted:
                    return R"(
(vec3 uv) {
    return vec4(1.0 - sampler_00(uv).g);
})";
                case channel_t::blue_channel_inverted:
                    return R"(
(vec3 uv) {
    return vec4(1.0 - sampler_00(uv).b);
})";
                case channel_t::alpha_channel_inverted:
                    return R"(
(vec3 uv) {
    return vec4(1.0 - sampler_00(uv).a);
})";
            }

        }

        void on_cache_uniforms_locations(GLuint program) override {
        }

        void on_upload_uniforms_request(GLuint program) override {
        }

    public:
        channel_t channel;

        /**
         *
         * @param sampler A sampler we want to sample a channel from
         * @param channel the channel enum specifier { red, green, blue, alpha, red_inverted, etc..}
         */
        explicit channel_sampler(sampler_t * sampler,
                                 channel_t channel = channel_t::alpha_channel) :
                                 channel(channel), base(sampler) {
        }
    };
}