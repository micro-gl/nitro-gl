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

#include "gl_texture.h"

namespace nitrogl {

    class fbo {
    public:
        fbo() : _id(0), _attached_tex_id(0) {};
        ~fbo() { _attached_tex_id=_id=0; unbind(); }

    public:

        void attachTexture(const gl_texture & texture) {
            if(_attached_tex_id==tex_id) return;
            bind();
            // attach texture to the currently bound frame buffer object
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture.id(), 0);
            // i do not keep track of the texture, since we might want it managed by other components
            _attached_tex_id = texture.id();
            // check for completeness
//            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//                log("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
        }
        bool wasGenerated() const { return _id; }
        void generate() { glGenFramebuffers(1, &_id); }
        GLuint id() const { return _id; }
        void del() { if(_id) { glDeleteFramebuffers(1, &_id); _attached_tex_id=_id=0; } }
        void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, _id); }
        void bind_read() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, _id); }
        void bind_draw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id); }
        static void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    private:
        GLuint _id;
        GLuint _attached_tex_id;
    };

}

