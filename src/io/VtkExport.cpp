#include "oktal/io/VtkExport.hpp"

#include "oktal/octree/CellGrid.hpp"

#include <cstdint>
#include <span>
#include <vector>

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

CellGridVtkExporter::CellGridVtkExporter(const CellGrid &cells,
                                         advpt::htgfile::SnapshotHtgFile &&file)
    : cells_(cells), file_(std::move(file)) {}

CellGridVtkExporter exportCellGrid(const CellGrid &cells,
                                   const std::filesystem::path &filepath) {
  using advpt::htgfile::HyperTree;
  using advpt::htgfile::SnapshotHtgFile;

  const auto &octree = cells.octree();
  const auto &geom = octree.geometry();
  const auto origin = geom.origin();
  const double L = geom.sidelength();

  HyperTree ht{};
  ht.xCoords = {origin[0], origin[0] + L};
  ht.yCoords = {origin[1], origin[1] + L};
  ht.zCoords = {origin[2], origin[2] + L};

  const std::size_t levels = octree.numberOfLevels();
  ht.nodesPerDepth.resize(levels);

  std::size_t totalNodes = 0;
  for (std::size_t level = 0; level < levels; ++level) {
    const auto count = octree.numberOfNodes(level);
    ht.nodesPerDepth[level] = static_cast<std::int64_t>(count);
    totalNodes += count;
  }

  const std::size_t lastLevelCount =
      (levels > 0) ? octree.numberOfNodes(levels - 1) : 0;
  const std::size_t descriptorBits = totalNodes - lastLevelCount;
  const std::size_t descriptorBytes = (descriptorBits + 7) / 8;
  ht.descriptor.assign(descriptorBytes, std::uint8_t{0});

  auto setDescriptorBit = [&](std::size_t bitIndex, bool value) {
    if (!value || descriptorBits == 0)
      return;
    const std::size_t byteIndex = bitIndex / 8;
    const int bitInByte = 7 - static_cast<int>(bitIndex % 8);
    ht.descriptor[byteIndex] |= static_cast<std::uint8_t>(1u << bitInByte);
  };

  std::size_t bitIndex = 0;
  for (std::size_t level = 0; level + 1 < levels; ++level) {
    const auto levelNodes = octree.nodesStream(level);
    for (const auto &node : levelNodes) {
      setDescriptorBit(bitIndex, node.isRefined());
      ++bitIndex;
    }
  }

  const std::size_t maskBits = totalNodes;
  const std::size_t maskBytes = (maskBits + 7) / 8;
  ht.mask.assign(maskBytes, std::uint8_t{0});

  auto setMaskBit = [&](std::size_t bitIndex, bool value) {
    if (!value || maskBits == 0)
      return;
    const std::size_t byteIndex = bitIndex / 8;
    const int bitInByte = 7 - static_cast<int>(bitIndex % 8);
    ht.mask[byteIndex] |= static_cast<std::uint8_t>(1u << bitInByte);
  };

  bitIndex = 0;
  for (std::size_t level = 0; level < levels; ++level) {
    const auto levelNodes = octree.nodesStream(level);
    for (const auto &node : levelNodes) {
      const bool maskNode = node.isPhantom() && !node.isRefined();
      setMaskBit(bitIndex, maskNode);
      ++bitIndex;
    }
  }

  auto file = SnapshotHtgFile::create(filepath, ht);

  std::vector<std::size_t> levelData(totalNodes);
  std::size_t idx = 0;
  for (std::size_t level = 0; level < levels; ++level) {
    const auto count = octree.numberOfNodes(level);
    for (std::size_t i = 0; i < count; ++i) {
      levelData[idx++] = level;
    }
  }

  file.writeCellData(
      "level", std::span<const std::size_t>(levelData.data(), levelData.size()));

  return CellGridVtkExporter(cells, std::move(file));
}

template <typename T>
CellGridVtkExporter &
CellGridVtkExporter::writeGridVector(const std::string &name,
                                     std::span<const T> gridVector) {
  const auto &octree = cells_.octree();
  const std::size_t totalNodes = octree.numberOfNodes();

  // VTK expects data for all nodes, but grid vectors only contain enumerated cells
  std::vector<T> fullVector(totalNodes, T{0});

  for (std::size_t enumIdx = 0; enumIdx < cells_.size(); ++enumIdx) {
    const auto &mortonIndex = cells_.mortonIndices()[enumIdx];
    auto cellOpt = octree.getCell(mortonIndex);
    if (cellOpt.has_value()) {
      fullVector[cellOpt->streamIndex()] = gridVector[enumIdx];
    }
  }

  file_.writeCellData(name, std::span<const T>(fullVector.data(), fullVector.size()));
  return *this;
}

// Explicit template instantiations for common types
template CellGridVtkExporter &
CellGridVtkExporter::writeGridVector<int32_t>(const std::string &,
                                              std::span<const int32_t>);
template CellGridVtkExporter &
CellGridVtkExporter::writeGridVector<float>(const std::string &,
                                            std::span<const float>);
template CellGridVtkExporter &
CellGridVtkExporter::writeGridVector<double>(const std::string &,
                                             std::span<const double>);

} // namespace oktal::io::vtk
