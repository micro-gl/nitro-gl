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

    class multi_render_node {

    public:
        using program_type = main_shader_program;
        using size_type = GLsizeiptr;
        struct data_type {
            const vec2f * pos;
            const vec2f * uvs;
            const float * qs;
            const GLuint * indices;

            size_type pos_size;
            size_type uvs_size;
            size_type qs_size;
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
            rectf bbox;
        };

        struct GVA {
            GVA()=default;
            static constexpr unsigned SIZE = 3;
            static constexpr unsigned size() { return SIZE; }
            nitrogl::generic_vertex_attrib_t data[SIZE];
        };

        GVA gva{};
        vbo_t _vbo_pos{}, _vbo_uvs{}, _vbo_qs{};
        vao_t _vao{};
        ebo_t _ebo{};

    public:
        multi_render_node()=default;
        ~multi_render_node()=default;

        void init() {
            // configure the vao, vbo, generic vertex attribs, non interleaved
            gva = {{
                { 0, GL_FLOAT, 2, OFFSET(0), 0, _vbo_pos.id()},
                { 1, GL_FLOAT, 2, OFFSET(0), 0, _vbo_uvs.id()},
                { 2, GL_FLOAT, 1, OFFSET(0), 0, _vbo_qs.id()}
            }};

#ifdef SUPPORTS_VAO
            _vao.bind();
            _ebo.bind();
            program_type::point_generic_vertex_attributes(gva.data,
                     program_type::shader_vertex_attributes().data, GVA::size());
            vao_t::unbind();
#endif
        }

        void render(const program_type & program, sampler_t & sampler, const data_type & data) const {
            const auto & d = data;
            const bool has_missing_uvs = d.uvs == nullptr;
            const bool has_missing_qs = d.qs == nullptr;

            program.use();
            // vertex uniforms
            glCheckError();
            program.updateModelMatrix(d.mat_model);
            glCheckError();
            program.updateViewMatrix(d.mat_view);
            program.updateProjectionMatrix(d.mat_proj);
            program.updateUVsTransformMatrix(d.mat_uvs_sampler);
            glCheckError();
            program.update_has_missing_uvs(has_missing_uvs);
            glCheckError();

            // fragment uniforms
            program.update_backdrop_texture(d.backdrop_texture);
            program.update_window_size(d.window_width, d.window_height);
            program.updateOpacity(d.opacity);
            glCheckError();
            program.update_has_missing_uvs(has_missing_uvs);
            glCheckError();
            program.update_has_missing_qs(has_missing_qs);
            glCheckError();
            if(has_missing_uvs)
                program.updateBBox(d.bbox.left, d.bbox.top, d.bbox.right, d.bbox.bottom);
            glCheckError();

            // sampler uniforms
            sampler.upload_uniforms(program.id());
            glCheckError();

            static constexpr auto FLOAT_SIZE = GLsizeiptr (sizeof(float));
            static constexpr auto VEC2_SIZE = GLsizeiptr (sizeof(vec2f));

            // upload pos
            _vbo_pos.uploadData(d.pos, d.pos_size*VEC2_SIZE*FLOAT_SIZE,
                                GL_DYNAMIC_DRAW);
            // upload uvs

            const vec2f * uvs = d.uvs;
            GLsizeiptr uvs_count = d.uvs_size;
            const float * qs = d.qs;
            GLsizeiptr qs_count = d.qs_size;

            // generate dummy uvs and qs if needed, because we have to supply something
            // otherwise, open-gl crashes for me if they are completely empty
            {
                static const vec2f dummy_uvs[1] = {{0,0}};
                static const float dummy_qs[1] = { 1.0f };
                if(has_missing_uvs) {
                    uvs = dummy_uvs;
                    uvs_count = 1;
                }
                if(has_missing_qs) {
                    qs = dummy_qs;
                    qs_count = 1;
                }
            }
            _vbo_uvs.uploadData(d.uvs, uvs_count*VEC2_SIZE * FLOAT_SIZE,
                            GL_DYNAMIC_DRAW);
            _vbo_qs.uploadData(d.qs,qs_count * FLOAT_SIZE,
                                GL_DYNAMIC_DRAW);
            // upload indices
            _ebo.uploadData(d.indices, GLsizeiptr(sizeof(GLuint))*d.indices_size,
                            GL_DYNAMIC_DRAW);
            glCheckError();

#ifdef SUPPORTS_VAO
            // VAO binds the: glEnableVertex attribs and pointing vertex attribs to VBO and binds the EBO
            _vao.bind();
            glDrawElements(d.triangles_type, GLsizei (d.indices_size), GL_UNSIGNED_INT, OFFSET(0));
            vao_t::unbind();
#else
            _ebo.bind();
            // this crates exccess 2 binds for vbos
            main_shader_program::point_generic_vertex_attributes(gva.data,
                    main_shader_program::vertex_attributes().data, gva.size());
            glDrawElements(GL_TRIANGLES, d.indices_size, GL_UNSIGNED_INT, OFFSET(0));
            _program.disableLocations(va.data, va.size());
#endif
            // un-use shader
            shader_program::unuse();
        }

    };

}