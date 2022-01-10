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

#include "nitrogl/ogl/shader_program.h"
#include "nitrogl/_internal/main_shader_program.h"
#include "nitrogl/color.h"
#include <array>

namespace nitrogl {

    class multi_render_node {

    public:
        using program_type = main_shader_program;
        using size_type = nitrogl::size_t;
        struct data_type {
            float * pos;
            float * uvs_sampler;
            GLuint * indices;
            size_type pos_size;
            size_type uvs_sampler_size;
            size_type indices_size;
            GLenum triangles_type;
            const mat4f & mat_model;
            const mat4f & mat_view;
            const mat4f & mat_proj;
            const mat3f & mat_uvs_sampler;
        };

        struct GVA {
            GVA()=default;
            nitrogl::generic_vertex_attrib_t data[2];
            constexpr unsigned size() const { return 2; }
        };

        GVA gva{};
        vbo_t _vbo_pos, _vbo_uvs_sampler;
        vao_t _vao;
        ebo_t _ebo;

    public:
        multi_render_node()=default;
        ~multi_render_node()=default;

        void init() {
            // configure the vao, vbo, generic vertex attribs
            gva = {{
                { 0, GL_FLOAT, 2, OFFSET(0), 0, _vbo_pos.id()},
                { 1, GL_FLOAT, 2, OFFSET(0), 0, _vbo_uvs_sampler.id()}
            }};

#ifdef SUPPORTS_VAO
            _vao.bind();
            _ebo.bind();
            program_type::point_generic_vertex_attributes(gva.data,
                     program_type::shader_vertex_attributes().data, gva.size());
            vao_t::unbind();
#endif
        }

        void render(const program_type & program, const data_type & data) {
            const auto & d = data;
            program.use();
            program.updateModelMatrix(d.mat_model);
            program.updateViewMatrix(d.mat_view);
            program.updateProjectionMatrix(d.mat_proj);
            program.updateUVsTransformMatrix(d.mat_uvs_sampler);
            program.updateOpacity(1.0f);
            glCheckError();

            // upload data
            _vbo_pos.uploadData(d.pos,
                                d.pos_size*sizeof(float),
                                GL_DYNAMIC_DRAW);
            _vbo_uvs_sampler.uploadData(d.uvs_sampler,
                                        d.uvs_sampler_size*sizeof(float),
                                        GL_DYNAMIC_DRAW);
            _ebo.uploadData(d.indices,
                            sizeof(GLuint)*d.indices_size,
                            GL_DYNAMIC_DRAW);

#ifdef SUPPORTS_VAO
            // VAO binds the: glEnableVertex attribs and pointing vertex attribs to VBO and binds the EBO
            _vao.bind();
            glDrawElements(GL_TRIANGLES, d.indices_size, GL_UNSIGNED_INT, OFFSET(0));
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