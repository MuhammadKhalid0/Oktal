#pragma once

#include "oktal/geometry/Vec.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <vector>
namespace oktal {

using morton_bits_t = uint64_t;

class MortonIndex {
public:
  MortonIndex() : bits_{1} {}
  MortonIndex(morton_bits_t bits) : bits_{bits} {}

  static constexpr unsigned int MAX_DEPTH = (sizeof(morton_bits_t) * 8 - 1) / 3;

  [[nodiscard]] morton_bits_t getBits() const { return bits_; }

  static MortonIndex fromPath(std::vector<morton_bits_t> const &v) {
    if (v.size() > MAX_DEPTH) {
      throw std::invalid_argument("Path length is too long");
    }
    morton_bits_t bits = 1;
    for (auto b : v) {
      if (b >= 8) {
        throw std::invalid_argument("Path contains invalid choice");
      }
      bits = bits << 3;
      bits |= b;
    }
    return {bits};
  }
  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, morton_bits_t>
  static MortonIndex fromPath(R const &v) {
    const auto len = std::ranges::distance(v);
    if (len > MAX_DEPTH) {
      throw std::invalid_argument("Path length is too long");
    }
    morton_bits_t bits = 1;
    for (auto b : v) {
      if (b >= 8) {
        throw std::invalid_argument("Path contains invalid choice");
      }
      bits = bits << 3;
      morton_bits_t bit = b;
      bits |= bit;
    }
    return {bits};
  }

  [[nodiscard]] std::vector<morton_bits_t> getPath() const;

  [[nodiscard]] std::size_t level() const;

  [[nodiscard]] bool isRoot() const;

  [[nodiscard]] std::size_t siblingIndex() const;

  [[nodiscard]] bool isFirstSibling() const;

  [[nodiscard]] bool isLastSibling() const;

  [[nodiscard]] MortonIndex parent() const;

  [[nodiscard]] MortonIndex safeParent() const;

  [[nodiscard]] MortonIndex child(morton_bits_t b) const;

  [[nodiscard]] MortonIndex safeChild(morton_bits_t b) const;

  bool operator==(MortonIndex const &index) const;
  bool operator!=(MortonIndex const &index) const;

  bool operator<(MortonIndex const &index) const;
  bool operator>(MortonIndex const &index) const;

  bool operator<=(MortonIndex const &index) const;
  bool operator>=(MortonIndex const &index) const;

  [[nodiscard]] Vec<size_t, 3> gridCoordinates() const;

  static MortonIndex fromGridCoordinates(size_t level,
                                         Vec<size_t, 3> const &values) {

    if (level > 0) {
      std::vector<morton_bits_t> v;
      auto x = values[0];
      auto y = values[1];
      auto z = values[2];

      for (size_t i = 0; i < level; i++) {
        auto a = z & 1;
        z = z >> 1;

        auto b = y & 1;
        y = y >> 1;

        auto c = x & 1;
        x = x >> 1;

        auto res = (a << 2) + (b << 1) + c;
        v.push_back(res);
      }
      std::ranges::reverse(v.begin(), v.end());
      return MortonIndex::fromPath(v);
    }

    return MortonIndex{};
  }

private:
  morton_bits_t bits_{};
};

} // namespace oktal