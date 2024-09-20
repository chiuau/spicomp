file(GLOB IMGUI_CPP *.cpp)
list(APPEND IMGUI_CPP backends/imgui_impl_sdl2.cpp backends/imgui_impl_sdlrenderer2.cpp)

add_library(imgui ${IMGUI_CPP})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/backends)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/sdl2)
find_package(SDL2 REQUIRED)
target_link_libraries(imgui SDL2::Main)

