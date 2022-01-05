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
#include "render_node.h"
#include "../color.h"
#include <array>

namespace nitrogl {
    bool interleave=false;

    class main_render_node : public render_node<main_shader_program> {
        using base = render_node<main_shader_program>;
    public:
        struct data_type {
            color_t color;
            float * pos;
            float * uvs_sampler;
            int pos_size;
            int uvs_sampler_size;
        };

        template<unsigned N>
        struct VA {
            shader_program::vbo_and_shader_attr_t data[N];
            constexpr unsigned size() const { return N; }
        };

        VA<2> va;
        vbo_t _vbo_pos, _vbo_uvs_sampler;

    public:

        main_render_node() : main_render_node(main_shader_program()) {}
        explicit main_render_node(const main_shader_program & program) : base(program),
                    va(), _vbo_pos(), _vbo_uvs_sampler() {}
        explicit main_render_node(main_shader_program && program ) noexcept :
                base(nitrogl::traits::move(program)), va(), _vbo_pos(), _vbo_uvs_sampler() {
        }
        main_render_node(const main_render_node &)=delete;
        main_render_node(main_render_node &&)=delete;
        main_render_node & operator=(main_render_node &&)=delete;
        main_render_node & operator=(const main_render_node &)=delete;
        ~main_render_node()=default;

        void init() {
            base::init();

            va = {
                {
                    {"VS_pos", 0, GL_FLOAT,
                     shader_program::shader_attribute_component_type::Float,
                     2, OFFSET(0), 0, _vbo_pos.id()},
                    {"VS_uvs_sampler", 2, GL_FLOAT,
                     shader_program::shader_attribute_component_type::Float,
                     2, OFFSET(0), 0, _vbo_uvs_sampler.id()}
                }
            };
            // enable and point vertex attributes
            _program.queryOrBindVertexAttributesLocations(va.data, va.size());

            // elements buffer
            GLuint e[6] = { 0, 1, 2, 2, 3, 0 };

            _vao.bind();
            _ebo.bind();
            _ebo.uploadData(e, sizeof(e));
#ifdef SUPPORTS_VAO
            _program.pointVertexAtrributes(va.data, va.size());
            vao_t::unbind();
#endif
        }

        void render(const data_type & data) {
            const auto & d = data;
            _program.use();
            _program.updateModelMatrix(_mat_model);
            _program.updateViewMatrix(_mat_view);
            _program.updateProjectionMatrix(_mat_proj);
            _program.updateUVsTransformMatrix(_mat_transform_uvs_sampler);
            _program.updateOpacity(1.0f);
            _program.updateColor(d.color.r, d.color.g, d.color.b, d.color.a);
            glCheckError();

            // data

            GLfloat pos[8] =  {
                    10.0f, 10.0f, // Bottom-left
                    250.0f, 10.0f, // Bottom-right
                    250.0f, 250.0f, // Top-right
                    10.0f, 250.0f, // Top-left
            };

            GLfloat pos3[8] =  {
                    50.0f, 50.0f, // Bottom-left
                    100.0f, 50.0f, // Bottom-right
                    100.0f, 100.0f, // Top-right
                    50.0f, 100.0f, // Top-left
            };

            GLfloat uv[12] =  {
                    0.0f, 0.0f, 1.0f, // Bottom-left
                    1.0f, 0.0f, 1.0f, // Bottom-right
                    1.0f, 1.0f, 1.0f, // Top-right
                    0.0f, 1.0f, 1.0f, // Top-left
            };

            _vbo_pos.uploadData(d.pos, d.pos_size*sizeof(float), GL_DYNAMIC_DRAW);
            _vbo_uvs_sampler.uploadData(d.uvs_sampler, d.uvs_sampler_size*sizeof(float), GL_DYNAMIC_DRAW);

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

        mat3f _mat_transform_uvs_sampler;

        void updateUVsMatrix(mat3f val) {
            _mat_transform_uvs_sampler=val;
        }
    };

}