#include <oktal/octree/OctreeGeometry.hpp>
namespace oktal {

using vector_type = OctreeGeometry::vector_type;

vector_type OctreeGeometry::cellExtents(std::size_t level) const {
  auto length = dx(level);
  return {length, length, length};
}

vector_type OctreeGeometry::cellMinCorner(MortonIndex const &m) const {
  auto coord = static_cast<OctreeGeometry::vector_type>(m.gridCoordinates());
  return origin_ + (coord * dx(m.level()));
}

vector_type OctreeGeometry::cellMaxCorner(MortonIndex const &m) const {
  auto length = dx(m.level());
  return cellMinCorner(m) + vector_type(length);
}

Box<double> OctreeGeometry::cellBoundingBox(MortonIndex const &m) const {
  return {cellMinCorner(m), cellMaxCorner(m)};
}

vector_type OctreeGeometry::cellCenter(MortonIndex const &m) const {
  auto half_length = dx(m.level()) / 2.0;
  return cellMinCorner(m) + vector_type(half_length);
}
} // namespace oktal