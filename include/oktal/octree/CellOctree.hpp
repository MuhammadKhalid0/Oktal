#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

#include <optional>

#include "oktal/geometry/Box.hpp"
#include "oktal/geometry/Vec.hpp"
#include "oktal/octree/MortonIndex.hpp"
#include "oktal/octree/OctreeGeometry.hpp"

namespace oktal {

class CellOctree {

public:
  class Node;
  class CellView;

  CellOctree();

  [[nodiscard]] std::size_t numberOfNodes() const;
  [[nodiscard]] std::size_t numberOfLevels() const;
  [[nodiscard]] std::size_t numberOfNodes(
      std::size_t level) const; // Overload function to return the number of
                                // nodes given a certain level

  [[nodiscard]] std::span<const Node>
  nodesStream() const; // should return a view to the entire node stream
  [[nodiscard]] std::span<const Node> nodesStream(std::size_t level)
      const; // should return a view to the node stream in a given level

  static CellOctree fromDescriptor(
      std::string_view descriptor); // to inatalize an octree from a descriptor
                                    // string X|.X...XP.|PPPP........PPPP

  explicit CellOctree(const OctreeGeometry &geometry);
  [[nodiscard]] const OctreeGeometry &geometry() const;

  [[nodiscard]] std::optional<CellView> getCell(MortonIndex index) const;
  [[nodiscard]] bool cellExists(MortonIndex index) const;
  [[nodiscard]] std::optional<CellView> getRootCell() const;

  class Node {

  public:
    Node() = default;

    Node(bool refined, bool phantom, std::size_t childrenIdx = 0)
        : childrenStartIndex_(childrenIdx) {
      if (refined) {
        childrenStartIndex_ |= 1ul << 63;
      }
      if (phantom) {
        childrenStartIndex_ |= 1ul << 62;
      }
    }

    [[nodiscard]] bool isRefined() const {
      return (childrenStartIndex_ & 1ul << 63) > 0;
    }

    [[nodiscard]] bool isPhantom() const {
      return (childrenStartIndex_ & 1ul << 62) > 0;
    }

    void setRefined(bool refined) {
      if (refined) {
        childrenStartIndex_ |= 1ul << 63;
      } else {
        unsigned long bitMask = 1ul << 63;
        bitMask -= 1;
        childrenStartIndex_ &= bitMask;
      }
    }

    void setPhantom(bool phantom) {
      if (phantom) {
        childrenStartIndex_ |= 1ul << 62;
      } else {
        unsigned long bitMask = 1ul << 62;
        bitMask -= 1;
        bitMask += 1ul << 63;
        childrenStartIndex_ &= bitMask;
      }
    }

    [[nodiscard]] std::size_t childrenStartIndex() const {
      unsigned long bitMask = 1ul << 62;
      bitMask -= 1;
      return childrenStartIndex_ & bitMask;
    }

    void setChildrenStartIndex(std::size_t idx) {
      auto ref = isRefined();
      auto ph = isPhantom();
      childrenStartIndex_ = idx;
      setRefined(ref);
      setPhantom(ph);
    }

    [[nodiscard]] std::size_t childIndex(std::size_t branch) const {
      return childrenStartIndex() + branch;
    }

  private:
    // Private members for Node
    std::size_t childrenStartIndex_{0};
  };

  class CellView {
  public:
    [[nodiscard]] MortonIndex mortonIndex() const;
    [[nodiscard]] bool isRoot() const;
    [[nodiscard]] bool isRefined() const;
    [[nodiscard]] std::size_t level() const;
    [[nodiscard]] std::size_t streamIndex() const;

    [[nodiscard]] Vec3D center() const;
    [[nodiscard]] Box<double> boundingBox() const;

  private:
    friend class CellOctree;

    CellView(const CellOctree *tree, std::size_t streamIndex,
             MortonIndex mortonIndex);

    const CellOctree *tree_{nullptr};
    std::size_t streamIndex_{0};
    MortonIndex mortonIndex_;
  };

private:
  // Private members for CellOctree
  OctreeGeometry geometry_;
  std::vector<Node> nodes_;
  std::vector<std::size_t> levelStarts_;
  std::vector<std::size_t> levelCounts_;
};

} // namespace oktal
