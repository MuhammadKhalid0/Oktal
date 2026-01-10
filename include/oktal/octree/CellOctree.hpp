#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "oktal/geometry/Box.hpp"
#include "oktal/geometry/Vec.hpp"
#include "oktal/octree/MortonIndex.hpp"
#include "oktal/octree/OctreeGeometry.hpp"

namespace oktal {

class OctreeCursor;

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

  static std::shared_ptr<const CellOctree> createUniformGrid(std::size_t level);
  static std::shared_ptr<const CellOctree>
  createUniformGrid(OctreeGeometry geom, std::size_t level);

  explicit CellOctree(const OctreeGeometry &geometry);
  [[nodiscard]] const OctreeGeometry &geometry() const;

  [[nodiscard]] std::optional<CellView> getCell(MortonIndex index) const;
  [[nodiscard]] bool cellExists(MortonIndex index) const;
  [[nodiscard]] std::optional<CellView> getRootCell() const;
  [[nodiscard]] auto preOrderDepthFirstRange() const;
  [[nodiscard]] auto horizontalRange(std::size_t level) const;

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

    CellView() = default;
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

class OctreeCursor {
public:
  OctreeCursor() = default;
  explicit OctreeCursor(CellOctree const &tree) : tree_(&tree), path_{0} {}
  OctreeCursor(CellOctree const &tree, std::span<const std::size_t> path)
      : tree_(&tree), path_(path.begin(), path.end()) {}
  [[nodiscard]] CellOctree const *octree() const { return tree_; }
  [[nodiscard]] std::span<const std::size_t> path() const { return path_; }
  [[nodiscard]] bool empty() const { return tree_ == nullptr; }
  [[nodiscard]] bool end() const { return !empty() && path_.empty(); }
  [[nodiscard]] std::size_t currentLevel() const { return path_.size() - 1; }
  [[nodiscard]] std::size_t currentStreamIndex() const { return path_.back(); }
  [[nodiscard]] std::optional<CellOctree::CellView> currentCell() const {
    return tree_->getCell(mortonIndex());
  } // slow
  [[nodiscard]] bool firstSibling() const {
    return path_.size() <= 1 ||
           currentStreamIndex() == getCurrentParentChildrenStartIndex();
  }
  [[nodiscard]] bool lastSibling() const {
    return path_.size() <= 1 ||
           currentStreamIndex() == getCurrentParentChildrenStartIndex() + 7;
  }
  [[nodiscard]] MortonIndex mortonIndex() const;
  bool operator==(const OctreeCursor &) const = default;
  void ascend() { path_.pop_back(); }
  void descend() { descend(0); }
  void descend(std::size_t childIdx);
  void previousSibling();
  void nextSibling();
  void toSibling(std::size_t siblingIdx);
  void toEnd() { path_.clear(); }

private:
  const CellOctree *tree_ = nullptr;
  std::vector<std::size_t> path_;
  [[nodiscard]] std::size_t getCurrentParentChildrenStartIndex() const;
};

template <typename OIP>
concept OctreeIteratorPolicy =
    std::semiregular<OIP> && requires(const OIP &policy, OctreeCursor &cursor) {
      { policy.advance(cursor) } -> std::same_as<void>;
    };

template <OctreeIteratorPolicy TPolicy> class OctreeCellsIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = CellOctree::CellView;
  using difference_type = std::ptrdiff_t;
  struct ArrowProxy {
  private:
    CellOctree::CellView v;
  public:
    const CellOctree::CellView *operator->() const { return &v; }
  };
  using pointer = ArrowProxy;
  using reference = CellOctree::CellView;
  OctreeCellsIterator() = default;
  OctreeCellsIterator(OctreeCursor cursor, TPolicy policy)
      : cursor_(std::move(cursor)), tpolicy_(std::move(policy)) {}
  reference operator*() const { return cursor_.currentCell().value_or(CellOctree::CellView{}); }
  ArrowProxy operator->() const {
    return ArrowProxy{cursor_.currentCell().value_or(CellOctree::CellView{})};
  }
  OctreeCellsIterator &operator++() {
    tpolicy_.advance(cursor_);
    return *this;
  }
  OctreeCellsIterator operator++(int) {
    OctreeCellsIterator temp = *this;
    ++(*this);
    return temp;
  }
  friend bool operator==(const OctreeCellsIterator &lhs,
                         const OctreeCellsIterator &rhs) {
    return lhs.cursor_ == rhs.cursor_;
  }
  friend bool operator!=(const OctreeCellsIterator &lhs,
                         const OctreeCellsIterator &rhs) {
    return !(lhs == rhs);
  }

private:
  OctreeCursor cursor_;
  TPolicy tpolicy_;
};
template <typename T> using OctreeIterator = OctreeCellsIterator<T>;

template <OctreeIteratorPolicy TPolicy> class OctreeCellsRange {
public:
  OctreeCellsRange(OctreeCursor start, OctreeCursor end, TPolicy policy)
      : start_(std::move(start)), end_(std::move(end)),
        policy_(std::move(policy)) {}
  [[nodiscard]] OctreeCellsIterator<TPolicy> begin() const {
    return OctreeCellsIterator<TPolicy>(start_, policy_);
  }
  [[nodiscard]] OctreeCellsIterator<TPolicy> end() const {
    return OctreeCellsIterator<TPolicy>(end_, policy_);
  }

private:
  OctreeCursor start_;
  OctreeCursor end_;
  TPolicy policy_;
};

class PreOrderDFSPolicy {
public:
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  void advance(OctreeCursor &c) const {
    if (c.end()) {
      return;
    }
    while (true) {
      const auto prevLevel = c.currentLevel();
      c.descend();
      if (c.currentLevel() == prevLevel) {
        while (c.lastSibling()) {
          if (c.currentLevel() == 0) {
            c.toEnd();
            return;
          }
          c.ascend();
        }
        c.nextSibling();
      }
      if (c.end()) {
        return;
      }
      if (c.currentCell().has_value()) {
        return;
      }
    }
  }
};

class HorizontalPolicy {
public:
  HorizontalPolicy() = default;
  explicit HorizontalPolicy(std::size_t targetLevel)
      : targetLevel_(targetLevel) {}

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  void advance(OctreeCursor &c) const {
    if (c.end()) {
      return;
    }
    while (true) {
      const auto prevLevel = c.currentLevel();
      if (c.currentLevel() < targetLevel_) {
        c.descend();
      } 
      if (c.currentLevel() == prevLevel) {
        while (c.lastSibling()) {
          if (c.currentLevel() == 0) {
            c.toEnd();
            return;
          }
          c.ascend();
        }
        c.nextSibling();
      }
      if (c.end()) {
        return;
      }
      if (c.currentLevel() == targetLevel_ && c.currentCell().has_value()) {
        return;
      }
    }
  }

private:
  std::size_t targetLevel_{0};
};

inline auto CellOctree::preOrderDepthFirstRange() const {
  OctreeCursor start(*this);
  OctreeCursor end(*this);
  end.toEnd();

  if (!start.end()) {
    if (!start.currentCell().has_value()) {
      const PreOrderDFSPolicy policy;
      policy.advance(start);
    }
  }

  return OctreeCellsRange<PreOrderDFSPolicy>(std::move(start), std::move(end),
                                             PreOrderDFSPolicy{});
}
inline auto CellOctree::horizontalRange(std::size_t level) const {
  OctreeCursor start(*this);
  OctreeCursor end(*this);
  end.toEnd();

  if (!start.end()) {
    if (level >= start.octree()->numberOfLevels()) {
      start.toEnd();
      return OctreeCellsRange<HorizontalPolicy>(std::move(start), std::move(end),
                                            HorizontalPolicy{level});
    }
    if (!start.currentCell().has_value() || start.currentLevel() < level) {
      const HorizontalPolicy policy(level);
      policy.advance(start);
    }
  }

  return OctreeCellsRange<HorizontalPolicy>(std::move(start), std::move(end),
                                            HorizontalPolicy{level});
}

} // namespace oktal
