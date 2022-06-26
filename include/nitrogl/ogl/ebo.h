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

#include "debug.h"

namespace nitrogl {

    class ebo_t {
        GLuint _id;
        bool owner;

        void generate() { if(!_id) glGenBuffers(1, &_id); glCheckError(); }
        ebo_t(GLuint id, bool owner) : _id(id), owner(owner) {};

    public:
        static ebo_t from_id(GLuint id, bool owner=true) { return { id, owner }; }
        ebo_t() : _id(0), owner(true) { generate(); };
        ebo_t(ebo_t && o)  noexcept : _id(o._id), owner(o.owner) { o.owner=false; }
        ebo_t(const ebo_t & o) : _id(o._id), owner(false) {}
        ebo_t & operator=(const ebo_t & o) {
            if(&o!=this) {
                del(); _id=o._id; owner=false;
            }
            return *this;
        };
        ebo_t & operator=(ebo_t && o) noexcept {
            if(&o!=this) {
                del(); _id=o._id; owner=o.owner; o.owner=false;
            }
            return *this;
        }
        ~ebo_t() { del(); unbind(); }

        bool wasGenerated() const { return _id; }
        void uploadData(const GLuint * array, GLsizeiptr array_size_bytes, GLenum usage=GL_STATIC_DRAW) const {
            if(_id==0) return;
            bind();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, array_size_bytes, array, usage); glCheckError();
        }
        GLuint id() const { return _id; }
        void del() { if(_id && owner) { glDeleteBuffers(1, &_id); glCheckError(); _id=0; } }
        void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id); glCheckError(); }
        static void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); glCheckError(); }
    };

}

