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
// co = αs x Fa x Cs + αb x Fb x Cb
// a0 = αs x Fa + αb x Fb
// result is pre-multiplied alpha color
vec4 __internal_porter_duff(float Fa, float Fb, vec4 s, vec4 b) {
    vec4 result;
    result.a = s.a * Fa + b.a * Fb;
    result.rgb = (s.a * Fa * s.rgb + b.a * Fb * b.rgb);
    return result;
}
)";
        }

        static const char * Clear() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(0.0, 0.0, s, b);
}
)";
        }

        static const char * Copy() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0, 0.0, s, b);
}
)";
        }

        static const char * Destination() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(0.0, 1.0, s, b);
}
)";
        }

        static const char * Source() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0, 0.0, s, b);
}
)";
        }

        static const char * SourceOver() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0, 1.0 - s.a, s, b);
}
)";
        }

        static const char * SourceOverOpaque() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return vec4((s.a * s.rgb + (1.0 - s.a) * b.rgb), 1.0);
}
)";
        }


        static const char * SourceIn() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(b.a, 0.0, s, b);
}
)";
        }

        static const char * SourceOut() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0 - b.a, 0.0, s, b);
}
)";
        }

        static const char * SourceAtop() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(b.a, 1.0 - s.a, s, b);
}
)";
        }

        static const char * DestinationOver() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0 - b.a, 1.0, s, b);
}
)";
        }


        static const char * DestinationIn() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(0.0, s.a, s, b);
}
)";
        }


        static const char * DestinationOut() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(0.0, 1.0 - s.a, s, b);
}
)";
        }


        static const char * DestinationAtop() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0 - b.a, s.a, s, b);
}
)";
        }

        static const char * XOR() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0 - b.a, 1.0 - s.a, s, b);
}
)";
        }

        static const char * Lighter() {
            return R"(
vec4 __COMPOSITE(vec4 s, vec4 b) {
    return __internal_porter_duff(1.0, 1.0, s, b);
}
)";
        }


    };

}