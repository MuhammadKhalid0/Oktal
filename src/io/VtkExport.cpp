#include <cstdint>
#include <span>
#include <vector>

#include <advpt/htgfile/VtkHtgFile.hpp>

#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellGrid.hpp"


namespace {

using advpt::htgfile::HyperTree;

// Set bit at bitIndex to 1 in a byte array (bit 0 = MSB of first byte, VTK convention).
void setBitMsbFirst(std::vector<std::uint8_t> &bytes, std::size_t bitIndex) {
  const std::size_t byteIndex = bitIndex / 8;
  const int bitInByte = 7 - static_cast<int>(bitIndex % 8);
  bytes[byteIndex] |= static_cast<std::uint8_t>(1u << bitInByte);
}

template <typename Predicate>
void setBitsFromNodes(std::vector<std::uint8_t> &bytes,
                      std::span<const oktal::CellOctree::Node> nodes,
                      Predicate pred) {
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    if (pred(nodes[i])) {
      setBitMsbFirst(bytes, i);
    }
  }
}

void setGeometry(HyperTree &ht, const oktal::CellOctree &octree) {
  const auto &geom = octree.geometry();
  const auto origin = geom.origin();
  const double L = geom.sidelength();
  ht.xCoords = {origin[0], origin[0] + L};
  ht.yCoords = {origin[1], origin[1] + L};
  ht.zCoords = {origin[2], origin[2] + L};
}

void setNodesPerDepth(HyperTree &ht, const oktal::CellOctree &octree) {
  const std::size_t levels = octree.numberOfLevels();
  if (levels == 0) {
    ht.nodesPerDepth.clear();
    return;
  }
  ht.nodesPerDepth.resize(levels);
  for (std::size_t level = 0; level < levels; ++level) {
    ht.nodesPerDepth[level] =
        static_cast<std::int64_t>(octree.numberOfNodes(level));
  }
}

void setDescriptor(HyperTree &ht, const oktal::CellOctree &octree) {
  const std::size_t levels = octree.numberOfLevels();
  if (levels == 0) {
    ht.descriptor.clear();
    return;
  }
  const std::size_t totalNodes = octree.numberOfNodes();
  const std::size_t lastLevelCount = octree.numberOfNodes(levels - 1);
  const std::size_t descriptorBits = totalNodes - lastLevelCount;
  if (descriptorBits == 0) {
    ht.descriptor.clear();
    return;
  }
  const std::size_t descriptorBytes = (descriptorBits + 7) / 8;
  ht.descriptor.assign(descriptorBytes, std::uint8_t{0});

  setBitsFromNodes(ht.descriptor, octree.nodesStream().first(descriptorBits),
                  [](const oktal::CellOctree::Node &n) { return n.isRefined(); });
}

void setMask(HyperTree &ht, const oktal::CellOctree &octree) {
  const std::size_t levels = octree.numberOfLevels();
  if (levels == 0) {
    ht.mask.clear();
    return;
  }
  const std::size_t totalNodes = octree.numberOfNodes();
  const std::size_t maskBytes = (totalNodes + 7) / 8;
  ht.mask.assign(maskBytes, std::uint8_t{0});

  setBitsFromNodes(ht.mask, octree.nodesStream(),
                  [](const oktal::CellOctree::Node &n) {
                    return n.isPhantom() && !n.isRefined();
                  });
}

HyperTree buildHyperTree(const oktal::CellOctree &octree) {
  HyperTree ht{};
  setGeometry(ht, octree);
  setNodesPerDepth(ht, octree);
  setDescriptor(ht, octree);
  setMask(ht, octree);
  return ht;
}

std::vector<std::size_t> buildLevelCellData(const oktal::CellOctree &octree) {
  std::vector<std::size_t> levelData;
  levelData.reserve(octree.numberOfNodes());
  for (std::size_t level = 0; level < octree.numberOfLevels(); ++level) {
    levelData.insert(levelData.end(), octree.numberOfNodes(level), level);
  }
  return levelData;
}

// Scatter GridVectorView (enumerated order) to stream order; one vector for Q==1, Q values per cell for Q>1.
template <typename T, size_t Q>
std::vector<T> scatterGridVectorToStreamOrder(const oktal::CellGrid &cells,
                                               oktal::GridVectorView<const T, Q> vector) {
  const std::size_t totalNodes = cells.octree().numberOfNodes();
  std::vector<T> out(totalNodes * Q, T{0});
  for (std::size_t streamIdx = 0; streamIdx < totalNodes; ++streamIdx) {
    const std::size_t enumIdx = cells.getEnumerationIndex(streamIdx);
    if (enumIdx != oktal::CellGrid::NOT_ENUMERATED) {
      if constexpr (Q == 1) {
        out[streamIdx] = vector[enumIdx];
      } else {
        for (size_t comp = 0; comp < Q; ++comp) {
          out[streamIdx * Q + comp] = vector[enumIdx, comp];
        }
      }
    }
  }
  return out;
}

}  // namespace

namespace oktal::io::vtk {

void exportOctree(const CellOctree &octree,
                  const std::filesystem::path &filepath) {
  using advpt::htgfile::SnapshotHtgFile;
  const advpt::htgfile::HyperTree ht = buildHyperTree(octree);
  auto file = SnapshotHtgFile::create(filepath, ht);
  std::vector<std::size_t> levelData = buildLevelCellData(octree);
  file.writeCellData(
      "level",
      std::span<const std::size_t>(levelData.data(), levelData.size()));
}

CellGridVtkExporter::CellGridVtkExporter(const CellGrid &cells,
                                         advpt::htgfile::SnapshotHtgFile &&file)
    : cells_(&cells), file_(std::move(file)) {}

CellGridVtkExporter exportCellGrid(const CellGrid &cells,
                                   const std::filesystem::path &filepath) {
  using advpt::htgfile::SnapshotHtgFile;
  const auto &octree = cells.octree();
  const advpt::htgfile::HyperTree ht = buildHyperTree(octree);
  auto file = SnapshotHtgFile::create(filepath, ht);
  std::vector<std::size_t> levelData = buildLevelCellData(octree);
  file.writeCellData(
      "level",
      std::span<const std::size_t>(levelData.data(), levelData.size()));
  return {cells, std::move(file)};
}

template <typename T>
CellGridVtkExporter &
CellGridVtkExporter::writeGridVector(const std::string &name,
                                     std::span<const T> gridVector) {
  const std::size_t totalNodes = cells_->octree().numberOfNodes();
  std::vector<T> fullVector(totalNodes, T{0});
  for (std::size_t streamIdx = 0; streamIdx < totalNodes; ++streamIdx) {
    const std::size_t enumIdx = cells_->getEnumerationIndex(streamIdx);
    if (enumIdx != CellGrid::NOT_ENUMERATED) {
      fullVector[streamIdx] = gridVector[enumIdx];
    }
  }
  file_.writeCellData(name,
                      std::span<const T>(fullVector.data(), fullVector.size()));
  return *this;
}

template <typename T, size_t Q>
CellGridVtkExporter &CellGridVtkExporter::writeGridVector(
    const std::string &id, GridVectorView<const T, Q> vector) {
  using advpt::htgfile::MdCellDataView;
  std::vector<T> fullVector = scatterGridVectorToStreamOrder<T, Q>(*cells_, vector);
  if constexpr (Q == 1) {
    file_.writeCellData(id,
                        std::span<const T>(fullVector.data(), fullVector.size()));
  } else {
    const MdCellDataView<T, 2> dataView(fullVector.data(), fullVector.size() / Q, Q);
    file_.writeCellData(id, dataView);
  }
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

// Explicit template instantiations for GridVectorView overload
template CellGridVtkExporter &
CellGridVtkExporter::writeGridVector<double, 1>(const std::string &,
                                                 GridVectorView<const double, 1>);
template CellGridVtkExporter &
CellGridVtkExporter::writeGridVector<double, 3>(const std::string &,
                                                 GridVectorView<const double, 3>);

} // namespace oktal::io::vtk
