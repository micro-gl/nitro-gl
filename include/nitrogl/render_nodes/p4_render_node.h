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
#include "../_internal/main_shader_program.h"
#include "../samplers/sampler.h"

namespace nitrogl {

    /**
     * optimized node for 4 point meshes. saves uploads for ebo, and uses interleaving
     */
    class p4_render_node {

    public:
        using program_type = main_shader_program;
        using size_type = GLsizeiptr;
        struct data_type {
            float * pos_and_uvs_qs_interleaved; //{(x,y,u,v,q), (x,y,u,v,q), ....}
            size_type size;
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
            nitrogl::generic_vertex_attrib_t data[3];
            static constexpr unsigned size() { return 3; }
        };

        GVA gva{};
        vbo_t _vbo_pos_uvs_qs{};
        vao_t _vao{};
        ebo_t _ebo{};

    public:
        p4_render_node()=default;
        ~p4_render_node()=default;

        void init() {
            // configure the vao, vbo, generic vertex attribs [(x,y,u,v,q) ....], interleaved
            gva = {{
                { 0, GL_FLOAT, 2, OFFSET(0),
                  5*sizeof (GLfloat), _vbo_pos_uvs_qs.id()},
                { 1, GL_FLOAT, 2, OFFSET(2*sizeof (GLfloat)),
                5*sizeof (GLfloat), _vbo_pos_uvs_qs.id()},
                { 2, GL_FLOAT, 1, OFFSET(4*sizeof (GLfloat)),
                  5*sizeof (GLfloat), _vbo_pos_uvs_qs.id()}
            }};

            // elements buffer
            GLuint e[6] = { 0, 1, 2, 2, 3, 0 };
            _vao.bind();
            _ebo.uploadData(e, sizeof(e), GL_STATIC_DRAW);

#ifdef SUPPORTS_VAO
            program_type::point_generic_vertex_attributes(gva.data,
                     program_type::shader_vertex_attributes().data, GVA::size());
            vao_t::unbind();
#endif
        }

        void render(const program_type & program, sampler_t & sampler, const data_type & data) const {
            const auto & d = data;
            program.use();
            // vertex uniforms
            program.updateModelMatrix(d.mat_model);
            program.updateViewMatrix(d.mat_view);
            program.updateProjectionMatrix(d.mat_proj);
            program.updateUVsTransformMatrix(d.mat_uvs_sampler);
            program.update_has_missing_uvs(false);
            program.update_has_missing_qs(false);

            // fragment uniforms
            program.update_backdrop_texture(d.backdrop_texture);
            program.update_window_size(d.window_width, d.window_height);
            program.updateOpacity(d.opacity);

            // sampler uniforms
            sampler.upload_uniforms(program.id());

//            glCheckError();

            static constexpr auto FLOAT_SIZE = GLsizeiptr (sizeof(float));
            // upload data
            _vbo_pos_uvs_qs.uploadData(d.pos_and_uvs_qs_interleaved,
                                       d.size*FLOAT_SIZE,
                                       GL_DYNAMIC_DRAW);

#ifdef SUPPORTS_VAO
            // VAO binds the: glEnableVertex attribs and pointing vertex attribs to VBO and binds the EBO
            _vao.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, OFFSET(0));
            vao_t::unbind();
#else
            _ebo.bind();
            // this crates exccess 2 binds for vbos
            program_type::point_generic_vertex_attributes(gva.data,
                    program_type::shader_vertex_attributes().data, gva.size());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, OFFSET(0));
            _program.disableLocations(va.data, va.size());
#endif
            // unuse shader
            shader_program::unuse();
        }

    };

}