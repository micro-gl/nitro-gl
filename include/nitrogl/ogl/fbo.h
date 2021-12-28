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

    class fbo {
    public:
        fbo() : _id(0), _attached_tex_id(0), _size_bytes(0) { generate(); bind(); };
        ~fbo() { del(); unbind(); }

        void attachTexture(GLuint tex_id) {
            if(_attached_tex_id==tex_id) return;
            bind();
            // attach texture to the currently bound frame buffer object
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, tex_id, 0);
            // i do not keep track of the texture, since we might want it managed by other components
            _attached_tex_id = tex_id;
            // check for completeness
//            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//                log("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
        }
        GLuint id() const { return _id; }
        void generate() { if(_id==0) glGenFramebuffers(1, &_id); bind(); }
        void del() { if(_id) { glDeleteFramebuffers(1, &_id); _attached_tex_id=_size_bytes=_id=0; } }
        void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, _id); }
        static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
        GLsizeiptr size() const { return _size_bytes / GLsizeiptr(sizeof(GLuint)); }
        GLsizeiptr size_bytes() const { return _size_bytes; }

    private:
        GLuint _id;
        GLuint _attached_tex_id;
        GLsizeiptr _size_bytes;
    };

}

