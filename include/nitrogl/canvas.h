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

#include "color.h"
#include "traits.h"
#include "channels.h"
#include "math.h"
#include "stdint.h"
#include "math/rect.h"
#include "math/vertex2.h"
#include "math/mat3.h"
#include "math/mat3.h"
//#include "porter_duff/None.h"
//#include "compositing/Normal.h"
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
#include "ogl/fbo.h"
#include "ogl/vbo.h"
#include "ogl/ebo.h"
#include "render_nodes/multi_render_node.h"
#include "render_nodes/p4_render_node.h"
#include "samplers/test_sampler.h"
#include "_internal/main_shader_program.h"
#include "_internal/shader_compositor.h"
#include "_internal/static_linear_allocator.h"
#include "_internal/lru_pool.h"
#include "camera.h"

// samplers
#include "samplers/sampler.h"
#include "nitrogl/samplers/shapes/circle_sampler.h"
#include "nitrogl/samplers/shapes/rounded_rect_sampler.h"
#include "samplers/color_sampler.h"
#include "samplers/channel_sampler.h"
#include "nitrogl/samplers/shapes/arc_sampler.h"
#include "nitrogl/samplers/shapes/pie_sampler.h"

#include "compositing/porter_duff.h"
#include "compositing/blend_modes.h"

using namespace microtess::triangles;
using namespace microtess::polygons;

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
        using static_alloc = micro_alloc::static_linear_allocator<char, 1<<14, 0>;
        using lru_main_shader_pool_t = microc::lru_pool<main_shader_program, 5,
                                                nitrogl::uintptr_type, static_alloc>;
        window_t _window;
        gl_texture _tex_backdrop;
        fbo_t _fbo;
        multi_render_node _node_multi;
        p4_render_node _node_p4;
        blend_mode_t _blend_mode;
        compositor_t _alpha_compositor;

        static static_alloc get_static_allocator() {
            // static allocator, shared by all canvases
            static static_alloc allocator_static;
            return allocator_static;
        }

        static lru_main_shader_pool_t & lru_main_shader_pool() {
            // shader pool is shared among all canvas instances
            static lru_main_shader_pool_t pool{0.5f, get_static_allocator()};
            // construct all of the shaders if needed
            if(!pool.are_items_constructed())
                pool.construct();
            return pool;
        }

        bool _is_pre_mul_alpha;

    private:
        //https://stackoverflow.com/questions/47173597/multisampled-fbos-in-opengl-es-3-0
        void internal_init(unsigned width, unsigned height) {
            updateClipRect(0, 0, width, height);
            updateCanvasWindow(0, 0, width, height);
            generate_backdrop();
            copy_to_backdrop();
            _node_p4.init();
            _node_multi.init();
            glCheckError();
        }

    public:
        void generate_backdrop() {
            // move
            _tex_backdrop = gl_texture::empty(width(), height(), GL_RGBA, _is_pre_mul_alpha,
                                         GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        }

        /**
         * Update the blend mode and alpha compositor(usually Porter-Duff opertor)
         */
        void update_composition(blend_mode_t blend_mode,
                                compositor_t alpha_compositor=porter_duff::SourceOver()) {
            _blend_mode=blend_mode; _alpha_compositor=alpha_compositor;
        }


        // if wants AA:
        // 1. create RBO with multisampling and attach it to color in fbo_t, and then blit to texture's fbo_t
        // if you are given texture, then draw into it
        explicit canvas(const gl_texture & tex) : _tex_backdrop(gl_texture::un_generated_dummy()),
                                                  _fbo(), _node_multi(), _node_p4(), _window(),
                                                  _is_pre_mul_alpha(tex.is_premul_alpha()),
                                                  _blend_mode(blend_modes::Normal()),
                                                  _alpha_compositor(porter_duff::SourceOver()) {
            _fbo.attachTexture(tex);
            internal_init(tex.width(), tex.height());
        }

        // if you got nothing, draw to bound fbo
        canvas(int width, int height, bool is_pre_mul_alpha=true) :
                _tex_backdrop(gl_texture::un_generated_dummy()), _fbo(fbo_t::from_current()),
                _node_multi(), _node_p4(), _window(), _is_pre_mul_alpha(is_pre_mul_alpha),
                _blend_mode(blend_modes::Normal()), _alpha_compositor(porter_duff::SourceOver()) {
            internal_init(width, height);
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
            updateCanvasWindow(left, top, width(), height());
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
        unsigned int width() const { return _window.canvas_rect.width(); };
        // get canvas height
        unsigned int height() const { return _window.canvas_rect.height(); };
        // get the pixels array from the underlying bitmap
        void clear(const color_t &color) const {
            clear(color.r, color.g, color.b, color.a);
        }
        void clear(float r, float g, float b, float a) const {
            _fbo.bind();
            if(_is_pre_mul_alpha) { r*=a; g*=a; b*=a; }
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT);
            copy_to_backdrop();
            nitrogl::fbo_t::unbind();
        }

#define max___(a, b) ((a)<(b) ? (b) : (a))
#define min___(a, b) ((a)<(b) ? (a) : (b))

    private:
        void copy_region_to_backdrop(int left, int top, int right, int bottom) const {
            copy_region_to_texture(_tex_backdrop, left, top, left, top, right, bottom);
        }
        void copy_to_backdrop() const {
            copy_region_to_backdrop(0, 0, width(), height());
        }

        void copy_region_to_texture(const gl_texture &texture,
                            int textureLeft, int textureTop,
                            int left, int top, int right, int bottom) const {
            // should take 1ms per copy
            rect t = rect(textureLeft, textureTop, texture.width(), texture.height());
            rect c = rect(left, top, right, bottom)
                        .translate(textureLeft, textureTop)
                        .intersect(t)
                        .translate(-textureLeft, -textureTop)
                        .intersect(canvasWindowRect());
            // invert to opengl coordinates (0,0) is bottom-left
            int y_canvas = height() - c.bottom;
            int y_texture = texture.height() - (textureTop + c.height());
            _fbo.bind();
            texture.use(0);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, textureLeft, y_texture,
                                c.left, y_canvas, c.width(), c.height());
            gl_texture::unuse();
            fbo_t::unbind();
        }

        /**
         * given a sampler, generate the main shader of it and use the pool
         * to get it or update it
         * @param sampler Sampler object
         * @return
         */
        main_shader_program & get_main_shader_program_for_sampler(
                sampler_t & sampler) {
            /*{ // debug
                static int LL = 0;
                if(LL%1000==0) {
                    _blend_mode = blend_modes::Normal();
                } else {
                    _blend_mode = blend_modes::Multiply();
                }
                ++LL;
            }*/
            // we always regenerate a traversal because parts of a sampler
            // tree may have been used in another sampler, which might have
            // written the traversal info
            sampler.generate_traversal(0);
            microc::iterative_murmur<nitrogl::uintptr_type> murmur;
            const auto sampler_key = sampler.hash_code();
            const auto key = murmur.begin(sampler_key)
                  .next(_is_pre_mul_alpha ? 0 : 1)
                  .next_cast(_blend_mode)
                  .next_cast(_alpha_compositor).end();
            auto & pool = lru_main_shader_pool();
            auto res = pool.get(key);
            auto & program = res.object;
            if(!res.is_active) {
                // if it is not active, reconfigure it with new shader source code
                shader_compositor::composite_main_program_from_sampler(
                        program,sampler, _is_pre_mul_alpha,
                        _blend_mode, _alpha_compositor);
            }
            return program;
        }

    public:

        void drawRect(sampler_t & sampler,
                      float left, float top, float right, float bottom,
                      float opacity = 1.0,
                      mat3f transform = mat3f::identity(),
                      float u0=0., float v0=0., float u1=1., float v1=1.,
                      const mat3f & transform_uv = mat3f::identity()) {
            static float t =0;
            t+=0.01;
            glViewport(0, 0, width(), height());
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0, -1, 1);
            // make the transform about it's origin, a nice feature
            transform.post_translate(vec2f(-left, -top)).pre_translate(vec2f(left, top));
            // buffers
            float puvs[24] = {
                    left,  bottom, u0, v0, 0.0, 1.0, // xyuvpq
                    right, bottom, u1, v0, 0.0, 1.0,
                    right, top,    u1, v1, 0.0, 1.0,
                    left,  top,    u0, v1, 0.0, 1.0,
            };
            auto & program = get_main_shader_program_for_sampler(sampler);
            // data
            p4_render_node::data_type data = {
                    puvs, 24,
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity
            };
            glDisable(GL_BLEND);
            _node_p4.render(program, sampler, data);
            glEnable(GL_BLEND);
            fbo_t::unbind();
            copy_to_backdrop();
        }

        void drawMask(sampler_t & sampler, nitrogl::channels::channel channel,
                      float left, float top, float right, float bottom,
                      float u0=0., float v0=0., float u1=1., float v1=1.,
                      const mat3f & transform_uv = mat3f::identity()) {
            const auto * current_blend_mode = _blend_mode;
            const auto * current_alpha_compositor = _alpha_compositor;
            update_composition(blend_modes::Normal(), porter_duff::DestinationIn());
            channel_sampler cs {&sampler, channel};
            drawRect(cs, left, top, right, bottom, 1.0f, mat3f::identity(),
                     u0, v0, u1, v1, transform_uv);
            update_composition(current_blend_mode, current_alpha_compositor);
        }

        void drawCircle(sampler_t & sampler_fill, sampler_t & sampler_stroke,
                        float x, float y, float radius, float stroke,
                        float opacity = 1.0,
                        const mat3f & transform = mat3f::identity(),
                        float u0=0., float v0=0., float u1=1., float v1=1.,
                        const mat3f & transform_uv = mat3f::identity()) {
            float pad = stroke/2.0f + 5.0f;
            float ex_radi = radius + pad; // extended radius
            float l = x - ex_radi, t = y - ex_radi;
            float r = x + ex_radi, b = y + ex_radi;
            float w = r-l, h = b-t;
            float radius_n = radius/w;
            float stroke_n = stroke/w;
            float aa_fill = 1.0f/w;
            float aa_stroke = stroke_n==0.0f ? 0.0f : (1.0f/w);

            circle_sampler cs(&sampler_fill, &sampler_stroke, radius_n, stroke_n, aa_fill, aa_stroke);

            // make the transform about left-top of shape
            auto transform_modified = transform;
//            pad/=2.0f;
            transform_modified.post_translate(vec2f(-pad, -pad)).pre_translate(vec2f(pad, pad));

            drawRect(cs, l, t, r, b,
                     opacity,
                     transform_modified,
                     u0, v0, u1, v1, transform_uv);
        }

        void drawArc(sampler_t & sampler_fill, sampler_t & sampler_stroke,
                        float x, float y, float radius,
                        float from_angle, float to_angle,
                        float inner_radius=5,
                        float stroke=1,
                        float opacity = 1.0,
                        const mat3f & transform = mat3f::identity(),
                        float u0=0., float v0=0., float u1=1., float v1=1.,
                        const mat3f & transform_uv = mat3f::identity()) {
            float pad = inner_radius + (stroke)/2.0f + 5.0f;
            float ex_radi = radius + pad; // extended radius
            float l = x - ex_radi, t = y - ex_radi;
            float r = x + ex_radi, b = y + ex_radi;
            float w = r-l, h = b-t;
            float radius_n = radius/w;
            float radius_inner_n = inner_radius/w;
            float stroke_n = stroke/w;
            float aa_fill = 1.0f/w;
            float aa_stroke = stroke_n==0.0f ? 0.0f : (1.0f/w);
            from_angle = nitrogl::math::clamp(from_angle, 0.0f, math::pi<float>()*2.0f);
            to_angle = nitrogl::math::clamp(to_angle, 0.0f, math::pi<float>()*2.0f);
            arc_sampler cs {&sampler_fill, &sampler_stroke, from_angle, to_angle, radius_n,
                            radius_inner_n, stroke_n, aa_fill, aa_stroke };
            // make the transform about left-top of shape
            auto transform_modified = transform;
            //            pad/=2.0f;
            transform_modified.post_translate(vec2f(-pad, -pad)).pre_translate(vec2f(pad, pad));

            drawRect(cs, l, t, r, b,
                     opacity,
                     transform_modified,
                     u0, v0, u1, v1, transform_uv);
        }

        void drawPie(sampler_t & sampler_fill, sampler_t & sampler_stroke,
                     float x, float y, float radius,
                     float from_angle, float to_angle,
                     float stroke=1,
                     float opacity = 1.0,
                     const mat3f & transform = mat3f::identity(),
                     float u0=0., float v0=0., float u1=1., float v1=1.,
                     const mat3f & transform_uv = mat3f::identity()) {
            float pad = (stroke)/2.0f + 5.0f;
            float ex_radi = radius + pad; // extended radius
            float l = x - ex_radi, t = y - ex_radi;
            float r = x + ex_radi, b = y + ex_radi;
            float w = r-l, h = b-t;
            float radius_n = radius/w;
            float stroke_n = stroke/w;
            float aa_fill = 1.0f/w;
            float aa_stroke = stroke_n==0.0f ? 0.0f : (1.0f/w);
//            from_angle = nitrogl::math::clamp(from_angle, 0.0f, math::pi<float>()*2.0f);
//            to_angle = nitrogl::math::clamp(to_angle, 0.0f, math::pi<float>()*2.0f);
            pie_sampler cs {&sampler_fill, &sampler_stroke, from_angle, to_angle, radius_n,
                            stroke_n, aa_fill, aa_stroke };
            // make the transform about left-top of shape
            auto transform_modified = transform;
            //            pad/=2.0f;
            transform_modified.post_translate(vec2f(-pad, -pad)).pre_translate(vec2f(pad, pad));

            drawRect(cs, l, t, r, b,
                     opacity,
                     transform_modified,
                     u0, v0, u1, v1, transform_uv);
        }

        void drawQuadrilateral(sampler_t & sampler,
                               float v0_x, float v0_y,
                               float v1_x, float v1_y,
                               float v2_x, float v2_y,
                               float v3_x, float v3_y,
                               float opacity = 1.0,
                               mat3f transform = mat3f::identity(),
                               float u0=0., float v0=0., float u1=1., float v1=1.,
                               const mat3f & transform_uv = mat3f::identity()) {
            float q0 = 1.0f, q1 = 1.0f, q2 = 1.0f, q3 = 1.0f;
            float p0x = v0_x, p0y = v0_y;
            float p1x = v1_x, p1y = v1_y;
            float p2x = v2_x, p2y = v2_y;
            float p3x = v3_x, p3y = v3_y;
            float ax = p2x - p0x, ay = p2y - p0y;
            float bx = p3x - p1x, by = p3y - p1y;
            float t, s;
            float cross = ax * by - ay * bx;
            if (cross != 0.0f) {
                float cy = p0y - p1y;
                float cx = p0x - p1x;
                s = (ax * cy - ay * cx) / cross;
                if (s > 0.0f && s < 1.0f) {
                    t = (bx * cy - by * cx) / cross;
                    if (t > 0.0f && t < 1.0f) { // here casting t, s to float
                        q0 = 1.0f / (1.0f - t);
                        q1 = 1.0f / (1.0f - s);
                        q2 = 1.0f / t;
                        q3 = 1.0f / s;
                    }
                }
            }
            float u0_=u0, v0_=v1, u1_=u1, v1_=v1;
            float u2_ = u1, v2_=v0, u3_=u0, v3_=v0;
            float u0_q0 = u0_*q0, v0_q0 = v0_*q0;
            float u1_q1 = u1_*q1, v1_q1 = v1_*q1;
            float u2_q2 = u2_*q2, v2_q2 = v2_*q2;
            float u3_q3 = u3_*q3, v3_q3 = v3_*q3;
            //
            glViewport(0, 0, width(), height());
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0, -1, 1);
            // make the transform about it's origin, a nice feature
            transform.post_translate(vec2f(-v0_x, -v0_y)).pre_translate(vec2f(v0_x, v0_y));
            // buffers
            float puvs[24] = {
                    v0_x,  v0_y, u0_q0, v0_q0, 0.0, q0, // xyuvpq
                    v1_x,  v1_y, u1_q1, v1_q1, 0.0, q1,
                    v2_x,  v2_y, u2_q2, v2_q2, 0.0, q2,
                    v3_x,  v3_y, u3_q3, v3_q3, 0.0, q3,
            };
            auto & program = get_main_shader_program_for_sampler(sampler);
            // data
            p4_render_node::data_type data = {
                    puvs, 24,
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity
            };
            glDisable(GL_BLEND);
            _node_p4.render(program, sampler, data);
            glEnable(GL_BLEND);
            fbo_t::unbind();
            copy_to_backdrop();
        }
        
        void drawRoundedRect(sampler_t & sampler_fill, sampler_t & sampler_stroke,
                             float left, float top, float right, float bottom,
                             float radius, float stroke,
                             float opacity = 1.0,
                             const mat3f & transform = mat3f::identity(),
                             float u0=0., float v0=0., float u1=1., float v1=1.,
                             const mat3f & transform_uv = mat3f::identity()) {
            float pad_and_stroke = 5.0f + stroke/2.0f;
            float l = left + radius, t = top + radius;
            float r = right - radius, b = bottom - radius;
            float w = r-l, h = b-t;
            float w_c = right-left + pad_and_stroke*2.0f, h_c = bottom-top + pad_and_stroke*2.0f;
            float max_d = w_c > h_c ? w_c : h_c;
            float l_c = left-pad_and_stroke, t_c = top-pad_and_stroke;
            w /= max_d; h /= max_d;
            float off_l = (max_d-(right-left))/2.0f;
            float off_t = (max_d-(bottom-top))/2.0f;
            l_c=left-off_l;
            t_c=top-off_t;
            float radius_n = radius/max_d;
            float stroke_n = stroke/max_d;
            float aa_fill = 1.0f/max_d;
            float aa_stroke = stroke_n==0.0f ? 0.0f : (1.0f/max_d);
            rounded_rect_sampler cs(&sampler_fill, &sampler_stroke, w, h, radius_n, stroke_n,
                                    aa_fill, aa_stroke);

            // make the transform about left-top of shape
            auto transform_modified = transform;
            transform_modified.post_translate(vec2f(-off_l, -off_t)).pre_translate(vec2f(off_l, off_t));

            drawRect(cs,
                     l_c, t_c, l_c + max_d, t_c + max_d,
                     opacity, transform_modified,
                     u0, v0, u1, v1, transform_uv);
        }

        void drawRect_multi_node(float left, float top, float right, float bottom,
                      mat3f transform = mat3f::identity(),
                      float u0=0., float v0=0., float u1=1., float v1=1.,
                      const mat3f & transform_uv = mat3f::identity(),
                      opacity_t opacity = 255) {
            static float t =0;
            t+=0.01;
            glViewport(0, 0, width(), height());
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0, -1, 1);
            // make the transform about it's center of mass, a nice feature
            transform.post_translate(vec2f(-left, -top)).pre_translate(vec2f(left, top));

            // buffers
            float points[8] = {
                    left, bottom,
                    right, bottom,
                    right, top,
                    left, top
            };
            float uvs_sampler[8] = {
                    u0, v0,
                    u1, v0,
                    u1, v1,
                    u0, v1
            };
            float uvs_sampler2[8] = {
                    0.3, 0.3,
                    0.6, 0.3,
                    0.6, 0.6,
                    0.3, 0.6
            };

            static GLuint e[6] = { 0, 1, 2, 2, 3, 0 };

            // get shader from cache
            test_sampler<> sampler;
            main_shader_program program;
            shader_compositor::composite_main_program_from_sampler(program, sampler);

            // data
            multi_render_node::data_type data = {
                    points, uvs_sampler, e,
                    8, 8, 6, GL_TRIANGLES,
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv
            };
            _node_multi.render(program, sampler, data);

            //
            copy_region_to_backdrop(int(left), int(top),
                                    int(right+0.5f), int(bottom+0.5f));
            fbo_t::unbind();
        }

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
