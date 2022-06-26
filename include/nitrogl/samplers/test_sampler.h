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
     * test sampler to test uv coords
     * @tparam horizontal
     */
    template<bool horizontal=true>
    struct test_sampler : public sampler_t {
        test_sampler() : sampler_t() {}

        const char * name() const override { return "test_sampler"; }

        const char * main() const override {
            if(horizontal) {
                return R"(
(in vec3 uv) {
    return vec4(uv.x, uv.x, uv.x, 1.0);
}
)";
            } else {
                return R"(
(in vec3 uv) {
    return vec4(uv.y, uv.y, uv.y, 1.0);
}
)";
            }
        }

    };

}