#pragma once

#include "oktal/octree/CellOctree.hpp"

#include <string>

namespace oktal {

// Placeholder interface for Task 3c (VTK export).
// The real implementation will go into src/io/VtkExport.cpp later.

inline void exportToVtk(const CellOctree & /*tree*/,
                        const std::string & /*filename*/) {
  // dummy implementation for now
}

} // namespace oktal
