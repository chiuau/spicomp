#include <cmath>

#include "util/string_processing.h"
#include "util/math.h"

#include "spicomp_gui.h"


void SpicompGui::reset() {
  make_bg_texture();

  window_size_x = setting.getWindowSizeX();
  window_size_y = setting.getWindowSizeY();
  origin_x = window_size_x / 2.0;
  origin_y = 3.0 * window_size_y / 4.0;
  delta_x = - M_PI / 12.0;
  delta_y = M_PI / 12.0;
  length_scale = 1.0;
  length_scale_step = 0.1;

  // z_view_len_scale = 400.0;
  z_view_len_scale = 600.0;
  drone_radius = 5.0;

  is_dragging = false;
  is_left_mouse_btn_down = false;
  is_right_mouse_btn_down = false;
  last_mouse_x = 0;
  last_mouse_y = 0;

  simulator.reset();
}


bool SpicompGui::run_one_step() {
  ImGui_ImplSDLRenderer2_NewFrame();

  Uint64 frame_start_time = SDL_GetPerformanceCounter();  // see https://thenumbat.github.io/cpp-course/sdl2/08/08.html

  // ------ handle user inputs ------
  SDL_Event event;
  while(SDL_PollEvent(&event)) {   // process all events in the queue before drawing the next frame
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      return false;
    }
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == gui_context.getWindowID()) {
      return false;
    }

    if (!((ImGui::GetIO()).WantCaptureMouse || (ImGui::GetIO()).WantCaptureKeyboard)) {
      if (event.type == SDL_KEYDOWN) {
        Uint64 ticksNow = SDL_GetTicks64();
        if (ticksNow > ticksForNextKeyDown) {
          ticksForNextKeyDown = ticksNow + 10;   // Throttle keydown events for 10ms.
          switch (event.key.keysym.sym) {
            case SDLK_q:
              return false;  // terminate and exit
            case SDLK_r:
              reset();
              break;
            case SDLK_SPACE:
              is_sim_paused = !is_sim_paused;
              break;
          }
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (!is_dragging) {
          is_dragging = true;
          last_mouse_x = event.button.x;
          last_mouse_y = event.button.y;
          if (event.button.button == SDL_BUTTON_LEFT) {
            is_left_mouse_btn_down = true;
            last_delta_x = delta_x;
            last_delta_y = delta_y;
          } else if (event.button.button == SDL_BUTTON_RIGHT) {
            is_right_mouse_btn_down = true;
            last_origin_x = origin_x;
            last_origin_y = origin_y;
          }
        }
      } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (is_dragging) {
          is_dragging = false;
          is_left_mouse_btn_down = false;
          is_right_mouse_btn_down = false;
          last_mouse_x = 0;
          last_mouse_y = 0;
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        if (is_dragging) {
          int x = event.button.x;
          int y = event.button.y;
          int dx = x - last_mouse_x;
          int dy = y - last_mouse_y;

          if (is_left_mouse_btn_down) {
            delta_x = last_delta_x + dx * M_PI / 360.0;
            delta_y = last_delta_y + dy * M_PI / 360.0;
          } else if (is_right_mouse_btn_down) {
            origin_x = last_origin_x + dx;
            origin_y = last_origin_y + dy;
          }
        }
      } else if (event.type == SDL_MOUSEWHEEL) {
        if(event.wheel.y > 0) {   // scroll up
          length_scale += length_scale_step;
        } else if(event.wheel.y < 0) {  // scroll down
          length_scale -= length_scale_step;
          if (length_scale < length_scale_step) length_scale = length_scale_step;
        }
      }

    }
  }

  // ------ drawing ------

  // Prepare to draw a frame
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();

  // Drawing the background
  draw_sdl_background();
  SDL_RenderCopy(renderer, bg_texture, nullptr, nullptr);

  // Prepare the ImGUI frame
  ImGui::NewFrame();
  ImGui::PushFont(gui_context.getFont("DroidSans", 18));

  // Make new ImGui window
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_size_x), static_cast<float>(window_size_y)));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1,1));
  ImGui::Begin("W1", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
  // ImGui::Begin("?", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);  // with background

  // Draw on the ImGui window

  draw_micro_frame(simulator.getCurrentMicroFrame());
  // draw_time_step();
  draw_controller();

  // Wrap up the ImGUI window
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

  // Wrap up the ImGUI frame
  ImGui::PopFont();
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(renderer);   // rendering

  if (!is_sim_paused) {
    simulator.nextStep();
    if (simulator.isStopped()) {
      is_sim_paused = true;
    }
  }

  // frame rate control
  Uint64 frame_end_time = SDL_GetPerformanceCounter();
  auto max_frame_duration = simulator.getTimeStepDuration() / playback_speed_multipler;
  auto elapsed_in_sec = static_cast<double>((frame_end_time - frame_start_time)) / static_cast<double>(SDL_GetPerformanceFrequency());
  if (elapsed_in_sec < max_frame_duration - 0.0001) {
    SDL_Delay(static_cast<Uint32>((max_frame_duration - elapsed_in_sec) * 1000.0));  // in milliseconds
    is_frame_delayed = false;
  } else {  // else just run as quickly as possible
    // std::cout << "Warning: frame delayed by " << (elapsed_in_sec - max_frame_duration) * 1000.0 << "ms" << std::endl;
    is_frame_delayed = true;
  }

  return true;
}


void SpicompGui::make_bg_texture() {
  if (bg_texture == nullptr) {
    bg_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                   setting.getWindowSizeX(),
                                   setting.getWindowSizeY());
    SDL_SetRenderTarget(renderer, bg_texture);
    draw_sdl_background();
    // draw_sdl_grid();
    SDL_SetRenderTarget(renderer, nullptr);
  }
}

void SpicompGui::draw_sdl_background() {
  set_draw_color(COLOR_GREY);
  SDL_RenderClear(renderer);
}


void SpicompGui::draw_string(int x, int y, const std::string& text, int font_size, const ImColor& color) {
  ImGui::PushFont(gui_context.getFont("Arial", font_size));
  ImGui::GetWindowDrawList()->AddText({static_cast<float>(x), static_cast<float>(y)}, color, text.c_str());
  ImGui::PopFont();
}


void SpicompGui::draw_centered_string(int cx, int cy, const std::string& text, int font_size, const ImColor& color) {
  ImGui::PushFont(gui_context.getFont("Arial", font_size));
  auto [width, height] = ImGui::CalcTextSize(text.c_str());
  ImGui::GetWindowDrawList()->AddText({static_cast<float>(cx) - width/2, static_cast<float>(cy) - height/2}, color, text.c_str());
  ImGui::PopFont();
}


void SpicompGui::draw_micro_frame(const Frame& frame) const {
  std::vector<DroneDrawData> drone_draw_data_list;

  double cos_delta_x = std::cos(delta_x);
  double sin_delta_x = std::sin(delta_x);
  double cos_delta_y = std::cos(delta_y);
  double sin_delta_y = std::sin(delta_y);

  for(auto& pixel : frame.getPixels()) {

    // swap the y-axis and z-axis and invert the z-axis

    double rx = pixel.x;
    double ry = -pixel.z;
    double rz = pixel.y;

    double screen_x = rx * length_scale;
    double screen_y = ry * length_scale;
    double screen_z = rz * length_scale;

    double new_screen_z = screen_z * cos_delta_x - screen_x * sin_delta_x;
    double new_screen_x = screen_z * sin_delta_x + screen_x * cos_delta_x;
    screen_z = new_screen_z;
    screen_x = new_screen_x;

    new_screen_z = screen_z * cos_delta_y - screen_y * sin_delta_y;
    double new_screen_y = screen_z * sin_delta_y + screen_y * cos_delta_y;
    screen_z = new_screen_z;
    screen_y = new_screen_y;

    screen_x += origin_x;
    screen_y += origin_y;

    double scaled_radius = (drone_radius * length_scale) * std::exp(screen_z / z_view_len_scale);

    drone_draw_data_list.emplace_back(screen_x, screen_y, screen_z, scaled_radius, pixel.red, pixel.green, pixel.blue);
  }

  // sort by z axis
  std::sort(drone_draw_data_list.begin(), drone_draw_data_list.end(), drone_draw_data_comparator);

  // draw pixels
  for(auto& drone_draw_data : drone_draw_data_list) {
    draw_pixel(drone_draw_data);
  }

}


void SpicompGui::draw_pixel(const DroneDrawData& drone_draw_data) const {
  ImGui::GetWindowDrawList()->AddCircleFilled( { static_cast<float>(drone_draw_data.screen_x), static_cast<float>(drone_draw_data.screen_y) }, static_cast<float>(drone_draw_data.scaled_radius), drone_draw_data.color);
  ImGui::GetWindowDrawList()->AddCircle( { static_cast<float>(drone_draw_data.screen_x), static_cast<float>(drone_draw_data.screen_y) }, static_cast<float>(drone_draw_data.scaled_radius), IMGUI_COLOR_BLACK);
}


void SpicompGui::draw_time_step() {
  int step_count = simulator.getSimStepCount();
  double step_time = simulator.getTimeStepDuration();
  std::string str = cpp11_string_format("%.2f", step_count * step_time) + "s";
  draw_string(10, 10, str, 32, IMGUI_COLOR_BLACK);
}


void SpicompGui::draw_controller() {

  // Make new ImGui window
  ImGui::SetNextWindowPos(ImVec2(0, static_cast<float>(window_size_y - 30.0)));
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_size_y), 0.0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.0, 3.0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1, 1));
  ImGui::Begin("W2_PLAYBACK_SPEED", nullptr, ImGuiWindowFlags_NoDecoration);

  // Align to center (see https://github.com/ocornut/imgui/discussions/3862)
  ImGuiStyle &style = ImGui::GetStyle();
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - ImGui::CalcTextSize("Playback Speed: ").x - 450.0f - style.ItemSpacing.x * 3) * 0.5f;
  if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

  // Draw on the ImGui window
  ImGui::Text("Playback Speed: ");
  ImGui::SameLine();
  ImGui::PushItemWidth(250);
  if (is_frame_delayed) {
    ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(IMGUI_COLOR_RED));
  }
  ImGui::SliderFloat("##SLIDER", &playback_speed_multipler, 0.05f, 3.0f, "%.2fx", ImGuiSliderFlags_Logarithmic);
  if (is_frame_delayed) {
    ImGui::PopStyleColor();
  }
  ImGui::PopItemWidth();

  ImGui::SameLine();
  if (ImGui::Button((is_sim_paused ? "Continue (_)##PAUSE" : "Pause (_)##PAUSE"), ImVec2(100, 0))) {
    is_sim_paused = !is_sim_paused;
  }

  ImGui::SameLine();
  if (ImGui::Button("Reset (r)", ImVec2(100, 0))) {
    simulator.reset();
  }

  // Wrap up the ImGui window
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}


