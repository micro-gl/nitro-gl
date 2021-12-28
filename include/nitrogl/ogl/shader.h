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

namespace nitrogl {
    class shader {
        enum class type { vertex, fragment, unknown };
        static GLenum type2enum(const type t) { return t==type::vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER; }

    public:
        /**
         *
         * @param sources array of char arrays, each is a source code for shader
         * @param count number of sources
         * @param length If length is NULL, each string is assumed to be null terminated. If length
         * is a value other than NULL, it points to an array containing a string length for each of
         * the corresponding elements of string. Each element in the length array may contain the length
         * of the corresponding string (the null character is not counted as part of the string length) or
         * a value less than 0 to indicate that the string is null terminated.
         * @param shader_type vertex or fragment enum
         * @param compile compile right away ?
         */
        shader(const type shader_type, const GLchar ** sources, GLsizei count=1,
               const GLint *length=nullptr, bool compile_right_away=false) :
                _type(shader_type), _is_compiled(false), _id(0) {
            _id = glCreateShader(type2enum(_type));
            // upload source code as soon as we have sources, we don't know how long
            // these pointed objects will last
            glShaderSource(_id, count, sources, length);
            if(compile_right_away) compile();
        }
        /**
         * Convenience Ctor. Shader's source is assumed to be null terminated
         */
        shader(const type shader_type, const GLchar * source, bool compile_right_away=false) :
                shader(shader_type, &source, 1, nullptr, compile_right_away) { }
        ~shader() { del(); }

        GLuint id() const { return _id; }
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
        bool compile() {
            if(isCompiled()) return true;
            glCompileShader(_id);
            //Check shader for errors
            GLint compile_status = GL_FALSE;
            glGetShaderiv(_id, GL_COMPILE_STATUS, &compile_status);
            _is_compiled = compile_status;
            return compile_status;
        }
        GLint info_log(char * log_buffer = nullptr, GLint log_buffer_size=0) const {
            if(!log_buffer or !glIsShader(_id)) return 0;
            // Shader copied log length
            GLint copied_length = 0;
            // copy info log
            glGetShaderInfoLog(_id, log_buffer_size, &copied_length, log_buffer);
            return copied_length;
        }
        void del() { glDeleteShader(_id); _is_compiled=false; _id=0; _type=type::unknown; }

    private:
        GLuint _id;
        type _type;
        bool _is_compiled;
    };
}