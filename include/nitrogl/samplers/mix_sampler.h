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
#include "../traits.h"

namespace nitrogl {

    struct mix_sampler : public multi_sampler<2> {
        const char * name() const override { return "mix_sampler"; }
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
            return R"(
(vec3 uv) {
    return (sampler_0(uv) + sampler_1(uv))/2.0;
}
            )";
        }

        color_sampler sampler_1{1.0, 0.0, 0.0, 1.0};
        color_sampler sampler_2{0.0, 1.0, 0.0, 1.0};

    public:
        mix_sampler() : multi_sampler<2>() {
            add_sub_sampler(&sampler_1);
            add_sub_sampler(&sampler_1);
//            add_sub_sampler(&sampler_2);
        }
    };
}