#pragma once

#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>

template<class canvas> int __get_width(canvas c) { return c.width(); }
template<class canvas> int __get_height(canvas c) { return c.height(); }
template<> int __get_width<decltype(nullptr)>(decltype(nullptr) c) { return 200; }
template<> int __get_height<decltype(nullptr)>(decltype(nullptr) c) { return 200; }

template<class canvas_type=void, class render_callback>
void example_run(canvas_type canvas, const render_callback &render) {
    SDL_Window * window;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "failure: SDL_Init(SDL_INIT_VIDEO)\n";
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE | SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    int w = __get_width(canvas);
    int h = __get_height(canvas);

    //Create window
    window = SDL_CreateWindow( "SDL ", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                w, h,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

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
    SDL_Surface *  surface = SDL_GetWindowSurface(window);

    bool quit = false;
    SDL_Event event;

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

        render(window, context, surface);
        SDL_GL_SwapWindow(window);
    }

    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

