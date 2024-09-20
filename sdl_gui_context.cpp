#include "sdl_gui_context.h"


SdlGuiContext::SdlGuiContext(int window_size_x, int window_size_y) {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    SDL_Log("Error: %s.", SDL_GetError());
    throw std::runtime_error("Error when calling SDL_Init()");
  }

  // Setup window
  // SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  // SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("Demo",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            window_size_x, window_size_y,
                            window_flags);
  if (window == nullptr) {
    SDL_Log("Error in SDL_CreateWindow().");
    throw std::runtime_error("Error when calling SDL_CreateWindow()");
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_element_css_size("canvas", window_size_x, window_size_y);
#endif

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    SDL_Log("Error in SDL_Renderer().");
    throw std::runtime_error("Error when calling SDL_Renderer()");
  }

  // see https://github.com/ocornut/imgui/issues/4768
  int gl_w;
  int gl_h;
  SDL_GL_GetDrawableSize(window, &gl_w, &gl_h);
  int sdl_w;
  int sdl_h;
  SDL_GetWindowSize(window, &sdl_w, &sdl_h);

  SDL_RenderSetScale(renderer,
                     static_cast<float>(gl_w) / static_cast<float>(sdl_w),
                     static_cast<float>(gl_h) / static_cast<float>(sdl_h));

  // SDL_RendererInfo info;
  // SDL_GetRendererInfo(renderer, &info);
  // SDL_Log("Current SDL_Renderer: %s", info.name);
  // std::cout << "emscripten_get_device_pixel_ratio() = " << emscripten_get_device_pixel_ratio() << std::endl;
  // SDL_RenderSetLogicalSize(renderer, 2560, 1440);
  // int w, h;
  // SDL_RenderGetLogicalSize(renderer, &w, &h);
  // std::cout << w << " " << h << std::endl;


  // float diagDPI = -1;
  // float horiDPI = -1;
  // float vertDPI = -1;
  // int dpiReturn = SDL_GetDisplayDPI (0, &diagDPI, &horiDPI, &vertDPI);
  // std::cout << "GetDisplayDPI() returned " << dpiReturn << std::endl;
  // std::cout << diagDPI << " " << horiDPI << " " << vertDPI << " " << std::endl;

  // int gl_w;
  // int gl_h;
  // SDL_GL_GetDrawableSize (window, &gl_w, &gl_h);
  // std::cout << "GL_W: " << gl_w << std::endl << "GL_H: " << gl_h << std::endl;

  // int sdl_w;
  // int sdl_h;
  // SDL_GetWindowSize (window, &sdl_w, &sdl_h);
  // std::cout << "SDL_W: " << sdl_w << std::endl << "SDL_H: " << sdl_h << std::endl;


  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); // (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // For an Emscripten build we are disabling file-system access,
  // so let's not attempt to do a fopen() of the imgui.ini file.
  // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
#ifdef __EMSCRIPTEN__
  io.IniFilename = NULL;
#endif

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  // Load Fonts
  font_db["DroidSans"];
  font_db["Arial"];
  // font_db["Cousine-Regular"];
  // font_db["Roboto-Medium"];

  ImFontConfig font_config;
  // font_config.OversampleH = 2;
  // font_config.OversampleV = 1;
  // font_config.GlyphExtraSpacing.x = 1.0f;

  for(auto& [font_name, vec] : font_db) {
    for(int pixel_size = 4; pixel_size <= 24; pixel_size+=2) {
      vec[pixel_size] = io.Fonts->AddFontFromFileTTF(("fonts/" + font_name + ".ttf").c_str(), static_cast<float>(pixel_size), &font_config);
    }
    for(int pixel_size = 28; pixel_size <= 48; pixel_size+=4) {
      vec[pixel_size] = io.Fonts->AddFontFromFileTTF(("fonts/" + font_name + ".ttf").c_str(), static_cast<float>(pixel_size), &font_config);
    }
  }

//  io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 18.0);
//  io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 18.0);
//  io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 18.0);
//  io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 18.0);
  // io.Fonts->AddFontDefault();
}


SdlGuiContext::~SdlGuiContext() {
  // Cleanup
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

ImFont* SdlGuiContext::getFont(const std::string& font_name, int pixel_size) {
  return font_db.at(font_name).at(pixel_size);

//// cannot load font after NewFrame()
//  if (!font_db.contains(font_name))  return nullptr;
//  try {
//    return font_db.at(font_name).at(pixel_size);
//  } catch(const std::out_of_range& e) {
//    ImFontConfig font_config;
//    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(("fonts/" + font_name + ".ttf").c_str(), static_cast<float>(pixel_size), &font_config);
//    font_db[font_name][pixel_size] = font;
//    return font;
//  }
}
