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

#include "shader.h"

namespace nitrogl {
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

    class shader_program {
    private:
        shader _vertex, _fragment;
        GLuint _id;

        enum class type { vertex, fragment, unknown };
        static GLenum type2enum(const type t) { return t==type::vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER; }

    public:
        shader_program(const shader & vertex, const shader & fragment) :
                _vertex(vertex), _fragment(fragment), _id(0) {
            _id = glCreateProgram();
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
            link();
        }
        ~shader_program() { del(); }

        GLuint id() const { return _id; }
        const shader & vertex() const { return _vertex; }
        const shader & fragment() const { return _fragment; }
        void use() const { glUseProgram(_id); }
        void unUse() const { glUseProgram(0); }
        bool link() const {
            glLinkProgram(_id);
            GLint is_linked;
            // Check the link status
            glGetProgramiv(_id, GL_LINK_STATUS, &is_linked);
            return is_linked;
        }

        GLint info_log(char * log_buffer = nullptr, GLint log_buffer_size=0) const {
            if(!log_buffer or !glIsProgram(_id)) return 0;
            // Shader copied log length
            GLint copied_length = 0;
            // copy info log
            glGetProgramInfoLog(_id, log_buffer_size, &copied_length, log_buffer);
            return copied_length;
        }

        /**
         * copy the source code of shader to a buffer. This is better than storing pointers
         * to char arrays that were uploaded to GPU, because they are not guaranteed to be static.
         * @return length of copied source
         */
        GLsizei get_source(char * buff, GLsizei buff_size) const {
            GLsizei length;
            glGetShaderSource(_id, buff_size, &length, buff);
            return length;
        }
        type shader_type() const { return _type; }
        bool isVertexShader() const { return _type==type::vertex; }
        bool isFragmentShader() const { return _type==type::fragment; }
        bool isCompiled() const { return _is_compiled; }
        bool compile(char * log_bufer = nullptr, GLint log_buffer_size=0) {
            if(isCompiled()) return true;
            glCompileShader(_id);
            //Check shader for errors
            GLint compile_status = GL_FALSE;
            glGetShaderiv(_id, GL_COMPILE_STATUS, &compile_status);
            // record
            if(log_bufer && compile_status!=GL_TRUE) {
                //Make sure name is shader
                if(glIsShader(_id)) {
                    // Shader copied log length
                    GLint copied_length = 0;
                    // copy info log
                    glGetShaderInfoLog(_id, log_buffer_size, &copied_length, log_bufer);
                }
            }
            _is_compiled = compile_status;
            return compile_status;
        }
        void del() {
            glDetachShader(_id, _vertex.id());
            glDetachShader(_id, _fragment.id());
            glDeleteProgram(_id);
            _id=0;
        }

    private:
    };
}