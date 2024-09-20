cmake_minimum_required(VERSION 3.22)

project(spicomp VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdl2_cmake_files)

find_package(SDL2 REQUIRED)
find_package(SDL2_gfx REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_subdirectory(imgui)
add_subdirectory(yaml-cpp)

include_directories(.)
include_directories(yaml-cpp/include)


file(GLOB YAML_CPP_SRC "yaml-cpp/src/*.h" "yaml-cpp/src/*.cpp")
# set(YAML_CPP_SRC)

add_executable(${PROJECT_NAME} main.cpp
        ${YAML_CPP_SRC}
        shared.h shared.cpp
        util/graph.h util/graph.cpp
        util/debug.cpp util/debug.h
        util/string_processing.h util/string_processing.cpp
        util/math.cpp util/math.h
        util/stl.cpp util/stl.h
        util/rng.cpp util/rng.h
        util/name_id_map.cpp util/name_id_map.h
        util/expected.h
        sdl_gui_context.cpp sdl_gui_context.h
        spicomp_gui.cpp
        spicomp_gui.h
        spicomp_simulator.cpp
        spicomp_simulator.h
        spicomp_setting.cpp
        spicomp_setting.h
        spicomp_simulator.h
        spicomp_simulator.h
        spicomp_simulator.h)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::Main SDL2::GFX imgui)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(${PROJECT_NAME} PRIVATE
#        "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

set(link_src "${CMAKE_SOURCE_DIR}/fonts")
set(link_dst "${CMAKE_CURRENT_BINARY_DIR}/fonts")

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${link_src} ${link_dst}
  DEPENDS ${link_dst}
  COMMENT "symbolic link ${link_src} => ${link_dst}"
)
