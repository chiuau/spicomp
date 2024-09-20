#ifndef UTIL_MATH_H
#define UTIL_MATH_H


#include <cmath>
#include <random>

#define EPSILON 0.000001
#define TWO_PI  (2.0 * M_PI)


/* --------------------------------------------------------------------------------------------------
 * Basic calculations
 * -------------------------------------------------------------------------------------------------- */

constexpr bool isZero(double v) {
  return fabs(v) <= EPSILON;
}

constexpr bool isNotZero(double v) {
  return fabs(v) > EPSILON;
}

constexpr bool isEqual(double v1, double v2) {
  return fabs(v2 - v1) <= EPSILON;
}

constexpr bool isNotEqual(double v1, double v2) {
  return fabs(v2 - v1) > EPSILON;
}

constexpr int int_ceil(float v) {
  return static_cast<int>(std::ceil(v));
}

constexpr int int_ceil(double v) {
  return static_cast<int>(std::ceil(v));
}

constexpr double distance(double x1, double y1, double x2, double y2) {
  double dx = x2 - x1;
  double dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

constexpr float distance(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

constexpr double distance(int x1, int y1, int x2, int y2) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  return sqrt(static_cast<double>(dx * dx + dy * dy));
}

template<typename T>
class MinMaxRange {

  T min_value = std::numeric_limits<T>::max();
  T max_value = std::numeric_limits<T>::min();

public:

  MinMaxRange() = default;

  MinMaxRange(T min_value, T max_value) :
      min_value(min_value), max_value(max_value)
  {
    // do nothing
  }

  T getMinValue() const { return min_value; }
  T getMaxValue() const { return max_value; }

  bool isValidRange() const { return min_value <= max_value; }

  void insertValue(T v) {
    if (v < min_value) min_value = v;
    if (v > max_value) max_value = v;
  }

  void crop(T lower_bound, T upper_bound) {
    if (isValidRange()) {  // no need to crop invalid range
      if (min_value < lower_bound) min_value = lower_bound;
      if (max_value > upper_bound) max_value = upper_bound;
    }
  }

  void crop(const MinMaxRange<T>& bound) { crop(bound.min_value, bound.max_value); }

  bool isOverlapped(const MinMaxRange<T>& r) const {  // assume *this and r are valid.
    // if (!isValidRange() || !r.isValidRange()) return false;
    return !(min_value > r.max_value || max_value < r.min_value);
  }

  MinMaxRange<T> calcIntersection(const MinMaxRange<T>& r) const {  // assume they are overlapped
    MinMaxRange<T> result = r;
    result.crop(min_value, max_value);
    return result;
  }

  T getLength() const {
    if (isValidRange()) {
      return getMaxValue() - getMinValue() + 1;
    } else {
      return -1;
    }
  }

};

template<typename T, typename V>
class MinKeeper {
  T min_data;
  V min_value;
public:
  MinKeeper(T default_data, V min_value = std::numeric_limits<V>::max()) : min_data{default_data}, min_value{min_value} {}

  void insert(T data, V value) {
    if (value < min_value) {
      min_data = data;
      min_value = value;
    }
  }

  T getMinData() const { return min_data; }
  V getMinValue() const { return min_value; }

};

template<typename T, typename V>
class MaxKeeper {
  T max_data;
  V max_value;
public:
  MaxKeeper(T default_data, V max_value = std::numeric_limits<V>::min()) : max_data{default_data}, max_value{max_value} {}

  void insert(T data, V value) {
    if (value > max_value) {
      max_data = data;
      max_value = value;
    }
  }

  T getMaxData() const { return max_data; }
  V getMaxValue() const { return max_value; }

};


#endif //UTIL_MATH_H
