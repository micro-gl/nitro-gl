#pragma once

#include <iostream>
#include <chrono>
//#include <glad/glad.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <SDL.h>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            //            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            //            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

template<class on_init_callback>
void example_init(const on_init_callback &on_init) {
    SDL_Window * window;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized: " << SDL_GetError();
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //Create window
    window = SDL_CreateWindow( "SDL ", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               5, 5,
//                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | 0);

    // use this for DPI scaling of window
    int wPixels, hPixels;
    int wScreen, hScreen;
    SDL_GL_GetDrawableSize(window, &wPixels, &hPixels);
    SDL_GetWindowSize(window, &wScreen, &hScreen);

    SDL_GL_SetSwapInterval(0);

    if(!window) {
        std::cout << "Window could not be created! SDL_Error: " + std::string(SDL_GetError()) << std::endl;
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

//     Setup our function pointers
//    gladLoadGLLoader(SDL_GL_GetProcAddress);

    int maj=0, min=0;
//    glGetIntegerv(GL_MAJOR_VERSION, &maj);
//    glGetIntegerv(GL_MINOR_VERSION, &min);
    const unsigned char * version = glGetString(GL_VERSION);

    std::cout << version <<" hello\n";

    on_init(window, context);
}

template<class canvas_type=void, class render_callback>
void example_run(const canvas_type & canvas, const render_callback &render) {
    auto ctx = SDL_GL_GetCurrentContext();
    SDL_Window * window = SDL_GL_GetCurrentWindow();
    bool quit = false;
    SDL_Event event;
    int w = canvas.width();
    int h = canvas.height();
    SDL_SetWindowSize(window, w, h);
    while (!quit) {
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = true;
                break;
        };
        Uint64 start = SDL_GetPerformanceCounter();
        render();
        Uint64 end = SDL_GetPerformanceCounter();
        float elapsed = float(end - start) / (float)SDL_GetPerformanceFrequency();
//        std::cout << "Current FPS: " << std::to_string(1.0f / elapsed) << std::endl;
        SDL_GL_SwapWindow(window);
    }
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

