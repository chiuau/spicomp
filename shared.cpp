#include <sstream>
#include <algorithm>

#include "shared.h"


std::unique_ptr<Shared> Shared::instance = nullptr;
std::once_flag Shared::flag;


void Shared::init(const std::string& setting_filename) {
  std::cout << std::boolalpha << std::fixed;

  std::call_once(Shared::flag, [&]() {
    Shared::instance.reset(new Shared);
    Shared::instance->getSetting().init(setting_filename);
    SharedRand::init(Shared::instance->getSetting().getRandSeed(), Shared::instance->getSetting().isShowRandSeed());
  });
}
