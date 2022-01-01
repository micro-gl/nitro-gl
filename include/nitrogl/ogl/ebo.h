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

    class ebo_t {
        GLuint _id;
        GLsizeiptr _size_bytes;

        void generate() { if(!_id) glGenBuffers(1, &_id); }

    public:
        ebo_t() : _id(0), _size_bytes(0) { generate(); };
        ebo_t(ebo_t && o)  noexcept : _id(o._id), _size_bytes(o._size_bytes) { o._id=0; }
        ebo_t(const ebo_t &)=default;
        ~ebo_t() { del(); unbind(); }
        ebo_t & operator=(ebo_t && o)  noexcept { _id=o._id; o._id=0; return *this;}
        ebo_t & operator=(const ebo_t &)=default;

        bool wasGenerated() const { return _id; }
        void uploadData(GLuint * array, GLsizeiptr array_size_bytes) {
            if(_id==0) return;
            bind();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, array_size_bytes, array, GL_STATIC_DRAW);
            _size_bytes = array_size_bytes;
        }
        GLuint id() const { return _id; }
        void del() { if(_id) { glDeleteBuffers(1, &_id); _size_bytes=_id=0; } }
        void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id); }
        static void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
        GLsizeiptr size() const { return _size_bytes / GLsizeiptr(sizeof(GLuint)); }
        GLsizeiptr size_bytes() const { return _size_bytes; }
    };

}

