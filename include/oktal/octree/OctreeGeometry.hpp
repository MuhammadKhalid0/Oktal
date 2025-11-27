#pragma once

#include "oktal/geometry/Box.hpp"
#include "oktal/geometry/Vec.hpp"
#include "oktal/octree/MortonIndex.hpp"

namespace oktal {

class OctreeGeometry {
public:
  using vector_type = Vec3D;
  OctreeGeometry() : side_length_(1.0) {}
  OctreeGeometry(vector_type const &origin, double side_length)
      : origin_(origin), side_length_(side_length) {}

  [[nodiscard]] constexpr vector_type origin() const { return origin_; }

  [[nodiscard]] constexpr double sidelength() const { return side_length_; }

  [[nodiscard]] constexpr double dx(std::size_t level) const {
    std::size_t const cells_per_dim = std::size_t{1} << level;
    return side_length_ / static_cast<double>(cells_per_dim);
  }

  [[nodiscard]] vector_type cellExtents(std::size_t level) const {
    auto length = dx(level);
    return {length, length, length};
  }

  [[nodiscard]] vector_type cellMinCorner(MortonIndex const &m) const {
    auto coord = static_cast<vector_type>(m.gridCoordinates());
    return origin_ + (coord * dx(m.level()));
  }

  [[nodiscard]] vector_type cellMaxCorner(MortonIndex const &m) const {
    auto length = dx(m.level());
    return cellMinCorner(m) + vector_type(length);
  }

  [[nodiscard]] Box<double> cellBoundingBox(MortonIndex const &m) const {
    return {cellMinCorner(m), cellMaxCorner(m)};
  }

  [[nodiscard]] vector_type cellCenter(MortonIndex const &m) const {
    auto half_length = dx(m.level()) / 2.0;
    return cellMinCorner(m) + vector_type(half_length);
  }

private:
  vector_type origin_;
  double side_length_;
};

} // namespace oktal