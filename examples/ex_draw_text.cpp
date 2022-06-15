#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/canvas.h>

using namespace nitrogl;

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(500,500);

        //        font = Resources::loadFont<Bitmap32_ARRAY>("minecraft-20");
        //    font = Resources::loadFont<Bitmap32_ARRAY>("digital_7-20");
        auto font = Resources::loadFont("assets/fonts/roboto-thin-28");
        //    font = Resources::loadFont<Bitmap32_ARRAY>("roboto-thin-14");
        //    font = Resources::loadFont<Bitmap32_ARRAY>("mont-med-16");
        //    font = Resources::loadFont<Bitmap32_ARRAY>("test");

        text::text_format format;
        // configure font
        // font.offsetX=5;
        // font.offsetY=5;
        font.padding=5;
        //font.lineHeight=19;
        format.letterSpacing=0;
        format.leading=5;
        //format.fontSize=40;
        //format.horizontalAlign=text::hAlign::right;
        format.horizontalAlign=text::hAlign::left;
        //format.horizontalAlign=text::hAlign::center;
        //format.verticalAlign=text::vAlign::center;
        //format.verticalAlign=text::vAlign::bottom;
        format.verticalAlign=text::vAlign::top;
        //format.wordWrap=text::wordWrap ::normal;
        format.wordWrap=text::wordWrap::break_word;
        const char * text = "Welcome to micro{gl} Welcome to micro{gl} Welcome to "
                            "micro{gl} Welcome to micro{gl} Welcome to micro{gl} ";

        glCheckError();
        canvas canva(500,500);
//        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
//        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
//        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true), true);
//        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        auto render = [&]() {
            static float t = 0.0f;
            t+=0.005;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawText(text,
                           font,
                           {1.0f, 1.0f, 1.0f, 1.0f},
                           format,
                           0, 0, 300, 300);
            glCheckError();
        };

        example_run<true>(canva, render);
    };

    example_init(on_init);
}

