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
    using blend_mode_t = const char *;
    struct blend_modes {

        static const char * Normal() {
                return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
    return s;
}
)";
        }

        static const char * Multiply() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
    return s*b;
})";
        };

        static const char * Overlay() {
            return R"(
float __blend_overlay(float s, float b) {
	return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_overlay(s.r, b.r),
	            __blend_overlay(s.g, b.g),
	            __blend_overlay(s.b, b.b));
}
)";
        }

        static const char * Divide() {
            return R"(
float __blend_divide(float s, float b) {
	return (b==0.0) ? 1.0 : (b/s);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_divide(s.r, b.r),
	            __blend_divide(s.g, b.g),
	            __blend_divide(s.b, b.b));
}
)";
        };

        static const char * Screen() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(1.0) - ((vec3(1.0)-b)*(vec3(1.0)-s));
}
)";
        };

        static const char * Darken() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return min(s, b);
}
            )";
        };

        static const char * Lighten() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return max(s, b);
}
)";
        };

        static const char * ColorDodge() {
            return R"(
float __blend_ColorDodge(float s, float b) {
	return (s==1.0) ? s : min(b/(1.0-s), 1.0);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_ColorDodge(s.r,b.r),
	            __blend_ColorDodge(s.g,b.g),
	            __blend_ColorDodge(s.b,b.b));
}
)";
        };

        static const char * ColorBurn() {
            return R"(
float __blend_ColorBurn(float s, float b) {
	return (s==0.0) ? s : max((1.0-((1.0-b)/s)), 0.0);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_ColorBurn(s.r,b.r),
	            __blend_ColorBurn(s.g,b.g),
	            __blend_ColorBurn(s.b,b.b));
}
            )";
        };

        static const char * HardLight() {
            return R"(
float __blend_overlay(float s, float b) {
	return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_overlay(b.r, s.r),
	            __blend_overlay(b.g, s.g),
	            __blend_overlay(b.b, s.b));
}
)";
        };

        static const char * SoftLight() {
            return R"(
float __blend_SoftLight(float s, float b) {
	return (s<0.5) ? (2.0*b*s+b*b*(1.0-2.0*s)) : (sqrt(b)*(2.0*s-1.0)+2.0*b*(1.0-s));
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_SoftLight(s.r, b.r),
	            __blend_SoftLight(s.g, b.g),
	            __blend_SoftLight(s.b, b.b));
}
)";
        };

        static const char * Difference() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return abs(s-b);
}
)";
        };

        static const char * Exclusion() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return b+s-2.0*b*s;
}
)";
        };

        static const char * LinearBurn() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return max(b+s-vec3(1.0), vec3(0.0));
}
)";
        };

        static const char * LinearDodge() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return min(b+s, vec3(1.0));
}
)";
        };

        static const char * LinearLight() {
            return R"(
float __blend_LinearBurn(float s, float b) {
	return max(b+s-1.0, 0.0);
}

float __blend_LinearLight(float s, float b) {
	return (s < 0.5) ? __blend_LinearBurn(2.0*s, b) : __blend_LinearBurn(2.0*(s-0.5), b);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_LinearLight(s.r, b.r),
	            __blend_LinearLight(s.g, b.g),
	            __blend_LinearLight(s.b, b.b));
}
)";
        };

        static const char * Negation() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(1.0)-abs(vec3(1.0)-b-s);
}
)";
        };

        static const char * PinLight() {
            return R"(
float __blend_PinLight(float b, float s) {
	return (s < 0.5) ? min(b,(2.0*s)) : max(b,(2.0*(s-0.5)));
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_PinLight(s.r, b.r),
	            __blend_PinLight(s.g, b.g),
	            __blend_PinLight(s.b, b.b));
}
)";
        };

        static const char * Reflect() {
            return R"(
float __blend_Reflect(float s, float b) {
	return (s==1.0) ? s : min(b*b/(1.0-s), 1.0);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_Reflect(s.r, b.r),
	            __blend_Reflect(s.g, b.g),
	            __blend_Reflect(s.b, b.b));
}
)";
        };

        static const char * Subtract() {
            return R"(
vec3 __BLEND(in vec3 s, in vec3 b) {
	return max(b+s-vec3(1.0), vec3(0.0));
}
)";
        };

        static const char * VividLight() {
            return R"(
float __blend_ColorDodge(float s, float b) {
	return (s==1.0) ? s : min(b/(1.0-s), 1.0);
}

float __blend_ColorBurn(float s, float b) {
	return (s==0.0) ? s : max((1.0-((1.0-b)/s)), 0.0);
}

float __blend_VividLight(float s, float b) {
	return (s < 0.5) ? __blend_ColorBurn(2.0*s, b) : __blend_ColorDodge(2.0*(s-0.5), b);
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_VividLight(s.r, b.r),
	            __blend_VividLight(s.g, b.g),
	            __blend_VividLight(s.b, b.b));
}
)";
        };

        static const char * HardMix() {
            return R"(
float __blend_ColorDodge(float s, float b) {
	return (s==1.0) ? s : min(b/(1.0-s), 1.0);
}

float __blend_ColorBurn(float s, float b) {
	return (s==0.0) ? s : max((1.0-((1.0-b)/s)), 0.0);
}

float __blend_VividLight(float s, float b) {
	return (s < 0.5) ? __blend_ColorBurn(2.0*s, b) : __blend_ColorDodge(2.0*(s-0.5), b);
}

float __blend_HardMix(float s, float b) {
	return (__blend_VividLight(s,b)<0.5) ? 0.0 : 1.0;
}

vec3 __BLEND(in vec3 s, in vec3 b) {
	return vec3(__blend_HardMix(s.r, b.r),
	            __blend_HardMix(s.g, b.g),
	            __blend_HardMix(s.b, b.b));
}
)";
        };



    };

}