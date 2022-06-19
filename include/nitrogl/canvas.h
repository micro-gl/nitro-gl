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

#ifndef MICROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/path.h"
#include "micro-tess/include/micro-tess/monotone_polygon_triangulation.h"
#include "micro-tess/include/micro-tess/fan_triangulation.h"
#include "micro-tess/include/micro-tess/ear_clipping_triangulation.h"
#include "micro-tess/include/micro-tess/bezier_patch_tesselator.h"
#include "micro-tess/include/micro-tess/curve_divider.h"
#include "micro-tess/include/micro-tess/dynamic_array.h"
#else
#include <micro-tess/path.h>
#include <micro-tess/monotone_polygon_triangulation.h>
#include <micro-tess/fan_triangulation.h>
#include <micro-tess/ear_clipping_triangulation.h>
#include <micro-tess/bezier_patch_tesselator.h>
#include <micro-tess/curve_divider.h>
#include <micro-tess/dynamic_array.h>
#endif

#include "functions/minmax.h"
#include "functions/clamp.h"
#include "functions/swap.h"
#include "functions/orient2d.h"
#include "functions/bits.h"
#include "functions/distance.h"
#include "triangles.h"
#include "polygons.h"

//tect
#include "text/bitmap_font.h"
#include "text/bitmap_glyph.h"
#include "text/text_format.h"

// ogl
#include "ogl/gl_texture.h"
#include "ogl/fbo.h"
#include "ogl/vbo.h"
#include "ogl/ebo.h"

// render nodes
#include "render_nodes/multi_render_node.h"
#include "render_nodes/multi_render_node_interleaved_xyuv.h"
#include "render_nodes/p4_render_node.h"

// internal
#include "_internal/main_shader_program.h"
#include "_internal/shader_compositor.h"
#include "_internal/static_linear_allocator.h"
#include "_internal/lru_pool.h"
#include "camera.h"
#include "path.h"

// samplers
#include "samplers/test_sampler.h"
#include "samplers/texture_sampler.h"
#include "samplers/shapes/circle_sampler.h"
#include "samplers/shapes/rounded_rect_sampler.h"
#include "samplers/color_sampler.h"
#include "samplers/tint_sampler.h"
#include "samplers/channel_sampler.h"
#include "samplers/shapes/arc_sampler.h"
#include "samplers/shapes/pie_sampler.h"

// compositing
#include "compositing/porter_duff.h"
#include "compositing/blend_modes.h"

namespace nitrogl {

    // draw mode enables to change the draw mode
    enum class draw_mode { fill=GL_FILL, line=GL_LINE, point=GL_POINT };

    class canvas {
    public:
        using index = GLuint;//unsigned int;
        using precision = unsigned char;
        using opacity_t = unsigned char;

        // rasterizer integers
        using rint_big = int64_t;
        struct window_t {
            rect_i canvas_rect;
            rect_i clip_rect;
        };

    private:
        using static_alloc = micro_alloc::static_linear_allocator<char, 1<<14, 0>;
        using lru_main_shader_pool_t = microc::lru_pool<main_shader_program, 5,
                                                nitrogl::uintptr_type, static_alloc>;
        window_t _window;
        gl_texture _tex_backdrop;
        fbo_t _fbo;
        multi_render_node _node_multi;
        multi_render_node_interleaved_xyuv _node_multi_interleaved;
        p4_render_node _node_p4;
        blend_mode_t _blend_mode;
        compositor_t _alpha_compositor;
        draw_mode _draw_mode;

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
            _node_multi_interleaved.init();
            updateDrawMode(_draw_mode);
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
                                                  _fbo(), _node_multi(), _node_multi_interleaved(), _node_p4(), _window(),
                                                  _is_pre_mul_alpha(tex.is_premul_alpha()),
                                                  _blend_mode(blend_modes::Normal()),
                                                  _alpha_compositor(porter_duff::SourceOver()),
                                                  _draw_mode(draw_mode::fill) {
            _fbo.attachTexture(tex);
            internal_init(tex.width(), tex.height());
        }

        // if you got nothing, draw to bound fbo
        canvas(int width, int height, bool is_pre_mul_alpha=true) :
                _tex_backdrop(gl_texture::un_generated_dummy()), _fbo(fbo_t::from_current()),
                _node_multi(), _node_p4(), _node_multi_interleaved(), _window(), _is_pre_mul_alpha(is_pre_mul_alpha),
                _blend_mode(blend_modes::Normal()), _alpha_compositor(porter_duff::SourceOver()),
                _draw_mode(draw_mode::fill) {
            internal_init(width, height);
        }

        /**
         * Change the draw mode to triangle fills, lines or points
         * @param mode enum { draw_mode::fill, draw_mode::line, draw_mode::point }
         */
        void updateDrawMode(draw_mode mode) {
            _draw_mode = mode;
            GLenum mode_gl = int(_draw_mode);
            glPolygonMode(GL_FRONT_AND_BACK, mode_gl);
        }

        /**
         * update the clipping rectangle of the canvas
         *
         * @param l left distance to x=0
         * @param t top distance to y=0
         * @param r right distance to x=0
         * @param b bottom distance to y=0
         */
        void updateClipRect(int l, int t, int r, int b) { _window.clip_rect = rect_i{l, t, r, b}; }

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
            _window.canvas_rect = rect_i{left, top, left + right, top + bottom };
            if(_window.clip_rect.empty()) _window.clip_rect=_window.canvas_rect;
        }

        void updateCanvasWindow(int left, int top) {
            updateCanvasWindow(left, top, width(), height());
        }

        /**
         * given that we know the canvas size and the clip rect_i, calculate
         * the sub rectangle (intersection), where drawing is visible
         */
        rect_i calculateEffectiveDrawRect() const {
            rect_i r = _window.canvas_rect.intersect(_window.clip_rect);
            r.bottom-=1;r.right-=1;
            return r;
        }

        /**
         * get the clipping rectangle
         */
        const rect_i & clipRect() const { return _window.clip_rect; }

        /**
         * get the canvas rectangle, should be (0, 0, width, height), unless
         * the sub windowing feature was used.
         * @return a rectangle
         */
        const rect_i & canvasWindowRect() const { return _window.canvas_rect; }

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
            copy_region_to_backdrop(0, 0, int(width()), int(height()));
        }

        void copy_region_to_texture(const gl_texture &texture,
                            int textureLeft, int textureTop,
                            int left, int top, int right, int bottom) const {
            // should take 1ms per copy
            rect_i t = rect_i(textureLeft, textureTop, texture.width(), texture.height());
            rect_i c = rect_i(left, top, right, bottom)
                        .translate(textureLeft, textureTop)
                        .intersect(t)
                        .translate(-textureLeft, -textureTop)
                        .intersect(canvasWindowRect());
            // invert to opengl coordinates (0,0) is bottom-left
            int y_canvas = int(height()) - c.bottom;
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

        static mat3f & prepare_uv_transform(mat3f & transform_uv,
                                     float bbox_width, float bbox_height,
                                     float intrinsic_width=0.0f, float intrinsic_height=0.0f,
                                     float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            //
            // Always remember, our basic UVS are ALWAYS relative to the bounding box of
            // the geometry and are stretched on it. Then, we fix by tiling and then focusing
            // on a UV window. Remember, pre_transformations are right-most (and are fast)
            // First create affine uv transform to the window
            transform_uv.pre_translate(vec2f(u0, v0)).pre_scale(vec2f{u1-u0, v1-v0});
//            transform_uv.pre_scale(vec2f{0.5f, 0.5f});
            if(intrinsic_width>0 && intrinsic_height>0) {
                // If we have intrinsic dimensions, apply uv transform fix to tile the sampler,
                // otherwise it will be just stretched.
                float scaled_w = intrinsic_width * (u1-u0);
                float scaled_h = intrinsic_height * (v1-v0);
                transform_uv.pre_scale(vec2f{bbox_width/scaled_w, bbox_height/scaled_h});
            }
            return transform_uv;
        }

    public:

        /**
         * Draw a batch of indexed triangles.
         * NOTES:
         * 1. if uvs is null, we will compute them for you on the gpu
         * 2. if indices is null, indices will be inferred as well
         * @param sampler the sampler to sample from
         * @param type Type of triangles {Triangles, Fan, Strip}
         * @param vertices The vertices array pointer
         * @param vertices_size The size of vertices array
         * @param indices (Optional) The indices array pointer
         * @param indices_size (Optional) The size of indices array
         * @param uvs (Optional) The UVs array pointer
         * @param uvs_size (Optional) The size of uvs array
         * @param transform vertices transform
         * @param opacity Opacity
         * @param transform_uv UVs transform
         * @param u0/v0/u1/v1 UVs window
         */
        void drawTriangles(sampler_t & sampler,
                           enum triangles::indices type,
                           const vec2f * vertices,
                           index vertices_size,
                           const index * indices=nullptr,
                           index indices_size=0,
                           const vec2f * uvs=nullptr,
                           index uvs_size=0,
                           mat3f transform = mat3f::identity(),
                           float opacity=1.0f,
                           mat3f transform_uv = mat3f::identity(),
                           float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            const auto bbox = nitrogl::triangles::triangles_bbox(vertices, vertices_size, indices, indices_size);
            prepare_uv_transform(transform_uv, bbox.width(), bbox.height(),
                                 sampler.intrinsic_width, sampler.intrinsic_height,
                                 u0, v0, u1, v1);

            //
            glViewport(0, 0, GLsizei(width()), GLsizei(height()));
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0.0f,
                                                        -1.0f, 1.0f);
            // make the transform about its origin, a nice feature
            transform.post_translate(vec2f(-bbox.left, -bbox.top)).pre_translate(vec2f(bbox.left, bbox.top));
            // buffers
            auto & program = get_main_shader_program_for_sampler(sampler);
            // data
            multi_render_node::data_type data = {
                    vertices, uvs, nullptr, indices,
                    vertices_size, uvs_size, 0, indices_size,
                    GLenum(type),
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv, //transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity,
                    bbox
            };
            glDisable(GL_BLEND);
            _node_multi.render(program, sampler, data);
            glEnable(GL_BLEND);
            fbo_t::unbind();
            copy_to_backdrop();
        }

        /**
         * Draw a batch of indexed interleaved triangles.
         * NOTES:
         * 1. if uvs is null, we will compute them for you
         * 2. if indices is null, indices will be inferred as well
         * @param sampler the sampler to sample from
         * @param type Type of triangles {Triangles, Fan, Strip}
         * @param xyuv The xyuv array pointer [(x,y,u,v), (x,y,u,v), ....]
         * @param xyuv_size The size of xyuv array
         * @param indices (Optional) The indices array pointer
         * @param indices_size (Optional) The size of indices array
         * @param transform vertices transform
         * @param opacity Opacity
         * @param transform_uv UVs transform
         * @param u0/v0/u1/v1 UVs window
         */
        void drawInterleavedTriangles(const sampler_t & sampler,
                                    enum triangles::indices type,
                                   const float * xyuv,
                                   index xyuv_size,
                                   const index * indices,
                                   index indices_size,
                                   mat3f transform = mat3f::identity(),
                                   float opacity=1.0f,
                                   mat3f transform_uv = mat3f::identity(),
                                   float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            const auto bbox = nitrogl::triangles::triangles_bbox_from_attribs(xyuv, xyuv_size/4, indices, indices_size,
                                                                              0, 1, 4);
            prepare_uv_transform(transform_uv, bbox.width(), bbox.height(),
                                 sampler.intrinsic_width, sampler.intrinsic_height,
                                 u0, v0, u1, v1);

            //
            glViewport(0, 0, GLsizei(width()), GLsizei(height()));
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0.0f,
                                                        -1.0f, 1.0f);
            // make the transform about its origin, a nice feature
            transform.post_translate(vec2f(-bbox.left, -bbox.top)).pre_translate(vec2f(bbox.left, bbox.top));
            // buffers
            auto & program = get_main_shader_program_for_sampler(const_cast<sampler_t &>(sampler));
            // data
            multi_render_node_interleaved_xyuv::data_type data = {
                    xyuv, indices,
                    xyuv_size, indices_size,
                    GLenum(type),
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv, //transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity,
            };
            glDisable(GL_BLEND);
            _node_multi_interleaved.render(program, const_cast<sampler_t &>(sampler), data);
            glEnable(GL_BLEND);
            fbo_t::unbind();
            copy_to_backdrop();
        }

        /**
         * Draw a Path Fill
         * @tparam path_container_template template of container used by path
         * @tparam tessellation_allocator the path allocator
         * @param sampler The sampler to sample from
         * @param path The path object
         * @param rule Fill rule { non_zero, even_odd }
         * @param quality Tessellation quality enum
         * @param transform vertices transform
         * @param transform_uv UVs transform
         * @param opacity Opacity
         * @param u0/v0/u1/v1 UVs window
         */
        template <template<typename...> class path_container_template,
                  class tessellation_allocator>
        void drawPathFill(sampler_t & sampler,
                          microtess::path<float, path_container_template, tessellation_allocator> & path,
                          const microtess::fill_rule &rule,
                          const microtess::tess_quality &quality,
                          const mat3f & transform = mat3f::identity(),
                          const mat3f & transform_uv = mat3f::identity(),
                          float opacity=1.0f,
                          float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            const auto & buffers= path.tessellateFill(rule, quality, false, false);
            if(buffers.output_vertices.size()==0) return;
            const auto type_out =
                    nitrogl::triangles::microtess_indices_type_to_nitrogl(
                            buffers.output_indices_type);

            drawTriangles(
                    sampler,
                    type_out,
                    buffers.output_vertices.data(), buffers.output_vertices.size(),
                    buffers.output_indices.data(), buffers.output_indices.size(),
                    nullptr, 0,
                    transform,
                    opacity,
                    transform_uv,
                    u0, v0, u1, v1);
//            if(debug) {
//                drawTrianglesWireframe({0,0,0,255}, transform,
//                                       buffers.output_vertices.data(),
//                                       buffers.output_indices.data(),
//                                       buffers.output_indices.size(),
//                                       buffers.output_indices_type,
//                                       40);
//                for (index ix = 0; ix < buffers.DEBUG_output_trapezes.size(); ix+=4)
//                    drawWuLinePath<number1>({0,0,0,255}, &buffers.DEBUG_output_trapezes[ix], 4, true);
//            }
        }

        /**
         * Draw a Path Stroke
         * @tparam Iterable Any numbers iterable container (implements begin()/)end())
         * @tparam path_container_template template of container used by path
         * @tparam tessellation_allocator the path allocator
         * @param sampler The sampler to sample from
         * @param path The path object
         * @param stroke_width          stroke width in pixels
         * @param cap                   stroke cap enum {butt, round, square}
         * @param line_join             stroke line join {none, miter, miter_clip, round, bevel}
         * @param miter_limit           the miter limit
         * @param stroke_dash_array     stroke dash pattern
         * @param stroke_dash_offset    stroke dash offset
         * @param transform vertices transform
         * @param transform_uv UVs transform
         * @param opacity Opacity
         * @param u0/v0/u1/v1 UVs window
         */
        template <class Iterable, template<typename...> class path_container_template,
                    class tessellation_allocator>
        void drawPathStroke(sampler_t & sampler,
                          microtess::path<float, path_container_template, tessellation_allocator> & path,
                          float stroke_width=1.0f,
                          microtess::stroke_cap cap=microtess::stroke_cap::butt,
                          microtess::stroke_line_join line_join=microtess::stroke_line_join::bevel,
                          const int miter_limit=4,
                          const Iterable & stroke_dash_array={},
                          int stroke_dash_offset=0,
                          const mat3f & transform = mat3f::identity(),
                          const mat3f & transform_uv = mat3f::identity(),
                          float opacity=1.0f,
                          float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            const auto & buffers= path.template tessellateStroke<Iterable>(
                    stroke_width, cap, line_join, miter_limit, stroke_dash_array, stroke_dash_offset);
            if(buffers.output_vertices.size()==0) return;
            const auto type_out =
                    nitrogl::triangles::microtess_indices_type_to_nitrogl(
                            buffers.output_indices_type);

            drawTriangles(
                    sampler,
                    type_out,
                    buffers.output_vertices.data(), buffers.output_vertices.size(),
                    buffers.output_indices.data(), buffers.output_indices.size(),
                    nullptr, 0,
                    transform,
                    opacity,
                    transform_uv,
                    u0, v0, u1, v1);
//            if(debug)
//                drawTrianglesWireframe({0, 0, 0, 255}, transform,
//                                       buffers.output_vertices.data(),
//                                       buffers.output_indices.data(),
//                                       buffers.output_indices.size(),
//                                       buffers.output_indices_type,
//                                       255);

        }

        /**
         * Draw a polygon of any type via tesselation given a hint.
         * Notes:
         * - Uses different algorithms for different polygon types:
         *      - Planar Subdivision for COMPLEX, SELF_INTERSECTING
         *      - Ear Clipping for SIMPLE, CONCAVE
         *      - Monotone triangulation for X_MONOTONE, Y_MONOTONE
         *      - Fan triangulation for CONVEX
         * - TIPS:
         *      - CONVEX polygons do not allocate more memory !!!
         *      - Use the hints properly to max your performance
         *
         * @tparam hint                     the type of polygon {SIMPLE, CONCAVE, X_MONOTONE, Y_MONOTONE, CONVEX, COMPLEX, SELF_INTERSECTING}
         * @tparam tessellation_allocator   type of allocator
         *
         * @param sampler       sampler reference
         * @param transform     3x3 matrix transform
         * @param points        vertex array pointer
         * @param size          size of vertex array
         * @param opacity       opacity [0..255]
         * @param u0            uv coord
         * @param v0            uv coord
         * @param u1            uv coord
         * @param v1            uv coord
         */
        template <nitrogl::polygons hint=nitrogl::polygons::SIMPLE,
                  class tessellation_allocator=nitrogl::std_rebind_allocator<>>
        void drawPolygon(sampler_t & sampler,
                         const vec2f * points,
                         index size,
                         const mat3f & transform = mat3f::identity(),
                         const mat3f & transform_uv = mat3f::identity(),
                         float opacity=1.0f,
                         float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f,
                         const tessellation_allocator & allocator=tessellation_allocator()) {

            microtess::triangles::indices type;
            using indices_allocator_t = typename tessellation_allocator::
                    template rebind<index>::other;
            using boundary_allocator_t = typename tessellation_allocator::
                    template rebind<microtess::triangles::boundary_info>::other;
            using indices_t = dynamic_array<index, indices_allocator_t>;
            using boundaries_t = dynamic_array<microtess::triangles::boundary_info, boundary_allocator_t>;

            indices_t indices{indices_allocator_t(allocator)};
            boundaries_t * boundary_buffer_ptr=nullptr;

            switch (hint) {
                case nitrogl::polygons::CONCAVE:
                case nitrogl::polygons::SIMPLE:
                {
                    using ect=microtess::ear_clipping_triangulation<float, indices_t, boundaries_t, tessellation_allocator>;
                    ect::compute(points, size, indices, boundary_buffer_ptr, type, allocator);
                    break;
                }
                case nitrogl::polygons::X_MONOTONE:
                case nitrogl::polygons::Y_MONOTONE:
                {
                    using mpt=microtess::monotone_polygon_triangulation<float, indices_t, boundaries_t, tessellation_allocator>;
                    typename mpt::monotone_axis axis=hint==polygons::X_MONOTONE ? mpt::monotone_axis::x_monotone :
                            mpt::monotone_axis::y_monotone;
                    mpt::compute(points, size, axis, indices, boundary_buffer_ptr, type, allocator);
                    break;
                }
                case nitrogl::polygons::CONVEX:
                {
                    type = microtess::triangles::indices::TRIANGLES_FAN;
                    break;
                }
                case nitrogl::polygons::NON_SIMPLE:
                case nitrogl::polygons::SELF_INTERSECTING:
                case nitrogl::polygons::COMPLEX:
                case nitrogl::polygons::MULTIPLE_POLYGONS:
                {
                    microtess::path<float, dynamic_array, tessellation_allocator> path(allocator);
                    path.addPoly(points, size);
                    drawPathFill<dynamic_array, tessellation_allocator> (
                            sampler,
                            path,
                            microtess::fill_rule::non_zero,
                            microtess::tess_quality::better,
                            transform,
                            transform_uv,
                            opacity, u0, v0, u1, v1);
                    return;
                }
                default:
                    return;
            }
            // convert from micro-tess indices type to nitro-gl indices type
            const auto type_out = nitrogl::triangles::microtess_indices_type_to_nitrogl(type);
            drawTriangles(sampler,
                    type_out,
                    points, size,
                    indices.data(), indices.size(),
                    nullptr, 0,
                    transform,
                    opacity,
                    transform_uv,
                    u0, v0, u1, v1);
        }

        /**
         * Draw a Quadratic or Cubic bezier patch
         *
         * @tparam patch_type  { microtess::patch_type::BI_QUADRATIC, microtess::patch_type::BI_CUBIC } enum
         * @tparam BlendMode    the blend mode struct
         * @tparam PorterDuff   the alpha compositing struct
         * @tparam antialias    enable/disable anti-aliasing, currently NOT supported
         * @tparam debug        enable debug mode ?
         * @tparam number1      number type for vertices
         * @tparam number2      number type for uv coords
         * @tparam Sampler      type of sampler
         *
         * @param sampler       sampler reference
         * @param transform     3x3 matrix transform
         * @param mesh          4*4*2=32 or 3*3*2=18 patch, flattened array of row-major (x, y) points
         * @param uSamples      the number of samples to take along U axis
         * @param vSamples      the number of samples to take along V axis
         * @param u0            uv coord
         * @param v0            uv coord
         * @param u1            uv coord
         * @param v1            uv coord
         * @param opacity       opacity [0..255]
         */
        template<microtess::patch_type patch_type, class Allocator=nitrogl::std_rebind_allocator<>>
        void drawBezierPatch(sampler_t & sampler,
                             const float *mesh,
                             unsigned uSamples=20, unsigned vSamples=20,
                             mat3f transform = mat3f::identity(),
                             float opacity = 1.0f,
                             float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f,
                             mat3f transform_uv = mat3f::identity(),
                             const Allocator & allocator=Allocator()) {
            using rebind_alloc_t1 = typename Allocator::template rebind<float>::other;
            using rebind_alloc_t2 = typename Allocator::template rebind<index>::other;
            rebind_alloc_t1 rebind_1{allocator};
            rebind_alloc_t2 rebind_2{allocator};
            dynamic_array<float, rebind_alloc_t1> v_a{rebind_1}; // vertices attributes
            dynamic_array<index, rebind_alloc_t2> indices{rebind_2};

            using tess= microtess::bezier_patch_tesselator<float, float,
                                        dynamic_array<float, rebind_alloc_t1>,
                                        dynamic_array<index, rebind_alloc_t2>>;
            microtess::triangles::indices indices_type;
            const auto window_size = tess::template compute<patch_type>(
                    mesh, 2, uSamples, vSamples, true, true,
                    v_a, indices, indices_type,
                    u0, v0, u1, v1);
            const index size = indices.size();
            if(size==0) return;
            const auto type_out = nitrogl::triangles::microtess_indices_type_to_nitrogl(indices_type);
            drawInterleavedTriangles(
                    sampler,
                    type_out,
                    v_a.data(), v_a.size(),
                    indices.data(), indices.size(),
                    transform,
                    opacity,
                    transform_uv,
                    0.0f, 0.0f, 1.0f, 1.0f);
        }

        void drawRect(const sampler_t & sampler,
                      float left, float top, float right, float bottom,
                      float opacity = 1.0f,
                      mat3f transform = mat3f::identity(),
                      float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f,
                      mat3f transform_uv = mat3f::identity()) {
            prepare_uv_transform(transform_uv, right-left, bottom-top,
                                 sampler.intrinsic_width, sampler.intrinsic_height,
                                 u0, v0, u1, v1);
            glViewport(0, 0, GLsizei(width()), GLsizei(height()));
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0.0f,
                                                        -1.0f, 1.0f);
            // make the transform about its origin, a nice feature
            transform.post_translate(vec2f(left, top)).pre_translate(vec2f(-left, -top));
            // buffers
            float puvs[20] = {
                    left,  bottom, 0.0f, 0.0f, 1.0f, // xyuvq
                    right, bottom, 1.0f, 0.0f, 1.0f,
                    right, top,    1.0f, 1.0f, 1.0f,
                    left,  top,    0.0f, 1.0f, 1.0f,
            };
            auto & program = get_main_shader_program_for_sampler(const_cast<sampler_t &>(sampler));
            // data
            p4_render_node::data_type data = {
                    puvs, 20,
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity
            };
            glDisable(GL_BLEND);
            _node_p4.render(program, const_cast<sampler_t &>(sampler), data);
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
            float aa_stroke = stroke_n==0.0f ? 0.0f : (1.f/w);

            circle_sampler cs(&sampler_fill, &sampler_stroke, radius_n, stroke_n, aa_fill, aa_stroke);

            // make the transform about left-top of shape
            auto transform_modified = transform;
//            pad/=2.0f;
            transform_modified.post_translate(vec2f(pad, pad)).pre_translate(vec2f(-pad, -pad));

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
            transform_modified.post_translate(vec2f(pad, pad)).pre_translate(vec2f(-pad, -pad));

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
            transform_modified.post_translate(vec2f(pad, pad)).pre_translate(vec2f(-pad, -pad));

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
                               float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f,
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
            transform.post_translate(vec2f(v0_x, v0_y)).pre_translate(vec2f(-v0_x, -v0_y));
            // buffers
            float puvs[20] = {
                    v0_x,  v0_y, u0_q0, v0_q0, q0, // xyuvq
                    v1_x,  v1_y, u1_q1, v1_q1, q1,
                    v2_x,  v2_y, u2_q2, v2_q2, q2,
                    v3_x,  v3_y, u3_q3, v3_q3, q3,
            };
            auto & program = get_main_shader_program_for_sampler(sampler);
            // data
            p4_render_node::data_type data = {
                    puvs, 20,
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
            transform_modified.post_translate(vec2f(off_l, off_t)).pre_translate(vec2f(-off_l, -off_t));

            drawRect(cs,
                     l_c, t_c, l_c + max_d, t_c + max_d,
                     opacity, transform_modified,
                     u0, v0, u1, v1, transform_uv);
        }

        /**
         * Draw text based on a regular bitmap font
         * @tparam max_chars max amount of chars in the bitmap font
         * @tparam Allocator memory allocator
         * @param text null terminated char array
         * @param font Bitmap font
         * @param color tint color
         * @param format text format
         * @param left pos left
         * @param top pos top
         * @param right pos right
         * @param bottom pos bottom
         * @param transform transform matrix
         * @param opacity opacity
         * @param allocator allocator reference
         */
        template<unsigned max_chars, class Allocator=nitrogl::std_rebind_allocator<>>
        void drawText(const char * text,
                      const nitrogl::text::bitmap_font<max_chars> & font,
                      const color_t & color,
                      nitrogl::text::text_format & format,
                      int left, int top, int right, int bottom,
                      mat3f transform = mat3f::identity(),
                      float opacity=1.0f,
                      const Allocator & allocator=Allocator()) {
            auto old=clipRect(); updateClipRect(left, top, right, bottom);
            unsigned int text_size=0;
            { const char * iter=text; while(*iter++!= '\0' && ++text_size); }

            // setup allocators
            using char_location_allocator_t = typename Allocator::template
                    rebind<nitrogl::text::char_location>::other;
            using index_allocator_t = typename Allocator::template rebind<index>::other;
            using float_allocator_t = typename Allocator::template rebind<float>::other;

            char_location_allocator_t char_location_allocator{allocator};
            index_allocator_t index_allocator{allocator};
            float_allocator_t float_allocator{allocator};

            // allocate char location buffer
            nitrogl::text::char_location * char_loc_buffer = char_location_allocator.allocate(text_size);

            // layout text
            const auto result=font.layout_text(text, text_size, right-left,
                                               bottom-top, format, char_loc_buffer);
            unsigned layout_size= result.end_index;
            const int P=result.precision, S=(result.scale)/(1<<P);
            const bool has_scaled=S!=1<<P;

            auto blah = sizeof (nitrogl::text::char_location);

            // allocate render buffers
            const auto indices_size = layout_size * 6;
            const auto xyuvs_size = layout_size * 4 * 4;
            index * indices = index_allocator.allocate(indices_size); // 6 indices per quad, we use triangles type
            float * xyuvs = float_allocator.allocate(xyuvs_size); // interleaved xyuv
            triangles::indices type = triangles::indices::TRIANGLES;

            // tessellate quads to triangles
            {
                const auto tess_quad = [&](const nitrogl::text::char_location & l, int index) {
                    // p0  p2
                    // |A /|
                    // | / |
                    // |/ B|
                    // p1  p3
                    int ix = index * 4 * 4;
                    //
                    float ll = float((left<<P) + l.x)/(1<<P), tt = float((top<<P) + l.y)/(1<<P);
                    float rr = ll + l.character->width*S, bb = tt + l.character->height*S;
//                    float ll = 100, tt=100, rr=400, bb=400;
                    float u0 = float(l.character->x)/font.bitmap.width();
                    float v0 = float(l.character->y)/font.bitmap.height();
                    float u1 = float(l.character->x+l.character->width)/font.bitmap.width();
                    float v1 = float(l.character->y+l.character->height)/font.bitmap.height();
                    // p0
                    int W = 0;
                    xyuvs[ix + W + 0] = ll; xyuvs[ix + W + 1] = tt;
                    xyuvs[ix + W + 2] = u0; xyuvs[ix + W + 3] = v0; W+=4;
                    // p1
                    xyuvs[ix + W + 0] = ll; xyuvs[ix + W + 1] = bb;
                    xyuvs[ix + W + 2] = u0; xyuvs[ix + W + 3] = v1; W+=4;
                    // p2
                    xyuvs[ix + W + 0] = rr; xyuvs[ix + W + 1] = tt;
                    xyuvs[ix + W + 2] = u1; xyuvs[ix + W + 3] = v0; W+=4;
                    // p3
                    xyuvs[ix + W + 0] = rr; xyuvs[ix + W + 1] = bb;
                    xyuvs[ix + W + 2] = u1; xyuvs[ix + W + 3] = v1;
                    // indices
                    // triangle A: 0-1-2
                    indices[index * 6 + 0] = index * 4 + 0;
                    indices[index * 6 + 1] = index * 4 + 1;
                    indices[index * 6 + 2] = index * 4 + 2;
                    // triangle B: 3-2-1
                    indices[index * 6 + 3] = index * 4 + 3;
                    indices[index * 6 + 4] = index * 4 + 2;
                    indices[index * 6 + 5] = index * 4 + 1;
                };

                for (int ix = 0; ix < layout_size; ++ix) {
                    const auto & l = char_loc_buffer[ix];
                    // tessellate a char quad into two triangles
                    tess_quad(l, ix);
                }
            }

            // setup text sampler
            texture_sampler tex {font.bitmap, false};
            tint_sampler tint { color, &tex };
            //drawRect(tex, 100, 100, 400,  400);

            // draw interleaved triangles
            drawInterleavedTriangles(tint,
                                     type,
                                     xyuvs, xyuvs_size,
                                     indices, indices_size,
                                     transform, opacity);

            updateClipRect(old.left, old.top, old.right, old.bottom);

            // de-allocate memory
            char_location_allocator.deallocate(char_loc_buffer);
            index_allocator.deallocate(indices);
            float_allocator.deallocate(xyuvs);
        }

        /**
         * Draw lines path
         *
         * @param sampler       The sampler
         * @param points        the points array pointer
         * @param size          the size of the points array
         * @param closed_path   is the path closed ?
         */
        void drawLines(const sampler_t & sampler,
                       const vec2f * points,
                       unsigned int size = 4,
                       bool closed_path = false,
                       mat3f transform = mat3f::identity(),
                       float opacity=1.0f,
                       mat3f transform_uv = mat3f::identity(),
                       float u0=0.f, float v0=0.f, float u1=1.f, float v1=1.f) {
            const auto bbox = nitrogl::triangles::triangles_bbox(points, size, nullptr, 0);
            prepare_uv_transform(transform_uv, bbox.width(), bbox.height(),
                                 sampler.intrinsic_width, sampler.intrinsic_height,
                                 u0, v0, u1, v1);

            //
            glViewport(0, 0, GLsizei(width()), GLsizei(height()));
            _fbo.bind();
            // inverted y projection, canvas coords to opengl
            auto mat_proj = camera::orthographic<float>(0.0f, float(width()),
                                                        float(height()), 0.0f,
                                                        -1.0f, 1.0f);
            // make the transform about its origin, a nice feature
            transform.post_translate(vec2f(-bbox.left, -bbox.top)).pre_translate(vec2f(bbox.left, bbox.top));
            // buffers
            auto & program = get_main_shader_program_for_sampler(const_cast<sampler_t &>(sampler));
            // data
            const auto type = closed_path ? nitrogl::triangles::LINE_LOOP : nitrogl::triangles::LINE_STRIP;
            multi_render_node::data_type data = {
                    points, nullptr, nullptr, nullptr,
                    size, 0, 0, 0,
                    GLenum(type),
                    mat4f(transform), // promote it to mat4x4
                    mat4f::identity(),
                    mat_proj,
                    transform_uv, //transform_uv,
                    _tex_backdrop,
                    width(), height(),
                    opacity,
                    bbox
            };
            glDisable(GL_BLEND);
            _node_multi.render(program, const_cast<sampler_t &>(sampler), data);
            glEnable(GL_BLEND);
            fbo_t::unbind();
            copy_to_backdrop();

        }

    };

}
