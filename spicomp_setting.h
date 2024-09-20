#ifndef SPICOMP_SPICOMP_SETTING_H
#define SPICOMP_SPICOMP_SETTING_H


#include <iostream>
#include <fstream>
#include <random>

#include "util/debug.h"
#include "yaml-cpp/yaml.h"

#ifdef __EMSCRIPTEN__
#define DEFAULT_SETTING_DIRECTORY "/testcase/"
#else
#define DEFAULT_SETTING_DIRECTORY "/Users/chiu/work/Papers/2025-ICRA-drone-game/code/spicomp/testcase/"
#endif


namespace YAML {
  template<>
  struct convert<std::pair<int,int>> {
    static Node encode(const std::pair<int,int>& rhs) {
      Node node;
      node.push_back(rhs.first);
      node.push_back(rhs.second);
      return node;
    }

    static bool decode(const Node& node, std::pair<int,int>& rhs) {
      if(!node.IsSequence() || node.size() != 2) {
        return false;
      }
      rhs.first = node[0].as<int>();
      rhs.second = node[1].as<int>();
      return true;
    }
  };
}


class SpicompSetting {

  std::string setting_filename;

  std::random_device::result_type rand_seed = 0;
  bool is_show_rand_seed = false;

  int window_size_x;
  int window_size_y;

  double scene_size_x;
  double scene_size_y;
  double scene_size_z;


public:

  void init(const std::string& setting_filename);

  std::random_device::result_type getRandSeed() const {  return rand_seed; }
  bool isShowRandSeed() const { return is_show_rand_seed; }

  int getWindowSizeX() const { return window_size_x; }
  int getWindowSizeY() const { return window_size_y; }

  double getSceneSizeX() const { return scene_size_x; }
  double getSceneSizeY() const { return scene_size_y; }
  double getSceneSizeZ() const { return scene_size_z; }

};



#endif //SPICOMP_SPICOMP_SETTING_H
