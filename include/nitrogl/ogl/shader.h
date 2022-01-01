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

    public:
        enum class type { vertex, fragment, unknown };
        static GLenum type2enum(const type t) { return t==type::vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER; }

        static shader from(const type shader_type, const GLchar ** sources, GLsizei count=1,
                           const GLint *length=nullptr) {
            auto shade = shader(shader_type);
            shade.create();
            shade.updateShaderSource(sources, count, length, true);
            return shade;
        }
        static shader from(const type shader_type, const GLchar * source)
        { return from(shader_type, &source, 1, nullptr); }
        static shader from_vertex(const GLchar ** sources, GLsizei count=1, const GLint *length=nullptr)
        { return from(type::vertex, sources, count, length); }
        static shader from_fragment(const GLchar ** sources, GLsizei count=1, const GLint *length=nullptr)
        { return from(type::fragment, sources, count, length); }
        static shader from_vertex(const GLchar * source)
        { return from(type::vertex, &source, 1, nullptr); }
        static shader from_fragment(const GLchar * source)
        { return from(type::fragment, &source, 1, nullptr); }

    private:
        GLuint _id;
        type _type;
        bool _is_compiled;

    public:
        explicit shader(const type shader_type) : _type(shader_type), _is_compiled(false), _id(0) {
            create();
        }
        shader(shader && o)  noexcept : _id(o._id), _type(o._type), _is_compiled(o._is_compiled) { o._id=0; }
        shader(const shader &)=default;
        shader & operator=(const shader & ) = default;
        shader & operator=(shader && o)  noexcept {
            _id=o._id; _type=o._type; _is_compiled=o._is_compiled;
            o._id=0; return *this;
        }

        ~shader() { del(); }

        void create() { if(!_id and _type!=type::unknown) { _id = glCreateShader(type2enum(_type)); } }
        bool wasCreated() const { return _id; }
        /**
         * @param sources array of char arrays, each is a source code for shader
         * @param count number of sources
         * @param length If length is NULL, each string is assumed to be null terminated. If length
         * is a value other than NULL, it points to an array containing a string length for each of
         * the corresponding elements of string. Each element in the length array may contain the length
         * of the corresponding string (the null character is not counted as part of the string length) or
         * a value less than 0 to indicate that the string is null terminated.
         */
        bool updateShaderSource(const GLchar ** sources, GLsizei count=1, const GLint *length=nullptr,
                                bool compile_right_away=false) {
            if(!wasCreated()) return false;
            glShaderSource(_id, count, sources, length);
            if(compile_right_away) compile();
            return true;
        }
        bool updateShaderSource(const GLchar * source, bool compile_right_away=false) {
            return updateShaderSource(&source, 1, nullptr, compile_right_away);
        }
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
            if(wasCreated()) return false;
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

    };
}