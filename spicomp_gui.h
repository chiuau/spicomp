#ifndef SPICOMP_SPICOMP_GUI_H
#define SPICOMP_SPICOMP_GUI_H

#include "util/rng.h"

#include "shared.h"
#include "sdl_gui_context.h"

#include "spicomp_setting.h"
#include "spicomp_simulator.h"


class SpicompGui {

  // Colors for SDL
  const Uint8 COLOR_BLACK[3]{0, 0, 0};
  const Uint8 COLOR_GREY[3]{100, 100, 100};
  const Uint8 COLOR_BROWN[3]{165, 42, 42};
  const Uint8 COLOR_ORANGE[3]{255, 127, 0};
  const Uint8 COLOR_WHEAT[3]{245, 222, 179};
  const Uint8 COLOR_WHITE[3]{255, 255, 255};

  // Colors for ImGUI
  const ImColor IMGUI_COLOR_BLACK{0, 0, 0};
  const ImColor IMGUI_COLOR_BLACK50{50, 50, 50};
  const ImColor IMGUI_COLOR_GREY{100, 100, 100};
  const ImColor IMGUI_COLOR_LIGHT_GREY{150, 150, 150};
  const ImColor IMGUI_COLOR_WHITE{255, 255, 255};
  const ImColor IMGUI_COLOR_RED{255, 0, 0};
  const ImColor IMGUI_COLOR_GREEN{0, 255, 0};
  const ImColor IMGUI_COLOR_DARK_GREEN{0, 200, 0};
  const ImColor IMGUI_COLOR_BLUE{0, 0, 255};
  const ImColor IMGUI_COLOR_YELLOW{255, 255, 0};
  const ImColor IMGUI_COLOR_MAGENTA{255, 0, 255};
  const ImColor IMGUI_COLOR_LIGHT_MAGENTA{255, 50, 255};
  const ImColor IMGUI_COLOR_CYAN{0, 255, 255};
  const ImColor IMGUI_COLOR_BROWN{150, 75, 0};
  const ImColor IMGUI_COLOR_WHEAT{245, 222, 179};


  struct DroneDrawData {
    double screen_x;
    double screen_y;
    double screen_z;
    double scaled_radius;
    ImColor color;

    DroneDrawData(double screen_x, double screen_y, double screen_z, double scaled_radius, int red, int green, int blue) :
        screen_x(screen_x), screen_y(screen_y), screen_z(screen_z), scaled_radius(scaled_radius), color(red, green, blue)
    {
      // do nothing
    }
  };

  struct DroneDrawDataComparator {
    bool operator()(const DroneDrawData& x1, const DroneDrawData& x2) {
      return x1.screen_z < x2.screen_z;
    }
  } drone_draw_data_comparator;

  const SpicompSetting& setting;
  SpicompSimulator& simulator;

  SdlGuiContext gui_context;
  SDL_Renderer* renderer;
  SDL_Texture* bg_texture = nullptr;

  Uint64 ticksForNextKeyDown = 0;
  bool is_sim_paused = false;
  float playback_speed_multipler = 0.3;
  bool is_frame_delayed = false;

  bool is_dragging;
  bool is_left_mouse_btn_down;
  bool is_right_mouse_btn_down;
  int last_mouse_x;
  int last_mouse_y;
  double last_delta_x;
  double last_delta_y;
  double last_origin_x;
  double last_origin_y;

  double window_size_x, window_size_y;
  double origin_x, origin_y;
  double delta_x, delta_y;  // rotate around z-axis and x-axis, respectively, using the right-hand rule.
  double length_scale;
  double length_scale_step;
  double z_view_len_scale;

  double drone_radius;

public:

  SpicompGui(const SpicompSetting& setting, SpicompSimulator& simulator) :
      setting(setting), simulator(simulator),
      gui_context(setting.getWindowSizeX(), setting.getWindowSizeY()),
      renderer(gui_context.getSdlRenderer())
  {
    reset();
  }

  bool run_one_step();

private:

  void reset();

  void make_bg_texture();

  void set_draw_color(const Uint8 color[]) { SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], SDL_ALPHA_OPAQUE); }

  void draw_sdl_background();

  void draw_string(int x, int y, const std::string& text, int font_size, const ImColor& color);

  void draw_centered_string(int x, int y, const std::string& text, int font_size, const ImColor& color);

  void draw_time_step();

  void draw_micro_frame(const Frame& frame) const;

  void draw_pixel(const DroneDrawData& drone_draw_data) const;

  void draw_controller();

};




#endif //SPICOMP_SPICOMP_GUI_H
