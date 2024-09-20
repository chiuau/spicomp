#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL2
#

PRELOAD_FILES = testcase

IMGUI_DIR = $(HOME)/work/Papers/2025-ICRA-drone-game/code/spicomp/imgui
YAML_CPP_DIR = $(HOME)/work/Papers/2025-ICRA-drone-game/code/spicomp/yaml-cpp

SOURCES = main.cpp
SOURCES += util/debug.cpp
SOURCES += util/graph.cpp
SOURCES += util/math.cpp
SOURCES += util/name_id_map.cpp
SOURCES += util/rng.cpp
SOURCES += util/stl.cpp
SOURCES += util/string_processing.cpp
SOURCES += spicomp_gui.cpp
SOURCES += spicomp_setting.cpp
SOURCES += spicomp_simulator.cpp
SOURCES += sdl_gui_context.cpp
SOURCES += shared.cpp

SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

SOURCES += $(YAML_CPP_DIR)/src/binary.cpp $(YAML_CPP_DIR)/src/convert.cpp $(YAML_CPP_DIR)/src/depthguard.cpp $(YAML_CPP_DIR)/src/directives.cpp $(YAML_CPP_DIR)/src/emit.cpp $(YAML_CPP_DIR)/src/emitfromevents.cpp $(YAML_CPP_DIR)/src/emitter.cpp $(YAML_CPP_DIR)/src/emitterstate.cpp $(YAML_CPP_DIR)/src/emitterutils.cpp $(YAML_CPP_DIR)/src/exceptions.cpp $(YAML_CPP_DIR)/src/exp.cpp $(YAML_CPP_DIR)/src/memory.cpp $(YAML_CPP_DIR)/src/node.cpp $(YAML_CPP_DIR)/src/node_data.cpp $(YAML_CPP_DIR)/src/nodebuilder.cpp $(YAML_CPP_DIR)/src/nodeevents.cpp $(YAML_CPP_DIR)/src/null.cpp $(YAML_CPP_DIR)/src/ostream_wrapper.cpp $(YAML_CPP_DIR)/src/parse.cpp $(YAML_CPP_DIR)/src/parser.cpp $(YAML_CPP_DIR)/src/regex_yaml.cpp $(YAML_CPP_DIR)/src/scanner.cpp $(YAML_CPP_DIR)/src/scanscalar.cpp $(YAML_CPP_DIR)/src/scantag.cpp $(YAML_CPP_DIR)/src/scantoken.cpp $(YAML_CPP_DIR)/src/simplekey.cpp $(YAML_CPP_DIR)/src/singledocparser.cpp $(YAML_CPP_DIR)/src/stream.cpp $(YAML_CPP_DIR)/src/tag.cpp $(YAML_CPP_DIR)/src/contrib/graphbuilder.cpp $(YAML_CPP_DIR)/src/contrib/graphbuilderadapter.cpp


### Shared flags ###
CXX_FLAGS = -std=c++2a -O3 -Wall -Wformat -I. -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(YAML_CPP_DIR)/include
LINK_FLAGS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(MAKECMDGOALS),web)
  TARGET_PLATFORM = web

  CXX = em++

  EMS += -fwasm-exceptions -s USE_SDL=2 -s USE_SDL_GFX=2 
  # EMS += -fexceptions -s USE_SDL=2 -s USE_SDL_GFX=2 -S USE_SDL_TTF=2

  USE_FILE_SYSTEM ?= 1
  ifeq ($(USE_FILE_SYSTEM), 0)
    LINK_FLAGS += -s NO_FILESYSTEM=1
    CXX_FLAGS += -DIMGUI_DISABLE_FILE_FUNCTIONS
  else
    LINK_FLAGS += --no-heap-copy --preload-file fonts@/fonts
  endif

  CXX_FLAGS += $(EMS)
  LINK_FLAGS += --shell-file shell_minimal.html --preload-file $(PRELOAD_FILES) $(EMS) -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1

  OUT_DIR = bin_$(TARGET_PLATFORM)
  OBJ_DIR = obj_$(TARGET_PLATFORM)
  OUT_EXE = $(OUT_DIR)/index.html

  OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
else
  UNAME_S := $(shell uname -s)

  CXX = clang++

  ifeq ($(UNAME_S), Linux) #LINUX
    TARGET_PLATFORM = linux
    LINK_FLAGS += -lGL -ldl `sdl2-config --libs` -lSDL2_gfx
    CXX_FLAGS += `sdl2-config --cflags`
  endif

  ifeq ($(UNAME_S), Darwin) #APPLE
    TARGET_PLATFORM = mac
    LINK_FLAGS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
    LINK_FLAGS += -lSDL2_gfx
    CXX_FLAGS += `sdl2-config --cflags`
    CXX_FLAGS += -I/usr/local/include
  endif

  ifeq ($(OS), Windows_NT)
    TARGET_PLATFORM = win
    LINK_FLAGS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`
    CXX_FLAGS += `pkg-config --cflags sdl2`
  endif

  OUT_DIR = bin_$(TARGET_PLATFORM)
  OBJ_DIR = obj_$(TARGET_PLATFORM)
  OUT_EXE = $(OUT_DIR)/main

  OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

all: $(OUT_EXE)
	@echo Build complete for $(TARGET_PLATFORM)

web: all
	sync.sh

$(OUT_EXE): $(OBJ_DIR) $(OBJS) $(OUT_DIR)
	$(CXX) $(LINK_FLAGS) -o $@ $(OBJS)

$(OUT_DIR):
	mkdir $@

$(OBJ_DIR):
	mkdir $@

$(OBJ_DIR)/%.o:util/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(YAML_CPP_DIR)/src/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(YAML_CPP_DIR)/src/contrib/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS) obj_web bin_web obj_linux bin_linux obj_mac bin_mac obj_win bin_win


.PHONY: all clean web

