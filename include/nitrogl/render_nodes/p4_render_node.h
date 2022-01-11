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

    /**
     * optimized node for 4 point meshes. saves uploads for ebo, and uses interleaving
     */
    class p4_render_node {

    public:
        using program_type = main_shader_program;
        using size_type = nitrogl::size_t;
        struct data_type {
            float * pos_and_uvs_interleaved; //{(x,y,u,v,p,q), (x,y,u,v,p,q), ....}
            size_type size;
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
        vbo_t _vbo_pos_uvs;
        vao_t _vao;
        ebo_t _ebo;

    public:
        p4_render_node()=default;
        ~p4_render_node()=default;

        void init() {
            // configure the vao, vbo, generic vertex attribs
            gva = {{
                { 0, GL_FLOAT, 2, OFFSET(0),
                  6*sizeof (GLfloat),_vbo_pos_uvs.id()},
                { 1, GL_FLOAT, 4, OFFSET(2*sizeof (GLfloat)),
                  6*sizeof (GLfloat), _vbo_pos_uvs.id()}
            }};

            // elements buffer
            GLuint e[6] = { 0, 1, 2, 2, 3, 0 };
            _vao.bind();
            _ebo.bind();
            _ebo.uploadData(e, sizeof(e), GL_STATIC_DRAW);

#ifdef SUPPORTS_VAO
            program_type::point_generic_vertex_attributes(gva.data,
                     program_type::shader_vertex_attributes().data, gva.size());
            vao_t::unbind();
#endif
        }

        void render(const program_type & program, sampler_t & sampler, const data_type & data) {
            const auto & d = data;
            program.use();
            program.updateModelMatrix(d.mat_model);
            program.updateViewMatrix(d.mat_view);
            program.updateProjectionMatrix(d.mat_proj);
            program.updateUVsTransformMatrix(d.mat_uvs_sampler);
            program.updateOpacity(1.0f);
            sampler.on_upload_uniforms();

            glCheckError();

            // upload data
            _vbo_pos_uvs.uploadData(d.pos_and_uvs_interleaved,
                                d.size*sizeof(float),
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