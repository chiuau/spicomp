#include <iostream>
#include <string>
#include <memory>
#include <chrono>

#include "shared.h"
#include "util/string_processing.h"
#include "util/debug.h"

#include "spicomp_simulator.h"
#include "spicomp_gui.h"


// ********************************************************************************
//   The Command Line Argument
// ********************************************************************************


class FoctlCommandLineArgument : public CommandLineArgument {
  bool is_help = false;
  bool is_verbose = false;
  bool is_show_gui = false;
  std::string setting_filename;

public:

  FoctlCommandLineArgument(int argc, char *argv[]) :
      CommandLineArgument(argc, argv, { { "-h", 0 }, { "-v", 0 }, { "-g", 0 }, { "", -1 } })
  {
    if (token_partition.contains("-h")) {
      is_help = true;
      return;
    }
    is_verbose = token_partition.contains("-v");
    is_show_gui = token_partition.contains("-g");
    if (!token_partition[""].empty()) {
      setting_filename = token_partition[""][0];
    }
  }

  bool isHelp() const { return is_help; }
  bool isVerbose() const { return is_verbose; }
  bool isExpr() const { return !is_show_gui; }
  bool isShowGUI() const { return is_show_gui; }
  bool isSettingFilenameExist() const { return !setting_filename.empty(); }
  const std::string& getSettingFilename() const { return setting_filename; }

  void printHelp() {
    std::cout << "Usage: " << program_name << " -h"<< std::endl;
    std::cout << "       " << program_name << " [-v] [-e] setting.txt"<< std::endl;
    std::cout << "       " << program_name << " [-v] [-g] setting.txt"<< std::endl;
    std::cout << std::endl;
    std::cout << "For example, " << std::endl;
    std::cout << std::endl;
    std::cout << "       " << program_name << " -v -g setting01.txt"<< std::endl;
  }
};


// ********************************************************************************
//   Main Context
// ********************************************************************************


class MainContext {

protected:

  SpicompSimulator simulator;

public:

  MainContext() :
      simulator(Shared::getSetting())
  {
    // do nothing
  }

  virtual ~MainContext() {}

  virtual void run() {
    while(run_one_step()) {}  // do nothing
  }

  virtual bool run_one_step() {
    simulator.nextStep();
    return !simulator.isStopped();
  }


};


class MainContextWithGui final : public MainContext {

  SpicompGui gui;

public:

  MainContextWithGui() : gui(Shared::getSetting(), simulator) {
    // do nothing
  }

  bool run_one_step() final {
    return gui.run_one_step();
  }

};

// ********************************************************************************
//   Helper Functions
// ********************************************************************************


void emscripten_main_loop() {
  static std::unique_ptr<MainContextWithGui> main_context;
  if (!main_context) {
    main_context = std::make_unique<MainContextWithGui>();
  }
  main_context->run_one_step();
}


// ********************************************************************************
//   The Main Function
// ********************************************************************************

int main(int argc, char** argv) {

  FoctlCommandLineArgument cl_arg{argc, argv};

  if (cl_arg.isHelp()) {
    cl_arg.printHelp();
    exit(EXIT_SUCCESS);
  }

  Shared::init(cl_arg.isSettingFilenameExist()?cl_arg.getSettingFilename():"config_001.yml");


//  __vv__(Shared::getSetting().getRandSeed());
//  __vv__(Shared::getSetting().isShowRandSeed());
//  __vv__(Shared::getSetting().getWindowSizeX());
//  __vv__(Shared::getSetting().getWindowSizeY());


  if (cl_arg.isExpr()) {
    __pp__("No experiment.");
  } else {

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_main_loop, 0, true);
    // emscripten_set_main_loop([&](){ emscripten_main_loop(sim_setting); } , 0, true);  // don't work
#else
    if (cl_arg.isShowGUI()) {
      MainContextWithGui().run();
    } else {
      MainContext().run();  // currently, it will never occur.
    }
#endif

  }

  return 0;
}



