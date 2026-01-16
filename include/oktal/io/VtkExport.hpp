#pragma once

#include <filesystem>
#include <span>
#include <string>

#include <advpt/htgfile/VtkHtgFile.hpp>

#include "oktal/octree/CellOctree.hpp"

namespace oktal {
class CellGrid;
}

namespace oktal::io::vtk {

void exportOctree(const CellOctree &octree,
                  const std::filesystem::path &filepath);

// Helper class for chaining multiple grid vector exports
class CellGridVtkExporter {
public:
  template <typename T>
  CellGridVtkExporter &writeGridVector(const std::string &name,
                                       std::span<const T> gridVector);

private:
  friend CellGridVtkExporter exportCellGrid(const CellGrid &cells,
                                            const std::filesystem::path &filepath);
  CellGridVtkExporter(const CellGrid &cells,
                      advpt::htgfile::SnapshotHtgFile &&file);

  const CellGrid *cells_;
  advpt::htgfile::SnapshotHtgFile file_;
};

CellGridVtkExporter exportCellGrid(const CellGrid &cells,
                                   const std::filesystem::path &filepath);

} // namespace oktal::io::vtk
