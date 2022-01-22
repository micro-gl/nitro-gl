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

namespace nitrogl {
    using compositor = const char *;

    struct porter_duff {

        static const char * base() {
            return R"(
// a0 = αs x Fa + αb x Fb
// co = αs x Fa x Cs + αb x Fb x Cb
// according to PDF spec, page 322, if we use source-over
// result is NOT alpha pre-multiplied color
vec4 __internal_porter_duff(float Fa, float Fb, vec4 s, vec4 b) {
    vec4 result;
    float as = s.a;
    float ab = b.a;
    vec3 Cs = s.rgb;
    vec3 Cb = b.rgb;

    result.a = as * Fa + ab * Fb;//alpha;

//    if(!areEqual(result.a , 0.0)) {
        // unmultiply alpha since the Porter-Duff equation results
        // in pre-multiplied alpha colors
    result.rgb = (as * Fa * Cs + ab * Fb * Cb);
//    result.rgb /= result.a;

//    }

    return result;
}

)";
        }

        static const char * source_over() {
            return R"(
vec3 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0, 1.0 - s.a, s, b);
})";
        }

    }

}