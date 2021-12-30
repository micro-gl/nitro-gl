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

    public:
        /**
         * NOTES about generic attribs to locations:
         * 1. After linking a program, every vertex attrib location is defined in the vertex
         *    shader. only relinking can change that.
         * 2. you can query the location by name after program linking of attributes and then point the arrays.
         * 3. you can decide the locations by name with glBindAttribLocation, but you will have to relink
         * 4. you can decide the locations inside the vertex shader with (location=#) specifier
         *
         * a bit of defs:
         * 1. generic vertex attribute = spread out vec1, vec2, vec3 or vec4
         * 2. each vec is made up of components
         * 3. interleaved examples = [(x,y,z),(u,v), (x,y,z),(u,v), (x,y,z),(u,v), ...]
         * 4. non-interleaved examples = [(x,y,z),(x,y,z),(x,y,z), (u,v),(u,v),(u,v), ...]
         * 5. stride is constant for interleaved = sizeof((x,y,z))+sizeof((u,v)) = 5
         * 6. stride for non-interleaved is per vertex attribute. stride((x,y,z))=sizeof((x,y,z))=3, stride((u,v))=2
         *
         */
        struct attr_t {
            const GLchar * name=nullptr; // name of vertex attribute
            GLint location=-1; // the index of the attribute in the vertex shader
            GLenum type=GL_FLOAT; // the type of element in attribute
            GLuint size=1; // the number of elements in attribute (1,2,3,4)
            // the attribute's first relative occurrence offset in the buffer
            const void * offset=nullptr;
            // stride can be calculated automatically if the buffer is interleaved or non.
        };

        struct uniform_t {
            const GLchar * name=nullptr; // name of uniform
            GLint location=0; // the location
        };

        static shader_program from_shaders(const shader & vertex, const shader & fragment) {
            shader_program prog;
            prog.create();
            prog.attach_shaders(vertex, fragment);
            return prog;
        }
        // ctor: init with empty shaders and attach which is legal
        shader_program() : _vertex(shader::type::vertex), _fragment(shader::type::fragment), _id(0) {}
        ~shader_program() { _id=0; }

        bool wasCreated() const { return _id; }
        void create() { if(!_id) _id = glCreateProgram(); }
        void attach_shaders(const shader & vertex, const shader & fragment) {
            detachShaders();
            _vertex = vertex;
            _fragment = fragment;
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
        }
        void detachShaders() const {
            glDetachShader(_id, _vertex.id());
            glDetachShader(_id, _fragment.id());
        }
        GLuint id() const { return _id; }
        shader & vertex() { return _vertex; }
        shader & fragment() { return _fragment; }
        void use() const { glUseProgram(_id); }
        static void unUse() { glUseProgram(0); }
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
        void del() {
            detachShaders();
            glDeleteProgram(_id);
            _id=0;
        }

        // generic attributes
        //
        bool wasLastLinkSuccessful() const {
            GLint stat; glGetProgramiv(_id, GL_LINK_STATUS, &stat); return stat;
        }
        void bindAttribLocation(GLuint index, const GLchar *name) const {
            glBindAttribLocation(_id, index, name);
            link(); // you have to link again
        }
        void bindAttribsLocations(const attr_t * attributes, const unsigned size) const {
            for (int ix = 0; ix < size; ++ix) {
                const auto loc = attributes[ix].location;
                if(loc) glBindAttribLocation(_id, loc, attributes[ix].name);
            }
            link(); // you have to link again
        }
        GLint attributeLocationByName(const GLchar * name) const {
            return glGetAttribLocation(_id, name);
        }

        /**
         *
         * three modes:
         * 1. { location<0, name!=nullptr } --> first link if hasn't, then glGetAttribLocation
         * 1. { location>=0, name!=nullptr } --> first glBindAttribLocation, and re-link at end
         * 1. { location>=0, name==nullptr } --> we assume user used (location=#) specifier in shader, nothing to do
         * 1. { location<0, name==nullptr } --> error
         * @return for errors
         */
        bool pointVertexAttributes(attr_t *attrs, const unsigned length, bool interleaved) const {
            bool has_user_defined_all_locations = true;
            {
                for (const attr_t * iter = attrs; iter < attrs+length; ++iter)
                    if(iter->location<0) {
                        has_user_defined_all_locations = false;
                        break;
                    }
            }

            // if user relies on auto-locations and querying them with names, we have to make
            // sure, the program was linked at least once.
            if(!has_user_defined_all_locations && !wasLastLinkSuccessful()) link();

            GLuint interleaved_window_size=0;
            if(interleaved) {
                // if interleaved, let's find out the total length in bytes of the window
                for (const attr_t * iter = attrs; iter < attrs+length; ++iter) {
                    GLuint single_size = 0;
                    switch(attrs->type) {
                        case GL_FLOAT: single_size = sizeof(GLfloat); break;
                        case GL_UNSIGNED_INT: single_size = sizeof(GLuint); break;
                        case GL_INT: single_size = sizeof(GLint); break;
                        case GL_BYTE: single_size = sizeof(GLbyte); break;
                        case GL_UNSIGNED_BYTE: single_size = sizeof(GLubyte); break;
                        case GL_SHORT: single_size = sizeof(GLshort); break;
                        case GL_UNSIGNED_SHORT: single_size = sizeof(GLushort); break;
                        default:break;
                    }
                    interleaved_window_size += (iter->size * single_size);
                }
            }

            bool binding_occured = false;
            for (attr_t * it = attrs; it < attrs + length; ++it) {
                if(it->location<0) { // -1=user asks opengl for auto-location
                    if(it->name==nullptr) return false; // must have name
                    it->location = glGetAttribLocation(_id, it->name);
                } else if(it->name) { // let's bind requested location to shader vertex's attribute name
                    glBindAttribLocation(_id, it->location, it->name);
                    binding_occured = true;
                } else { // it->location>=0 and it->name==nullptr
                    // We assume user has used (location=#) specifier in shader, so nothing to do here
                }

                if(it->location == -1) { // attribute was not found or optimized away
                    //log("Error:: attribute %s was not found or was optimized away by compiler ", it->name);
                    continue;
                }

                GLuint single_size = 0;
                switch(it->type) {
                    case GL_FLOAT: single_size = sizeof(GLfloat); break;
                    case GL_UNSIGNED_INT: single_size = sizeof(GLuint); break;
                    case GL_INT: single_size = sizeof(GLint); break;
                    case GL_BYTE: single_size = sizeof(GLbyte); break;
                    case GL_UNSIGNED_BYTE: single_size = sizeof(GLubyte); break;
                    case GL_SHORT: single_size = sizeof(GLshort); break;
                    case GL_UNSIGNED_SHORT: single_size = sizeof(GLushort); break;
                    default:break;
                }
                // calculate stride
                GLsizei stride = interleaved ? GLsizei(interleaved_window_size) : GLsizei(single_size * it->size);
                // enable generic vertex attrib for bound VBO
                glEnableVertexAttribArray((GLuint)it->location);
                // associate shader attribute with vertex buffer position in VBO
                glVertexAttribPointer((GLuint)it->location, GLint(it->size), it->type, GL_FALSE,
                                      GLsizei(single_size * it->size),
                                      it->offset);
            }

            // we must relink to make bind take effect
            if(binding_occured) link();
            return true;
        }

        // uniforms
        GLint uniformLocationByName(const GLchar * name) const {
            return glGetUniformLocation(_id, name);
        }

        // matrices
        GLint updateUniformMatrix2fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix2fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix3fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix3fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix4fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix4fv(location, 1, GL_FALSE, value);
            return location;
        }

#ifndef STRICT_ES2
        GLint updateUniformMatrix2x3fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix2x3fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix3x2fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix3x2fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix2x4fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix2x4fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix4x2fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix4x2fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix3x4fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix3x4fv(location, 1, GL_FALSE, value);
            return location;
        }
        GLint updateUniformMatrix4x3fv(const GLchar * name, const GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniformMatrix4x3fv(location, 1, GL_FALSE, value);
            return location;
        }
#endif

        // uniform 1
        GLint updateUniform1f(const GLchar * name, GLfloat x) const {
            const auto location = uniformLocationByName(name);
            glUniform1f(location, x);
            return location;
        }
        GLint updateUniform1fv(const GLchar * name, GLsizei length, GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniform1fv(location, length, value);
            return location;
        }
        GLint updateUniform1i(const GLchar * name, GLint x) const {
            const auto location = uniformLocationByName(name);
            glUniform1i(location, x);
            return location;
        }
        GLint updateUniform1iv(const GLchar * name, GLsizei length, GLint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform1iv(location, length, value);
            return location;
        }

#ifndef STRICT_ES2
        GLint updateUniform1ui(const GLchar * name, GLuint x) const {
            const auto location = uniformLocationByName(name);
            glUniform1ui(location, x);
            return location;
        }
        GLint updateUniform1uiv(const GLchar * name, GLsizei length, GLuint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform1uiv(location, length, value);
            return location;
        }
#endif

        // uniform 2
        GLint updateUniform2f(const GLchar * name, GLfloat x, GLfloat y) const {
            const auto location = uniformLocationByName(name);
            glUniform2f(location, x, y);
            return location;
        }
        GLint updateUniform2fv(const GLchar * name, GLsizei length, GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniform2fv(location, length, value);
            return location;
        }
        GLint updateUniform2i(const GLchar * name, GLint x, GLint y) const {
            const auto location = uniformLocationByName(name);
            glUniform2i(location, x, y);
            return location;
        }
        GLint updateUniform2iv(const GLchar * name, GLsizei length, GLint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform2iv(location, length, value);
            return location;
        }

#ifndef STRICT_ES2
        GLint updateUniform2ui(const GLchar * name, GLuint x, GLuint y) const {
            const auto location = uniformLocationByName(name);
            glUniform2ui(location, x, y);
            return location;
        }
        GLint updateUniform2uiv(const GLchar * name, GLsizei length, GLuint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform2uiv(location, length, value);
            return location;
        }
#endif

        // uniform 3
        GLint updateUniform3f(const GLchar * name, GLfloat x, GLfloat y, GLfloat z) const {
            const auto location = uniformLocationByName(name);
            glUniform3f(location, x, y, z);
            return location;
        }
        GLint updateUniform3fv(const GLchar * name, GLsizei length, GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniform3fv(location, length, value);
            return location;
        }
        GLint updateUniform3i(const GLchar * name, GLint x, GLint y, GLint z) const {
            const auto location = uniformLocationByName(name);
            glUniform3i(location, x, y, z);
            return location;
        }
        GLint updateUniform3iv(const GLchar * name, GLsizei length, GLint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform3iv(location, length, value);
            return location;
        }

#ifndef STRICT_ES2
        GLint updateUniform3ui(const GLchar * name, GLuint x, GLuint y, GLuint z) const {
            const auto location = uniformLocationByName(name);
            glUniform3ui(location, x, y, z);
            return location;
        }
        GLint updateUniform3uiv(const GLchar * name, GLsizei length, GLuint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform3uiv(location, length, value);
            return location;
        }
#endif

        // uniform 4
        GLint updateUniform4f(const GLchar * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const {
            const auto location = uniformLocationByName(name);
            glUniform4f(location, x, y, z, w);
            return location;
        }
        GLint updateUniform4fv(const GLchar * name, GLsizei length, GLfloat *value) const {
            const auto location = uniformLocationByName(name);
            glUniform4fv(location, length, value);
            return location;
        }
        GLint updateUniform4i(const GLchar * name, GLint x, GLint y, GLint z, GLint w) const {
            const auto location = uniformLocationByName(name);
            glUniform4i(location, x, y, z, w);
            return location;
        }
        GLint updateUniform4iv(const GLchar * name, GLsizei length, GLint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform4iv(location, length, value);
            return location;
        }

#ifndef STRICT_ES2
        GLint updateUniform4ui(const GLchar * name, GLuint x, GLuint y, GLuint z, GLuint w) const {
            const auto location = uniformLocationByName(name);
            glUniform4ui(location, x, y, z, w);
            return location;
        }
        GLint updateUniform4uiv(const GLchar * name, GLsizei length, GLuint *value) const {
            const auto location = uniformLocationByName(name);
            glUniform4uiv(location, length, value);
            return location;
        }
#endif
    };

}