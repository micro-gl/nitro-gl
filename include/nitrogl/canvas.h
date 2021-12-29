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

#include "nitrogl/math/rect.h"
#include "color.h"
#include "traits.h"
#include "masks.h"
#include "math.h"
#include "stdint.h"
#include "math/vertex2.h"
#include "math/mat3.h"
//#include "porter_duff/None.h"
//#include "blend_modes/Normal.h"
#ifndef MICROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/triangles.h"
#include "micro-tess/include/micro-tess/polygons.h"
#include "micro-tess/include/micro-tess/path.h"
#include "micro-tess/include/micro-tess/monotone_polygon_triangulation.h"
#include "micro-tess/include/micro-tess/ear_clipping_triangulation.h"
#include "micro-tess/include/micro-tess/bezier_patch_tesselator.h"
#include "micro-tess/include/micro-tess/dynamic_array.h"
#else
#include <micro-tess/triangles.h>
#include <micro-tess/polygons.h>
#include <micro-tess/path.h>
#include <micro-tess/monotone_polygon_triangulation.h>
#include <micro-tess/ear_clipping_triangulation.h>
#include <micro-tess/bezier_patch_tesselator.h>
#include <micro-tess/dynamic_array.h>
#endif
#include "functions/minmax.h"
#include "functions/clamp.h"
#include "functions/swap.h"
#include "functions/orient2d.h"
#include "functions/bits.h"
#include "functions/distance.h"
//#include "text/bitmap_font.h"

#include "ogl/gl_texture.h"

using namespace microtess::triangles;
using namespace microtess::polygons;
//using namespace nitrogl;

namespace nitrogl {

    class canvas {
    public:
        using rect = nitrogl::rect_t<int>;
        using index = unsigned int;
        using precision = unsigned char;
        using opacity_t = unsigned char;

        // rasterizer integers
        using rint_big = int64_t;
        struct window_t {
            rect canvas_rect;
            rect clip_rect;
        };
    private:

        window_t _window;
        gl_texture _tex;
        gl_texture _tex_backdrop;

    public:
        explicit canvas(const gl_texture & tex) : _tex(tex) {
            updateClipRect(0, 0, _tex.width(), _tex.height());
            updateCanvasWindow(0, 0);
        }

        canvas(int width, int height) : canvas(gl_texture(width, height, nullptr, 4)) {
        }

        /**
         * update the clipping rectangle of the canvas
         *
         * @param l left distance to x=0
         * @param t top distance to y=0
         * @param r right distance to x=0
         * @param b bottom distance to y=0
         */
        void updateClipRect(int l, int t, int r, int b) { _window.clip_rect = {l, t, r, b}; }

        /**
         * where to position the bitmap relative to the canvas, this feature
         * can help with block rendering, where the bitmap is smaller than the canvas
         * dimensions.
         *
         * @param left relative to x=0
         * @param top relative to y=0
         * @param right relative to x=0
         * @param bottom relative to y=0
         */
        void updateCanvasWindow(int left, int top, int right, int bottom) {
            _window.canvas_rect = {left, top, left + right, top + bottom };
            if(_window.clip_rect.empty()) _window.clip_rect=_window.canvas_rect;
        }

        void updateCanvasWindow(int left, int top) {
            updateCanvasWindow(left, top, _tex.width(), _tex.height());
        }

        /**
         * given that we know the canvas size and the clip rect, calculate
         * the sub rectangle (intersection), where drawing is visible
         */
        rect calculateEffectiveDrawRect() const {
            rect r = _window.canvas_rect.intersect(_window.clip_rect);
            r.bottom-=1;r.right-=1;
            return r;
        }

        /**
         * get the clipping rectangle
         */
        const rect & clipRect() const { return _window.clip_rect; }

        /**
         * get the canvas rectangle, should be (0, 0, width, height), unless
         * the sub windowing feature was used.
         * @return a rectangle
         */
        const rect & canvasWindowRect() const { return _window.canvas_rect; }

        // get canvas width
        int width() const { return _window.canvas_rect.width(); };
        // get canvas height
        int height() const { return _window.canvas_rect.height(); };
        // get the pixels array from the underlying bitmap
//    const pixel * pixels() const;
//    pixel * pixels();
        // get a pixel by position
//    pixel getPixel(int x, int y) const ;
//    pixel getPixel(int index) const ;
        // decode pixel color by position
//        void getPixelColor(int index, color_t & output) const {}
//        void getPixelColor(int x, int y, color_t & output) const {}

//    bitmap_type & bitmapCanvas() const;

        /**
         * clear the canvas with a color intensity
         * @tparam number the number type of the intensity
         * @param color the color intensity
         */
        template <typename number>
        void clear(const color_t<number> &color);

//        // integer blenders
//        /**
//         * blend and composite a given color at position to the backdrop of the canvas
//         *
//         * @tparam BlendMode the blend mode type
//         * @tparam PorterDuff the alpha compositing type
//         * @tparam a_src the bits of the alpha channel of the color
//         * @param val the color to blend
//         * @param index the position of where to compose in the canvas
//         * @param opacity 8 bit opacity [0..255]
//         */
//        template<typename BlendMode=blendmode::Normal,
//                typename PorterDuff=porterduff::FastSourceOverOnOpaque,
//                uint8_t a_src>
//        void blendColor(const color_t &val, int x, int y, opacity_t opacity);
//
//        template<typename BlendMode=blendmode::Normal,
//                typename PorterDuff=porterduff::FastSourceOverOnOpaque,
//                uint8_t a_src>
////    __attribute__((noinline))
//        static void blendColor(const color_t &val, int index, opacity_t opacity, canvas & canva) {
//            // correct index position when window is not at the (0,0) costs one subtraction.
//            // we use it for sampling the backdrop if needed and for writing the output pixel
//            index -= canva._window.index_correction;
//
//            // we assume that the color conforms to the same pixel-coder. but we are flexible
//            // for alpha channel. if coder does not have an alpha channel, the color itself may
//            // have non-zero alpha channel, for which we emulate 8-bit alpha processing and also pre
//            // multiply result with alpha
//            constexpr bool hasBackdropAlphaChannel = pixel_coder::rgba::a != 0;
//            constexpr bool hasSrcAlphaChannel = a_src != 0;
//            constexpr uint8_t canvas_a_bits = hasBackdropAlphaChannel ? pixel_coder::rgba::a : (a_src ? a_src : 8);
//            constexpr uint8_t src_a_bits = a_src ? a_src : 8;
//            constexpr uint8_t alpha_bits = src_a_bits;
//            constexpr unsigned int alpha_max_value = uint16_t (1 << alpha_bits) - 1;
//            constexpr bool is_source_over = microgl::traits::is_same<PorterDuff, porterduff::FastSourceOverOnOpaque>::value;
//            constexpr bool none_compositing = microgl::traits::is_same<PorterDuff, porterduff::None<>>::value;
//            constexpr bool skip_blending =microgl::traits::is_same<BlendMode, blendmode::Normal>::value;
//            constexpr bool premultiply_result = !hasBackdropAlphaChannel;
//            const bool skip_all= skip_blending && none_compositing && opacity == 255;
//            static_assert(src_a_bits==canvas_a_bits, "src_a_bits!=canvas_a_bits");
//
//            const color_t & src = val;
//            static color_t result{};
//
//            if(!skip_all) {
//                pixel output;
//                static color_t backdrop{}, blended{};
//                // normal blend and none composite do not require a backdrop
//                if(!(skip_blending && none_compositing))
//                    canva._bitmap_canvas.decode(index, backdrop); // not using getPixelColor to avoid extra subtraction
//
//                // support compositing even if the surface is opaque.
//                if(!hasBackdropAlphaChannel) backdrop.a = alpha_max_value;
//
//                if(is_source_over && src.a==0) return;
//
//                // if we are normal then do nothing
//                if(!skip_blending && backdrop.a!=0) { //  or backdrop alpha is zero is also valid
//                    BlendMode::template blend<pixel_coder::rgba::r,
//                            pixel_coder::rgba::g,
//                            pixel_coder::rgba::b>(backdrop, src, blended);
//                    // if backdrop alpha!= max_alpha let's first composite the blended color, this is
//                    // an intermediate step before Porter-Duff
//                    if(backdrop.a < alpha_max_value) {
//                        // if((backdrop.a ^ _max_alpha_value)) {
//                        unsigned int comp = alpha_max_value - backdrop.a;
//                        // this is of-course a not accurate interpolation, we should
//                        // divide by 255. bit shifting is like dividing by 256 and is FASTER.
//                        // you will pay a price when bit count is low, this is where the error
//                        // is very noticeable.
//                        blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
//                        blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
//                        blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
//                    }
//                }
//                else {
//                    // skipped blending therefore use src color
//                    blended.r = src.r; blended.g = src.g; blended.b = src.b;
//                }
//
//                // support alpha channel in case, source pixel does not have
//                blended.a = hasSrcAlphaChannel ? src.a : alpha_max_value;
//
//                // I fixed opacity is always 8 bits no matter what the alpha depth of the native canvas
//                if(opacity < 255)
//                    blended.a =  (int(blended.a) * int(opacity)*int(257) + 257)>>16; // blinn method
//
//                // finally alpha composite with Porter-Duff equations,
//                // this should be zero-cost for None option with compiler optimizations
//                // if we do not own a native alpha channel, then please keep the composited result
//                // with premultiplied alpha, this is why we composite for None option, because it performs
//                // alpha multiplication
//                PorterDuff::template composite<alpha_bits, premultiply_result>(backdrop, blended, result);
//                canva.coder().encode(result, output);
//                canva._bitmap_canvas.writeAt(index, output);
//            }
//            else {
//                pixel output;
//                canva.coder().encode(val, output);
//                canva._bitmap_canvas.writeAt(index, output);
//            }
//        }


    };

}
