#ifndef UTIL_RNG_H
#define UTIL_RNG_H

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

#include "stl.h"

// #define NDEBUG


/* --------------------------------------------------------------------------------------------------
 * A singleton of global variables.
 * -------------------------------------------------------------------------------------------------- */

class SharedRand final {
public:

  ~SharedRand() = default;

  SharedRand(const SharedRand &) = delete;
  SharedRand(SharedRand &&) = delete;
  SharedRand &operator=(const SharedRand &) = delete;
  SharedRand &operator=(SharedRand &&) = delete;

  static void init(std::random_device::result_type rand_seed,
                   bool is_show_rand_seed);

  [[nodiscard]]
  inline static std::random_device::result_type getRandSeed() {
    return instance->rand_seed;
  }

  inline static std::mt19937& getRng() {
    return instance->rng;
  }

  inline static int getRandInt(int range) {
    assert(range > 0);
    std::uniform_int_distribution<> rand_gen(0, range-1);
    return rand_gen(instance->rng);
  }

  inline static double getRandDouble() {
    std::uniform_real_distribution<> rand_gen(0.0, 1.0);
    return rand_gen(instance->rng);
  }

  static int getRandWeightedIndex(const std::vector<double>& weights);

private:

  SharedRand() = default;

  void setRandSeed(std::random_device::result_type rand_seed);

  static std::unique_ptr<SharedRand> instance;
  static std::once_flag flag;

  std::random_device::result_type rand_seed = 0;
  std::mt19937 rng;
};


/* --------------------------------------------------------------------------------------------------
 * Utility functions
 * -------------------------------------------------------------------------------------------------- */

std::vector<int> makeRandomIntSeq(std::mt19937& rng, unsigned int size);



#endif //UTIL_RNG_H
