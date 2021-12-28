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
            // ONLY for non-interleaved. it's first relative occurrence offset in the buffer
            const void * offset=nullptr;
        };

        struct uniform_t {
            const GLchar * name=nullptr; // name of uniform
            GLint location=0; // the location
        };

        shader_program(const shader & vertex, const shader & fragment) :
                _vertex(vertex), _fragment(fragment), _id(0) {
            _id = glCreateProgram();
            glAttachShader(_id, _vertex.id());
            glAttachShader(_id, _fragment.id());
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
        void del() {
            glDetachShader(_id, _vertex.id());
            glDetachShader(_id, _fragment.id());
            glDeleteProgram(_id);
            _id=0;
        }

        // generic attributes
        //
        void bindAttribLocation(GLuint index, const GLchar *name) const {
            glBindAttribLocation(_id, index, name);
            link();
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

        void pointVertexAttributes(attr_t *attrs, const unsigned length, bool interleaved) const {
            use();
            GLuint interleaved_window_size = 0;
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

            GLuint offset_bytes = 0;

            for (attr_t * iter = attrs; iter < attrs+length; ++iter) {
                GLint location = glGetAttribLocation(_id, iter->name);
                GLuint single_size = 0;

                if(location == -1) {
                    //log("Error:: attribute %s was not found or was optimized away by compiler ", iter->name);
                    continue;
                }

                switch(iter->type) {
                    case GL_FLOAT: single_size = sizeof(GLfloat); break;
                    case GL_UNSIGNED_INT: single_size = sizeof(GLuint); break;
                    case GL_INT: single_size = sizeof(GLint); break;
                    case GL_BYTE: single_size = sizeof(GLbyte); break;
                    case GL_UNSIGNED_BYTE: single_size = sizeof(GLubyte); break;
                    case GL_SHORT: single_size = sizeof(GLshort); break;
                    case GL_UNSIGNED_SHORT: single_size = sizeof(GLushort); break;
                    default:break;
                }

                // enable generic vertex attrib for bound VBO
                glEnableVertexAttribArray((GLuint)location);

                if(interleaved) {
                    // associate shader attribute with vertex buffer position in VBO
                    glVertexAttribPointer((GLuint)location, GLint(iter->size), iter->type, GL_FALSE,
                                          GLsizei(interleaved_window_size), // stride
                                          (void*)BUFFER_OFFSET(offset_bytes));
                    offset_bytes += (iter->size * single_size);
                } else {
                    // associate shader attribute with vertex buffer position in VBO
                    glVertexAttribPointer((GLuint)location, GLint(iter->size), iter->type, GL_FALSE,
                                          GLsizei(single_size * iter->size),
                                          iter->offset);
                }
                iter->location = location;
            }
            unUse();
        }
    };
}