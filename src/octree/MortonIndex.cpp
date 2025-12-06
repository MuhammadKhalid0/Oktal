#include <oktal/octree/MortonIndex.hpp>
using namespace oktal;

[[nodiscard]] std::vector<morton_bits_t> MortonIndex::getPath() const {
  std::vector<morton_bits_t> v{};
  morton_bits_t bits = bits_;
  while (bits > 1) {
    v.push_back(bits & 0b111);
    bits = bits >> BITS_PER_LEVEL_;
  }
  std::ranges::reverse(v.begin(), v.end());
  return v;
}

[[nodiscard]] std::size_t MortonIndex::level() const {
  auto zeroes = static_cast<std::size_t>(std::countl_zero(bits_));
  return MAX_DEPTH - zeroes / BITS_PER_LEVEL_;
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
  return !(*this==index);
}

bool MortonIndex::operator<(MortonIndex const &possible_ancestor) const { 
  //readable
  auto ancestor_level = possible_ancestor.level(); 
  if (ancestor_level >= level()) { 
    return false; 
  } 
  if (possible_ancestor.isRoot()) { 
    return true; 
  } 
  auto possible_descendant = *this; 
  while(possible_descendant.level() > ancestor_level) { 
    possible_descendant = possible_descendant.parent();
  }
  return possible_descendant == possible_ancestor;
}

bool MortonIndex::operator>(MortonIndex const &possible_descendant) const {
  // performant
  auto bit_delta = std::countl_zero(bits_) - std::countl_zero(possible_descendant.bits_);
  if (bit_delta <= 0) {
    return false;
  }
  if (isRoot()) {
    return true;
  }
  auto desc_bits = possible_descendant.bits_>> bit_delta;
  return desc_bits == bits_;
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
  auto path = getPath();
  return pathToGridCoords(path);
}
