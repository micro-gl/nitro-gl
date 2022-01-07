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

#include "../traits.h"

namespace nitrogl {

    struct test_sampler {
        static const char * name() {
            return "test_sampler";
        }

        static const char * other() {
            return R"foo(
vec4 other_function(float t) {
    return vec4(t);
}
)foo";
        }

        static const char * main() {
            return R"foo(
(vec3 uv, float time) {
    return vec4(uv.x, uv.x, uv.x, 1.0);
}
)foo";
        }


    };

}