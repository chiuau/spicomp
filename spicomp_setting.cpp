#include "spicomp_setting.h"



void SpicompSetting::init(const std::string& setting_filename) {

  SpicompSetting::setting_filename = setting_filename;

  YAML::Node config = YAML::LoadFile(DEFAULT_SETTING_DIRECTORY + setting_filename);


  if (config["RandSeed"]) {
    rand_seed = config["RandSeed"].as<std::random_device::result_type>();
  } else {
    throw std::runtime_error("RandSeed not found in " + setting_filename);
  }

  if (config["IsShowRandSeed"]) {
    is_show_rand_seed = config["IsShowRandSeed"].as<bool>();
  } else {
    throw std::runtime_error("IsShowRandSeed not found " + setting_filename);
  }


  if (config["WindowSizeX"]) {
    window_size_x = config["WindowSizeX"].as<int>();
  } else {
    throw std::runtime_error("WindowSizeX not found " + setting_filename);
  }

  if (config["WindowSizeY"]) {
    window_size_y = config["WindowSizeY"].as<int>();
  } else {
    throw std::runtime_error("WindowSizeY not found " + setting_filename);
  }

  if (config["SceneSizeX"]) {
    scene_size_x = config["SceneSizeX"].as<double>();
  } else {
    throw std::runtime_error("SceneSizeX not found " + setting_filename);
  }

  if (config["SceneSizeY"]) {
    scene_size_y = config["SceneSizeY"].as<double>();
  } else {
    throw std::runtime_error("SceneSizeY not found " + setting_filename);
  }

  if (config["SceneSizeZ"]) {
    scene_size_z = config["SceneSizeZ"].as<double>();
  } else {
    throw std::runtime_error("SceneSizeZ not found " + setting_filename);
  }

}