#pragma once

#define SDL_MAIN_HANDLED
#define GL_SILENCE_DEPRECATION
#define NITROGL_USE_STD_MATH
#define NITROGL_DEBUG_MODE
#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <chrono>
//#include <glad/glad.h>
//#include <OpenGL/gl3.h>
//#include <OpenGL/gl3ext.h>
#include <GL/glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

template<class on_init_callback>
void example_init(const on_init_callback &on_init) {
    std::cout << "nitro{gl} Example init\n";
    SDL_Window * window;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << " - ERROR: SDL could not be initialized: " << SDL_GetError();
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
        std::cout << " - ERROR: Window could not be created! SDL_Error: " + std::string(SDL_GetError()) << std::endl;
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if( context == NULL ) {
        printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
        return;
    }

    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
    }
//     Setup our function pointers
//    gladLoadGLLoader(SDL_GL_GetProcAddress);

    int maj=0, min=0;
    glGetIntegerv(GL_MAJOR_VERSION, &maj);
    glGetIntegerv(GL_MINOR_VERSION, &min);
    const unsigned char * version = glGetString(GL_VERSION);
    const unsigned char * glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << " - OpenGL {GL_VERSION} String:: "<< version << std::endl;
    std::cout << " - OpenGL {GL_MAJOR_VERSION}.{GL_MINOR_VERSION}:: "<< maj << '.' << min << std::endl;
    std::cout << " - OpenGL {GL_SHADING_LANGUAGE_VERSION}:: "<< glsl_version << std::endl;

    on_init(window, context);
}

template<bool show_fps=false, class canvas_type=void, class render_callback>
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
        if(show_fps) {
            Uint64 start = SDL_GetPerformanceCounter();
            render();
            Uint64 end = SDL_GetPerformanceCounter();
            float elapsed = float(end - start) / (float)SDL_GetPerformanceFrequency();
            std::cout << "Current FPS: " << std::to_string(1.0f / elapsed) << std::endl;
        } else {
            render();
        }

        SDL_GL_SwapWindow(window);
    }
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

