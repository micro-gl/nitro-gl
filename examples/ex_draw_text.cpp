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

        //auto font = Resources::loadFont("assets/fonts/minecraft-20");
        //auto font = Resources::loadFont("assets/fonts/digital_7-20");
        //auto font = Resources::loadFont("assets/fonts/roboto-thin-28");
        //auto font = Resources::loadFont("assets/fonts/roboto-thin-14");
        //auto font = Resources::loadFont("assets/fonts/mont-med-16");
        auto font = Resources::loadFont("assets/fonts/test");

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
        const char * text = "Welcome to nitro{gl} Welcome to nitro{gl} Welcome to "
                            "nitro{gl} Welcome to nitro{gl} Welcome to nitro{gl} ";

        glCheckError();
        canvas canva(500,500);

        auto render = [&]() {
            static float t = 0.0f;
            t+=0.005;
            canva.clear(0.286f, 0.329f, 0.396f, 1.0f);
            canva.drawText(text,
                           font,
                           {1.0f, 1.0f, 1.0f, 1.0f},
                           format,
                           0, 0, 400, 400);
            glCheckError();
        };

        example_run<true>(canva, render);
    };

    example_init(on_init);
}
