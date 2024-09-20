#include <sstream>
#include <algorithm>

#include "util/rng.h"


std::unique_ptr<SharedRand> SharedRand::instance = nullptr;
std::once_flag SharedRand::flag;


void SharedRand::init(std::random_device::result_type rand_seed, bool is_show_rand_seed) {

  // cout setting
  std::cout << std::boolalpha;

  // random seek
  std::call_once(SharedRand::flag, [&]() {
    SharedRand::instance.reset(new SharedRand);
    SharedRand::instance->setRandSeed(rand_seed);
    if (is_show_rand_seed) {
      std::cout << "Random seed = " << SharedRand::instance->rand_seed << std::endl;
    }
  });
}


void SharedRand::setRandSeed(std::random_device::result_type new_rand_seed) {
  if (new_rand_seed == 0) {
    std::random_device dev;
    new_rand_seed = dev();
  }
  rand_seed = new_rand_seed;

  rng = std::mt19937{rand_seed};
}


int SharedRand::getRandWeightedIndex(const std::vector<double>& weights) {
  assert(!weights.empty());
  auto total_weight = std::accumulate(weights.begin(), weights.end(), 0.0);
  std::vector<double> running_weights(weights.size());
  running_weights[0] = weights[0] / total_weight;
  for(int i=1; i<weights.size(); i++) {
    running_weights[i] = running_weights[i-1] + (weights[i] / total_weight);
  }
  double rand_double = getRandDouble();
  for(int i=0; i<weights.size()-1; i++) {
    if (rand_double < running_weights[i])  return i;
  }
  return weights.size()-1;
}


std::vector<int> makeRandomIntSeq(std::mt19937& rng, unsigned int size) {
  std::vector<int> result(size);

  for(unsigned int i=0; i<size; i++) {
    result[i] = i;
  }

  std::uniform_int_distribution<> rand_gen(0, size-1);

  for(unsigned int i=0; i<size; i++) {
    auto j = rand_gen(rng);
    auto x = result[i];
    result[i] = result[j];
    result[j] = x;
  }

  return result;
}
