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

private:
  static constexpr std::size_t BITS_PER_LEVEL_ = 3;

  static morton_bits_t append_decision(morton_bits_t bits, morton_bits_t const decision) { 
    if (decision >= 8) { 
      throw std::invalid_argument("Path contains invalid choice"); 
    } 
    bits = bits << BITS_PER_LEVEL_;
    bits |= decision;
    return bits;
  }
  
public:
  static constexpr unsigned int MAX_DEPTH = (sizeof(morton_bits_t) * 8 - 1) / BITS_PER_LEVEL_;

  [[nodiscard]] morton_bits_t getBits() const { return bits_; }

  static MortonIndex fromPath(std::vector<morton_bits_t> const &v) {
    if (v.size() > MAX_DEPTH) {
      throw std::invalid_argument("Path length is too long");
    }
    morton_bits_t bits = 1;
    for (auto b : v) {
      bits = append_decision(bits, b);
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
      auto const bit = static_cast<morton_bits_t>(b);
      bits = append_decision(bits, bit);
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

  bool operator<(MortonIndex const &possible_ancestor) const;
  bool operator>(MortonIndex const &possible_descendant) const;

  bool operator<=(MortonIndex const &index) const;
  bool operator>=(MortonIndex const &index) const;

  [[nodiscard]] Vec<size_t, 3> gridCoordinates() const;

  [[nodiscard]] static MortonIndex fromGridCoordinates(size_t level,
                                         Vec<size_t, 3> const &values) {
    auto path = gridCoordsToPath(level, values);
    return MortonIndex::fromPath(path);
  }

private:
  morton_bits_t bits_{};

  [[nodiscard]] static std::vector<morton_bits_t> gridCoordsToPath(size_t level, Vec<size_t, 3> const &values){
    if (level == 0) {
      return {};
    }

    const morton_bits_t max_coord = morton_bits_t{1} << level; // 2 to the power of level
    if (values[0] >= max_coord || values[1] >= max_coord || values[2] >= max_coord) {
        throw std::out_of_range("grid coordinate does not match level");
    }

    std::vector<morton_bits_t> path;
    auto x = static_cast<morton_bits_t>(values[0]);
    auto y = static_cast<morton_bits_t>(values[1]);
    auto z = static_cast<morton_bits_t>(values[2]);

    for (size_t i = 0; i < level; i++) {
      morton_bits_t const z_bit = z & 1;
      z = z >> 1;

      morton_bits_t const y_bit = y & 1;
      y = y >> 1;

      morton_bits_t const x_bit = x & 1;
      x = x >> 1;

      morton_bits_t const res = (z_bit << 2) + (y_bit << 1) + x_bit;
      path.push_back(res);
    }
    std::ranges::reverse(path.begin(), path.end());
    return path;
  }

  [[nodiscard]] static Vec<size_t, 3> pathToGridCoords(std::vector<oktal::morton_bits_t>& path) {
  morton_bits_t x = 0;
  morton_bits_t y = 0;
  morton_bits_t z = 0;
  for (auto e : path) {

    x = x << 1;
    x += e & 0b1;

    y = y << 1;
    y += (e & 0b10) >> 1;

    z = z << 1;
    z += (e & 0b100) >> 2;
  }
  return {x, y, z};
}

};

} // namespace oktal