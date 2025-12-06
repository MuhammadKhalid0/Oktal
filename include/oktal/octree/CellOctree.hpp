#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

#include <optional>

#include "oktal/octree/OctreeGeometry.hpp"
#include "oktal/octree/MortonIndex.hpp"
#include "oktal/geometry/Box.hpp"
#include "oktal/geometry/Vec.hpp"

namespace oktal {

class CellOctree {

public:

  class Node;
  class CellView;

  CellOctree();

  std::size_t numberOfNodes() const;
  std::size_t numberOfLevels() const;
  std::size_t numberOfNodes( std::size_t level ) const; // Overload function to return the number of nodes given a certain level

  std::span<const Node> nodesStream() const; // should return a view to the entire node stream
  std::span<const Node> nodesStream( std::size_t level ) const; // should return a view to the node stream in a given level

  static CellOctree fromDescriptor( std::string_view descriptor ); //to inatalize an octree from a descriptor string X|.X...XP.|PPPP........PPPP 

  explicit CellOctree( const OctreeGeometry &geometry );
  const OctreeGeometry &geometry() const;

  std::optional<CellView> getCell( MortonIndex index ) const;
  bool cellExists( MortonIndex index ) const;
  std::optional<CellView> getRootCell() const;

  class Node {

  public:

    Node() = default;

    Node( bool refined, bool phantom, std::size_t childrenIdx = 0 )
      : refined_( refined ),
        phantom_( phantom ),
        childrenStartIndex_( childrenIdx ) { }

    bool isRefined() const {
      return refined_;
    }

    bool isPhantom() const {
      return phantom_;
    }

    void setRefined( bool refined ) {
      refined_ = refined;
    }

    void setPhantom( bool phantom ) {
      phantom_ = phantom;
    }

    std::size_t childrenStartIndex() const {
      return childrenStartIndex_;
    }

    void setChildrenStartIndex( std::size_t idx ) {
      childrenStartIndex_ = idx;
    }

    std::size_t childIndex( std::size_t branch ) const {
      return childrenStartIndex_ + branch;
    }

  private:
    // Private members for Node
    bool refined_{ false };
    bool phantom_{ false };
    std::size_t childrenStartIndex_{ 0 };
  };

  class CellView {
  public:
    MortonIndex mortonIndex() const;
    bool isRoot() const;
    bool isRefined() const;
    std::size_t level() const;
    std::size_t streamIndex() const;

    Vec3D center() const;
    Box<double> boundingBox() const;

  private:
    friend class CellOctree;

    CellView( const CellOctree *tree,
              std::size_t streamIndex,
              MortonIndex mortonIndex );

    const CellOctree *tree_{ nullptr };
    std::size_t streamIndex_{ 0 };
    MortonIndex mortonIndex_{};
  };

private:
  // Private members for CellOctree
  OctreeGeometry geometry_;
  std::vector<Node> nodes_{};
  std::vector<std::size_t> levelStarts_{};
  std::vector<std::size_t> levelCounts_{};
};

} // namespace oktal
