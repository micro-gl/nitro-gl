#include "src/example.h"
#include "src/Resources.h"
#include <nitrogl/samplers/texture_sampler.h>
#include <nitrogl/canvas.h>

using namespace nitrogl;

vec2f * curve_cubic_1() {
    static vec2f data[4] = {{5, 480 - 5},
                            {640/8, 480/4},
                            {640/3, 480/2},
                            {640/2, 480/2}};
    return data;
}

int main() {

    auto on_init = [](SDL_Window *, void *) {
        auto tex = gl_texture(100,200);

        canvas canva(600,600);
        auto tex_sampler_1 = texture_sampler(Resources::loadTexture("assets/images/test.png", true));
        auto tex_sampler_2 = texture_sampler(Resources::loadTexture("assets/images/test.png", false));
        auto tex_sampler_3 = texture_sampler(Resources::loadTexture("assets/images/uv_256.png", true), true);
        color_sampler color_black(0.0,0.0,0.0,1.0);
        color_sampler color_red(1.0,0.0,0.0,1.0);

        vec2f * curve = curve_cubic_1();
        int degree = 3;
        auto render = [&]() {
            static float t = 0.0f;

            dynamic_array<vec2f> output{};

            using curve_divider = microtess::curve_divider<float, decltype(output)>;
            auto algo = microtess::CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium;
            auto type = degree==3 ? microtess::CurveType::Cubic : microtess::CurveType::Quadratic;

            t = 0.1f;
            curve[1].y -= t;
            curve_divider::compute(curve, output, algo, type);

            canva.clear(1.0, 1.0, 1.0, 1.0);
            canva.drawLines(color_black,
                            output.data(),
                            output.size(),
                            false);

            // Draw points as circles
            for (auto & p : output)
                canva.drawCircle(color_red, color_black,
                                 p.x, p.y, 5.0f, 1.0f);

        };

        example_run<false>(canva, render);
    };

    example_init(on_init);
}

