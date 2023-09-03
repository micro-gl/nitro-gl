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

#include "../ogl/shader_program.h"
#include "../ogl/vao.h"
#include "../ogl/vbo.h"
#include "../ogl/ebo.h"
#include "../_internal/main_shader_program.h"
#include "../samplers/sampler.h"
#include "../math.h"

namespace nitrogl {

    class multi_render_node_interleaved_xyuv {

    public:
        using program_type = main_shader_program;
        using size_type = GLsizeiptr;
        struct data_type {
            const float * xyuv;
            const GLuint * indices;

            size_type xyuv_size;
            size_type indices_size;

            GLenum triangles_type;

            const mat4f & mat_model;
            const mat4f & mat_view;
            const mat4f & mat_proj;
            const mat3f & mat_uvs_sampler;
            const gl_texture & backdrop_texture;
            const GLuint window_width;
            const GLuint window_height;
            const float opacity;
        };

        struct GVA {
            GVA()=default;
            static constexpr unsigned SIZE = 2;
            static constexpr unsigned size() { return SIZE; }
            nitrogl::generic_vertex_attrib_t data[SIZE];
        };

        GVA gva{};
        vbo_t _vbo_xyuv{};
        vao_t _vao{};
        ebo_t _ebo{};

    public:
        multi_render_node_interleaved_xyuv()=default;
        ~multi_render_node_interleaved_xyuv()=default;

        void init() {
            // configure the vao, vbo, generic vertex attribs, non interleaved
            // configure the vao, vbo, generic vertex attribs [(x,y,u,v) ....], interleaved
            const int STRIDE = 4*sizeof (GLfloat);
            gva = {{
                { 0, GL_FLOAT, 2, OFFSET(0),                    STRIDE, _vbo_xyuv.id()},
                { 1, GL_FLOAT, 2, OFFSET(2*sizeof (GLfloat)),   STRIDE, _vbo_xyuv.id()},
            }};

#ifdef NITROGL_SUPPORTS_VAO
            _vao.bind();
            _ebo.bind();
            program_type::point_generic_vertex_attributes(gva.data,
                     program_type::shader_vertex_attributes().data, GVA::size());
            vao_t::unbind();
#endif
        }

        void render(const program_type & program, sampler_t & sampler, const data_type & data) const {
            const auto & d = data;
            const bool has_missing_indices = d.indices == nullptr || d.indices_size==0;

            program.use();
            // vertex uniforms
            program.updateModelMatrix(d.mat_model);
            program.updateViewMatrix(d.mat_view);
            program.updateProjectionMatrix(d.mat_proj);
            program.updateUVsTransformMatrix(d.mat_uvs_sampler);

            // fragment uniforms
            program.update_backdrop_texture(d.backdrop_texture);
            program.update_window_size(d.window_width, d.window_height);
            program.updateOpacity(d.opacity);
            program.update_has_missing_uvs(false);
            program.update_has_missing_qs(true);

            // sampler uniforms
            sampler.upload_uniforms(program.id());

            static constexpr auto FLOAT_SIZE = GLsizeiptr (sizeof(float));
            static constexpr auto VEC2_SIZE = GLsizeiptr (sizeof(vec2f));

            // upload pos
            _vbo_xyuv.uploadData(d.xyuv, d.xyuv_size*FLOAT_SIZE,
                                GL_DYNAMIC_DRAW);

            // upload indices
            _ebo.uploadData(d.indices, GLsizeiptr(sizeof(GLuint))*d.indices_size,
                            GL_DYNAMIC_DRAW);

#ifdef NITROGL_SUPPORTS_VAO
            // VAO binds the: glEnableVertex attribs and pointing vertex attribs to VBO and binds the EBO
            _vao.bind();
            if(has_missing_indices) // non-indexed drawing, the EBO is bound BUT is not used
                glDrawArrays(d.triangles_type, 0, GLsizei(d.xyuv_size/4));
            else
                glDrawElements(d.triangles_type, GLsizei (d.indices_size), GL_UNSIGNED_INT, OFFSET(0));

            glCheckError();

            vao_t::unbind();
#else
            _ebo.bind();
            // this crates exccess 2 binds for vbos
            main_shader_program::point_generic_vertex_attributes(gva.data,
                                                                 main_shader_program::shader_vertex_attributes().data,
                                                                 gva.size());

            if(has_missing_indices) // non-indexed drawing, the EBO is bound BUT is not used
                glDrawArrays(d.triangles_type, 0, GLsizei(d.xyuv_size/4));
            else
                glDrawElements(d.triangles_type, GLsizei (d.indices_size), GL_UNSIGNED_INT, OFFSET(0));

            glCheckError();

            program.disableLocations(program_type::shader_vertex_attributes().data,
                                     program_type::shader_vertex_attributes().size());
#endif
            // un-use shader
            shader_program::unuse();
        }

    };

}