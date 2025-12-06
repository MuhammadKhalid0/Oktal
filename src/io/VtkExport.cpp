#include "oktal/io/VtkExport.hpp"

#include <cstdint>
#include <vector>
#include <span>

#include <advpt/htgfile/VtkHtgFile.hpp>

namespace oktal::io::vtk {

void exportOctree( const CellOctree &octree, const std::filesystem::path &filepath )
{
    using advpt::htgfile::HyperTree;
    using advpt::htgfile::SnapshotHtgFile;

    HyperTree ht{};

    // fill geometry (xCoords, yCoords, zCoords)
    const auto &geom   = octree.geometry();
    const auto  origin = geom.origin();      // Vec3D
    const double L     = geom.sidelength();

    ht.xCoords = { origin[0], origin[0] + L };
    ht.yCoords = { origin[1], origin[1] + L };
    ht.zCoords = { origin[2], origin[2] + L };

    // fill nodesPerDepth
    const std::size_t levels = octree.numberOfLevels();
    ht.nodesPerDepth.resize( levels );

    std::size_t totalNodes = 0;
    for( std::size_t level = 0; level < levels; ++level ) {
        const auto count = octree.numberOfNodes( level );
        ht.nodesPerDepth[ level ] = static_cast<std::int64_t>( count );
        totalNodes += count;
    }

    // build descriptor vector
    const std::size_t lastLevelCount = ( levels > 0 ) ? octree.numberOfNodes( levels - 1 ) : 0;

    const std::size_t descriptorBits  = totalNodes - lastLevelCount;
    const std::size_t descriptorBytes = ( descriptorBits + 7 ) / 8;
    ht.descriptor.assign( descriptorBytes, std::uint8_t{ 0 } );

    auto setDescriptorBit = [&]( std::size_t bitIndex, bool value ) {
        if( !value || descriptorBits == 0 ) {
            return;
        }
        const std::size_t byteIndex = bitIndex / 8;
        const int         bitInByte = 7 - static_cast<int>( bitIndex % 8 ); // MSB first
        ht.descriptor[ byteIndex ] |= static_cast<std::uint8_t>( 1u << bitInByte );
    };

    std::size_t bitIndex = 0;
    for( std::size_t level = 0; level + 1 < levels; ++level ) {
        const auto levelNodes = octree.nodesStream( level );
        for( const auto &node : levelNodes ) {
            setDescriptorBit( bitIndex, node.isRefined() );
            ++bitIndex;
        }
    }

    // build mask bit vector
    const std::size_t maskBits  = totalNodes;
    const std::size_t maskBytes = ( maskBits + 7 ) / 8;
    ht.mask.assign( maskBytes, std::uint8_t{ 0 } );

    auto setMaskBit = [&]( std::size_t bitIndex, bool value ) {
        if( !value || maskBits == 0 ) {
            return;
        }
        const std::size_t byteIndex = bitIndex / 8;
        const int         bitInByte = 7 - static_cast<int>( bitIndex % 8 ); // MSB first
        ht.mask[ byteIndex ] |= static_cast<std::uint8_t>( 1u << bitInByte );
    };

    bitIndex = 0;
    for( std::size_t level = 0; level < levels; ++level ) {
        const auto levelNodes = octree.nodesStream( level );
        for( const auto &node : levelNodes ) {
            const bool maskNode = node.isPhantom() && !node.isRefined();
            setMaskBit( bitIndex, maskNode );
            ++bitIndex;
        }
    }

    // snapshot file
    auto file = SnapshotHtgFile::create( filepath, ht );

    // add cell data array "level"
    std::vector<std::size_t> levelData;
    levelData.resize( totalNodes );

    std::size_t idx = 0;
    for( std::size_t level = 0; level < levels; ++level ) {
        const auto count = octree.numberOfNodes( level );
        for( std::size_t i = 0; i < count; ++i ) {
            levelData[ idx++ ] = level;
        }
    }

    file.writeCellData( "level",std::span<const std::size_t>( levelData.data(), levelData.size() ) );
}

} // namespace oktal::io::vtk
