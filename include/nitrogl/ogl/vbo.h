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

    class vbo {
    public:
        vbo() : _id(0), _size_bytes(0) { glGenBuffers(1, &_id); };
        ~vbo() { del(); unbind(); }

        void uploadData(const void * array, GLsizeiptr array_size_bytes) {
            if(_id==0) return;
            bind();
            glBufferData(GL_ARRAY_BUFFER, array_size_bytes, array, GL_STATIC_DRAW);
            _size_bytes = array_size_bytes;
        }
        void uploadSubData(GLintptr offset, const void *array, GLuint size_bytes) const {
            if(_id==0) return;
            bind();
            glBufferSubData(GL_ARRAY_BUFFER, offset, size_bytes, array);
        }
        GLuint id() const { return _id; }
        void del() { if(_id) { glDeleteBuffers(1, &_id); _size_bytes=_id=0; } }
        void bind() const { glBindBuffer(GL_ARRAY_BUFFER, _id); }
        static void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
        GLsizeiptr size() const { return _size_bytes / GLsizeiptr(sizeof(GLuint)); }
        GLsizeiptr size_bytes() const { return _size_bytes; }

    private:
        GLuint _id;
        GLsizeiptr _size_bytes;
    };

}

