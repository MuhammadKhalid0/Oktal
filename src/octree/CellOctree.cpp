#include "oktal/octree/CellOctree.hpp"

#include <stdexcept>
#include <string_view>

using namespace std;

namespace oktal {

CellOctree::CellOctree()
{
    nodes_.clear();
    levelStarts_.clear();
    levelCounts_.clear();

    // trivial tree: root only
    nodes_.emplace_back();
    levelStarts_.push_back( 0 ); //Level 0
    levelCounts_.push_back( 1 ); //one node to level 0
}

CellOctree::CellOctree( const OctreeGeometry &geometry ) //Constructor to set Geometry
    : geometry_( geometry )
{
    
    nodes_.clear();
    levelStarts_.clear();
    levelCounts_.clear();

    // trivial tree: root only
    nodes_.emplace_back();
    levelStarts_.push_back( 0 );
    levelCounts_.push_back( 1 );
}


size_t CellOctree::numberOfNodes() const
{
    return nodes_.size();
}

size_t CellOctree::numberOfLevels() const
{
    return levelCounts_.size();
}

size_t CellOctree::numberOfNodes( size_t level ) const
{
    return levelCounts_[ level ];
}

span<const CellOctree::Node> CellOctree::nodesStream() const
{
    return span<const Node>( nodes_.data(), nodes_.size() );
}

span<const CellOctree::Node> CellOctree::nodesStream( size_t level ) const
{
    const size_t start = levelStarts_[ level ];
    const size_t count = levelCounts_[ level ];

    return std::span<const Node>( nodes_.data() + start, count );
}

const OctreeGeometry &CellOctree::geometry() const
{
    return geometry_;
}

CellOctree::CellView::CellView( const CellOctree *tree,
                                size_t streamIndex,
                                MortonIndex mortonIndex )
    : tree_( tree )
    , streamIndex_( streamIndex )
    , mortonIndex_( mortonIndex )
{}

MortonIndex CellOctree::CellView::mortonIndex() const
{
    return mortonIndex_;
}

bool CellOctree::CellView::isRoot() const
{
    return mortonIndex_.isRoot();
}

bool CellOctree::CellView::isRefined() const
{
    return tree_->nodes_[ streamIndex_ ].isRefined();
}

size_t CellOctree::CellView::level() const
{
    return mortonIndex_.level();
}

size_t CellOctree::CellView::streamIndex() const
{
    return streamIndex_;
}

Vec3D CellOctree::CellView::center() const
{
    return tree_->geometry_.cellCenter( mortonIndex_ );
}

Box<double> CellOctree::CellView::boundingBox() const
{
    return tree_->geometry_.cellBoundingBox( mortonIndex_ );
}

// test case TestCellView::testCellQueries
std::optional<CellOctree::CellView>
CellOctree::getCell( MortonIndex index ) const
{
    // start at root
    size_t idx = 0;
    auto path = index.getPath(); // vector<morton_bits_t> with entries 0..7 from root to this index

    for ( auto choice : path ) {
        const Node &node = nodes_[ idx ];

        // cannot go deeper if not refined
        if ( !node.isRefined() ) { //test: testCellQueries
            return std::nullopt;
        }

        idx = node.childIndex( choice );
    }

    const Node &node = nodes_[ idx ];

    // phantom nodes are not real cells
    if ( node.isPhantom() ) {
        return std::nullopt;
    }

    return CellView{ this, idx, index };
}

bool CellOctree::cellExists( MortonIndex index ) const
{
    return getCell( index ).has_value();
}

std::optional<CellOctree::CellView> CellOctree::getRootCell() const
{
    return getCell( MortonIndex{} );
}

CellOctree CellOctree::fromDescriptor(string_view descriptor)
{
    CellOctree tree;

    //clean default one root tree
    tree.nodes_.clear();
    tree.levelStarts_.clear();
    tree.levelCounts_.clear();

    //number of levels
    size_t pos = 0;

    //1-parsing
    while ( pos < descriptor.size() ) {
        size_t next = descriptor.find( '|', pos ); //find next level after the current position 

        string_view levelStr;
        bool isLastLevel = ( next == string_view::npos );

        if (isLastLevel) {
            levelStr = descriptor.substr( pos ); //extract till last position
        } else {
            levelStr = descriptor.substr( pos, next - pos ); //extract just till next-pos (next level)
        }

        tree.levelStarts_.push_back( tree.nodes_.size() );

        size_t count = 0;

        for ( char c : levelStr ) {
            bool refined = false;
            bool phantom = false;

            switch (c) {
            case '.': refined = false; phantom = false; break;
            case 'R': refined = true;  phantom = false; break;
            case 'P': refined = false; phantom = true;  break;
            case 'X': refined = true;  phantom = true;  break;
            default:
                throw invalid_argument( "invalid symbol in descriptor" );
            }

            tree.nodes_.emplace_back( refined, phantom );
            ++count;
        }

        tree.levelCounts_.push_back( count );

        if ( next == string_view::npos ) {
            break;
        }

        pos = next + 1;
    }

    //2)Assign children index
    const size_t levels = tree.numberOfLevels();

    for ( size_t level = 0; level + 1 < levels; ++level ) { //not processing last level bcs it has not childern 

        size_t start = tree.levelStarts_[ level ];
        size_t count = tree.levelCounts_[ level ];

        size_t childStart = tree.levelStarts_[ level + 1 ];
        size_t childIdx = childStart;

        for ( size_t i = 0; i < count; ++i ) {
            auto& node = tree.nodes_[ start + i ];

            if ( node.isRefined() ) {
                node.setChildrenStartIndex( childIdx );
                childIdx += 8;
            }
        }

        //imp for test case: testInvalidDescriptors
        size_t expectedChildPos = childStart + tree.levelCounts_[ level + 1  ];
        if ( childIdx != expectedChildPos ) {
            throw invalid_argument( "descriptor inconsistent" );
        }
    }

    return tree;
}

} // namespace oktal
