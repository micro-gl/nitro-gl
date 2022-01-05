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

#include "shader_program.h"
#include "main_shader_program.h"
#include "../color.h"
#include <array>

namespace nitrogl {

    class main_render_node {

    public:
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

        template<unsigned N>
        struct VA {
            VA()=default;
            shader_program::vbo_and_shader_attr_t data[N];
            constexpr unsigned size() const { return N; }
        };

        VA<2> va;
        vbo_t _vbo_pos, _vbo_uvs_sampler;
        vao_t _vao;
        ebo_t _ebo;
        main_shader_program _program;

    public:
        main_render_node()=default;
        ~main_render_node()=default;

        void init() {

            va = {
                {
                    {"VS_pos", 0, GL_FLOAT,
                     shader_program::shader_attribute_component_type::Float,
                     2, OFFSET(0), 0, _vbo_pos.id()},
                    {"VS_uvs_sampler", 1, GL_FLOAT,
                     shader_program::shader_attribute_component_type::Float,
                     2, OFFSET(0), 0, _vbo_uvs_sampler.id()}
                }
            };
            // enable and point vertex attributes
            _program.queryOrBindVertexAttributesLocations(va.data, va.size());

            // elements buffer
//            GLuint e[6] = { 0, 1, 2, 2, 3, 0 };
//            _vao.bind();
//            _ebo.bind();
//            _ebo.uploadData(e, sizeof(e));

#ifdef SUPPORTS_VAO
            _vao.bind();
            _ebo.bind();
            _program.pointVertexAtrributes(va.data, va.size());
            vao_t::unbind();
#else

#endif
        }

        void render(const data_type & data) {
            const auto & d = data;
            _program.use();
            _program.updateModelMatrix(d.mat_model);
            _program.updateViewMatrix(d.mat_view);
            _program.updateProjectionMatrix(d.mat_proj);
            _program.updateUVsTransformMatrix(d.mat_uvs_sampler);
            _program.updateOpacity(1.0f);
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
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, OFFSET(0));
            vao_t::unbind();
#else
            _ebo.bind();
            // this crates exccess 2 binds for vbos
            _program.pointVertexAtrributes(va.data, va.size());
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            _program.disableLocations(va.data, va.size());
#endif
            // unuse shader
            shader_program::unuse();
        }

    };

}