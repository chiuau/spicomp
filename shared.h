#ifndef MULTIPLAN_SHARED_H
#define MULTIPLAN_SHARED_H

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <random>
#include <cassert>
#include <string_view>
#include <cmath>
// #include <experimental/source_location>
#include <utility>
#include <type_traits>

#include "util/debug.h"
#include "util/math.h"
#include "util/stl.h"
#include "util/rng.h"

#include "spicomp_setting.h"


// #define NDEBUG


/* --------------------------------------------------------------------------------------------------
 * A singleton of global variables.
 * -------------------------------------------------------------------------------------------------- */

class Shared final {
public:

  ~Shared() = default;

  Shared(const Shared &) = delete;
  Shared(Shared &&) = delete;
  Shared &operator=(const Shared &) = delete;
  Shared &operator=(Shared &&) = delete;

  static void init(const std::string& setting_filename);

  static inline SpicompSetting& getSetting() { return instance->setting; }

private:

  Shared() = default;

private:

  static std::unique_ptr<Shared> instance;
  static std::once_flag flag;

  SpicompSetting setting;
};


#endif //MULTIPLAN_SHARED_H
