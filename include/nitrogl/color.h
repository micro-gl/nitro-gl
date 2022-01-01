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

#include "stdint.h"

namespace nitrogl {

//    struct color_t {
//        using channel_t = unsigned char;
//        color_t(channel_t $r=0, channel_t $g=0, channel_t $b=0, channel_t $a=255) :
//                r{$r}, g{$g}, b{$b}, a{$a} {};
//        color_t & operator=(const color_t & rhs) {
//            r=rhs.r; g=rhs.g; b=rhs.b; a=rhs.a;
//            return (*this);
//        }
//        channel_t r, g, b, a;
//    };

    struct color_t {
        float r, g, b, a;
    };
//
//    /**
//     * convert a color of one depth space into another
//     *
//     * @tparam rgba_from the rgba_t info of the input color
//     * @tparam rgba_to the rgba_t info of the output color
//     * @param input input color
//     * @param output output color
//     */
//    template<typename rgba_from, typename rgba_to>
//    void convert_color(const color_t &input, color_t &output) {
//        output.r = convert_channel_correct<rgba_from::r, rgba_to::r>(input.r);
//        output.g = convert_channel_correct<rgba_from::g, rgba_to::g>(input.g);
//        output.b = convert_channel_correct<rgba_from::b, rgba_to::b>(input.b);
//        output.a = convert_channel_correct<rgba_from::a, rgba_to::a>(input.a);
//    }
//
//    /**
//     * convert an intensity to color, example :
//     *
//     * Example: intensity<float>(0.5, 1.0, 1.0f) -> color_t{128, 255, 255}
//     *
//     * @tparam number the underlying type of the intensity
//     * @tparam rgba the rgba_t info we desire to quantize
//     * @param input the input intensity
//     * @param output output color
//     */
//    template <typename number, typename rgba>
//    void convert_intensity_to_color(const intensity<number> &input, color_t &output) {
//        output.r = channel_t(number((1u << rgba::r) - 1) * input.r);
//        output.g = channel_t(number((1u << rgba::g) - 1) * input.g);
//        output.b = channel_t(number((1u << rgba::b) - 1) * input.b);
//        output.a = channel_t(number((1u << rgba::a) - 1) * input.a);
//    }
//
//    /**
//     * convert a color into intensity object
//     *
//     * Example: color_t{128, 255, 255} -> intensity<float>(0.5, 1.0, 1.0f)
//     *
//     * @tparam number number the underlying type of the intensity
//     * @tparam rgba the rgba_t info of the color
//     * @param input input color
//     * @param output output intensity
//     */
//    template <typename number, typename rgba>
//    void convert_color_to_intensity(const color_t &input, intensity<number> &output) {
//        output.r = number(input.r)/number((1u << rgba::r) - 1);
//        output.g = number(input.g)/number((1u << rgba::g) - 1);
//        output.b = number(input.b)/number((1u << rgba::b) - 1);
//        output.a = number(input.a)/number((1u << rgba::a) - 1);
//    }
}
