#include <oktal/octree/MortonIndex.hpp>
using namespace oktal;

[[nodiscard]] std::vector<morton_bits_t> MortonIndex::getPath() const {
  std::vector<morton_bits_t> v{};
  morton_bits_t bits = bits_;
  while (bits > 1) {
    v.push_back(bits & 0b111);
    bits = bits >> 3;
  }
  std::ranges::reverse(v.begin(), v.end());
  return v;
}

[[nodiscard]] std::size_t MortonIndex::level() const {
  auto bits = bits_;
  std::size_t level = 0;
  while (bits > 1) {
    bits = bits >> 3;
    ++level;
  }
  return level;
}

[[nodiscard]] bool MortonIndex::isRoot() const { return bits_ == 1; }

[[nodiscard]] std::size_t MortonIndex::siblingIndex() const {
  return isRoot() ? 0 : bits_ & 0b111;
}

[[nodiscard]] bool MortonIndex::isFirstSibling() const {
  return siblingIndex() == 0;
}

[[nodiscard]] bool MortonIndex::isLastSibling() const {
  return isRoot() ? true : MortonIndex::siblingIndex() == 0b111;
}

[[nodiscard]] MortonIndex MortonIndex::parent() const {
  return MortonIndex{bits_ >> 3};
}

[[nodiscard]] MortonIndex MortonIndex::safeParent() const {
  if (isRoot()) {
    throw std::logic_error("Calling parent method on root not allowed");
  }
  return parent();
}

[[nodiscard]] MortonIndex MortonIndex::child(morton_bits_t b) const {
  return MortonIndex{(bits_ << 3) | b};
}

[[nodiscard]] MortonIndex MortonIndex::safeChild(morton_bits_t b) const {
  if (level() == MAX_DEPTH) {
    throw std::logic_error(
        "Can't add a child because level is already at max depth");
  }
  return child(b);
}

bool MortonIndex::operator==(MortonIndex const &index) const {
  return bits_ == index.getBits();
}

bool MortonIndex::operator!=(MortonIndex const &index) const {
  return bits_ != index.getBits();
}

bool MortonIndex::operator<(MortonIndex const &index) const {
  if (index.level() >= level()) {
    return false;
  }
  if (index.isRoot()) {
    return true;
  }
  auto bits = index.getBits();
  while (MortonIndex(bits).level() < level()) {
    bits = bits << 3;
  }
  return (getBits() & bits) == bits;
}

bool MortonIndex::operator>(MortonIndex const &index) const {
  if (index.level() <= level()) {
    return false;
  }
  if (isRoot()) {
    return true;
  }
  auto bits = getBits();
  while (MortonIndex(bits).level() < index.level()) {
    bits = bits << 3;
  }
  return (index.getBits() & bits) == bits;
}

bool MortonIndex::operator<=(MortonIndex const &index) const {
  return ((*this) < index) || ((*this) == index);
}
bool MortonIndex::operator>=(MortonIndex const &index) const {
  return ((*this) > index) || ((*this) == index);
}

[[nodiscard]] Vec<size_t, 3> MortonIndex::gridCoordinates() const {
  if (isRoot()) {
    return {};
  };

  morton_bits_t x = 0;
  morton_bits_t y = 0;
  morton_bits_t z = 0;
  auto path = getPath();
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
