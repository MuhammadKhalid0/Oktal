#pragma once

#include <filesystem>

#include "oktal/octree/CellOctree.hpp"

namespace oktal::io::vtk {

void exportOctree(
    const CellOctree &octree,
    const std::filesystem::path &filepath);

} // namespace oktal::io::vtk
