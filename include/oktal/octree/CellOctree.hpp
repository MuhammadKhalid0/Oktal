#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

using std::size_t;
using std::span;
using std::string_view;
using std::vector;

namespace oktal {

class CellOctree {

public:

  class Node;

  CellOctree();

  size_t numberOfNodes() const;
  size_t numberOfLevels() const;
  size_t numberOfNodes( size_t level ) const; // Overload function to return the number of nodes given a certain level

  span<const Node> nodesStream() const; // should return a view to the entire node stream
  span<const Node> nodesStream( size_t level ) const; // should return a view to the node stream in a given level

  static CellOctree fromDescriptor( string_view descriptor ); //to inatalize an octree from a descriptor string X|.X...XP.|PPPP........PPPP 

  class Node {

  public:

    Node() = default;

    Node( bool refined, bool phantom, size_t childrenIdx = 0 )
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

    size_t childrenStartIndex() const {
      return childrenStartIndex_;
    }

    void setChildrenStartIndex( size_t idx ) {
      childrenStartIndex_ = idx;
    }

    size_t childIndex( size_t branch ) const {
      return childrenStartIndex_ + branch;
    }

  private:
// Private members for Node
    bool refined_{ false };
    bool phantom_{ false };
    size_t childrenStartIndex_{ 0 };
  };

private:
// Private members for CellOctree
  vector<Node> nodes_{};
  vector<size_t> levelStarts_{};
  vector<size_t> levelCounts_{};
};

} // namespace oktal
