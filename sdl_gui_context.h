#ifndef MULTIPLAN_SDL_GUI_CONTEXT_H
#define MULTIPLAN_SDL_GUI_CONTEXT_H

#include <iostream>
#include <string>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#else
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
// #include <SDL_ttf.h>
#endif

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// #include "shared.h"

class SdlGuiContext {

  SDL_Window *window;
  SDL_Renderer *renderer;

  std::unordered_map<std::string,std::unordered_map<int,ImFont*>> font_db;

public:

  SdlGuiContext(int window_size_x, int window_size_y);

  ~SdlGuiContext();

  SDL_Window* getSdlWindow() { return window; }

  SDL_Renderer* getSdlRenderer() { return renderer; }

  Uint32 getWindowID() const { return SDL_GetWindowID(window); }

  ImFont* getFont(const std::string& str, int size);

};




#endif //MULTIPLAN_SDL_GUI_CONTEXT_H
