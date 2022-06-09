#define GL_SILENCE_DEPRECATION
#define SUPPORTS_VAO
#define MICROGL_USE_STD_MATH
#define NITROGL_ENABLE_THROW

#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/test_sampler.h>
#include <nitrogl/samplers/mix_sampler.h>
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/samplers/shapes/circle_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

//using path_t = path<std::vector>;
using path_t = nitrogl::path<dynamic_array>;

float t = 0.0f;

path_t path_star() {
    path_t path{};
    path.lineTo({150, 150})
    .quadraticCurveTo({450, 0}, {450, 150})
    .lineTo({200,450})
    .lineTo({300,50})
    .lineTo({400,450})
    .closePath();
    return path;
}

path_t path_star_2() {
    path_t path{};

    path.linesTo2(150, 150, 450,150, 200,450, 300,50, 400,450)
        .moveTo({150/2, 150/2})
        .linesTo2(450/2,150/2, 200/2,450/2, 300/2,50/2, 400/2,450/2)
        .moveTo({150/10, 150/10})
        .linesTo2(450/10,150/10, 200/10,450/10, 300/10,50/10, 400/10,450/10)
        .rect(50, 50, 250, 250)
        .rect(50, 250, 550, 50, false)
        .rect(50, 450, 50, 50, false)
        .closePath();

    return path;
}

path_t path_arc_animation() {
    path_t path{};
    int div=32; //4
    path.arc({200,200}, 100,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div).closePath();

    path.arc({250,200}, 50,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             true, div).closePath();
    t+=0.033f;
    //    t=120.539963f;//819999992f;//-0.01f;
    ////t=26.0399914;
    path.moveTo({150,150});
    path.arc({150+0,150}, 50+t-0,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div);//.closePath();

    return path;
}

path_t path_rects() {
    path_t path{};
    path.rect(50, 50, 250, 250, false)
        .rect(50, 250, 550, 50, true)
        .rect(50, 450, 50, 50, true);
    return path;
}

path_t path_test() {
    path_t path{};
    using v = microtess::vec2<float>;
    int div=32;
    //    t+=0.01;
    t=137.999039f;
    path.linesTo2(100,100, 300,100, 300,100, 300, 300, 100,300);
    v start{22.0f, 150.0f - 0.002323204};
    path.moveTo(start);
    path.linesTo3(start, v{300,120.002323204-t}, v{300, 300}, v{100,300});
    path.moveTo({200, 200});
    path.lineTo({300,10});

    return path;
}

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(100,200);
        glCheckError();
        canvas canva(600,600);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true), true);
        color_sampler sampler_color(1.0,0.0,0.0,1.0/2);

        // paths
        auto path = path_star_2();
//        auto path = path_star();
//        auto path = path_rects();
//        auto path = path_arc_animation();
//        auto path = path_test();

        auto render = [&]() {
            // auto path = path_arc_animation();
            static float t =0;
            t+=0.001f;
            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawPathFill(
                    tex_sampler_3,
                    path,
                    microtess::fill_rule::even_odd,
                    microtess::tess_quality::prettier_with_extra_vertices,
                    //            tessellation::tess_quality::better,
                    mat3f::identity(),
                    mat3f::rotation(math::deg_to_rad(t), 0.5f, 0.5f)
                    );
            glCheckError();
        };

        example_run<true>(canva, render);
    };

    example_init(on_init);
}

