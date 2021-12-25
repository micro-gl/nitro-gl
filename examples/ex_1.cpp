#include "src/Resources.h"
#include "src/example.h"
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

struct CC {
    int width() { return 600; }
    int height() { return 600; }
};
int main() {
    const auto render = [](void *, void *, void *) {
        std::cout << "hello\n";
    };

    example_run(nullptr, render);
}

