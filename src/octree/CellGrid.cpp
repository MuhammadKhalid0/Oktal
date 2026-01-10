#include "oktal/octree/CellGrid.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace oktal {

CellGrid::CellGrid(std::shared_ptr<const CellOctree> octree,
                   std::vector<MortonIndex> mortonIndices)
    : m_octree(std::move(octree)), m_mortonIndices(std::move(mortonIndices)) {}

const CellOctree &CellGrid::octree() const { return *m_octree; }

std::span<const MortonIndex> CellGrid::mortonIndices() const {
  return m_mortonIndices;
}

size_t CellGrid::getEnumerationIndex(size_t streamIndex) const {
  if (auto it = m_streamToEnumeration.find(streamIndex);
      it != m_streamToEnumeration.end()) {
    return it->second;
  }
  return NOT_ENUMERATED;
}

size_t CellGrid::getEnumerationIndex(const CellOctree::CellView &cell) const {
  return getEnumerationIndex(cell.streamIndex());
}

std::span<const size_t>
CellGrid::neighborIndices(Vec<std::ptrdiff_t, 3> direction) const {
  const auto it = std::ranges::find(m_directions, direction);
  if (it == m_directions.end()) {
    throw std::out_of_range("Direction not found in neighborhood");
  }
  const auto index =
      static_cast<size_t>(std::distance(m_directions.begin(), it));
  return m_adjacencyLists[index];
}

CellGrid::CellView CellGrid::operator[](size_t index) const {
  return {this, index};
}

size_t CellGrid::size() const { return m_mortonIndices.size(); }

CellGrid::Iterator CellGrid::begin() const { return {this, 0}; }

CellGrid::Iterator CellGrid::end() const {
  return {this, m_mortonIndices.size()};
}

CellGridBuilder CellGrid::create(std::shared_ptr<const CellOctree> octree) {
  return CellGridBuilder(std::move(octree));
}

const MortonIndex &CellGrid::CellView::mortonIndex() const {
  return m_grid->m_mortonIndices[m_enumerationIndex];
}

size_t CellGrid::CellView::enumerationIndex() const {
  return m_enumerationIndex;
}

bool CellGrid::CellView::isValid() const {
  return m_enumerationIndex != CellGrid::NO_NEIGHBOR;
}

size_t CellGrid::CellView::level() const { return mortonIndex().level(); }
Vec3D CellGrid::CellView::center() const {
  auto cellOpt = m_grid->m_octree->getCell(mortonIndex());
  if (!cellOpt) {
    return Vec3D{0.0, 0.0, 0.0};
  }
  return cellOpt->center();
}

Box<double> CellGrid::CellView::boundingBox() const {
  auto cellOpt = m_grid->m_octree->getCell(mortonIndex());
  if (!cellOpt) {
    return Box<double>{};
  }
  return cellOpt->boundingBox();
}

CellGrid::CellView
CellGrid::CellView::neighbor(Vec<std::ptrdiff_t, 3> direction) const {
  auto neighbors = m_grid->neighborIndices(direction);
  size_t const neighborIdx = neighbors[m_enumerationIndex];
  return {m_grid, neighborIdx};
}

CellGrid::Iterator::reference CellGrid::Iterator::operator*() const {
  return {m_grid, m_index};
}

CellGrid::Iterator &CellGrid::Iterator::operator++() {
  ++m_index;
  return *this;
}

CellGrid::Iterator CellGrid::Iterator::operator++(int) {
  Iterator tmp = *this;
  ++(*this);
  return tmp;
}

bool CellGrid::Iterator::operator==(const Iterator &other) const {
  return m_index == other.m_index;
}

bool CellGrid::Iterator::operator!=(const Iterator &other) const {
  return !(*this == other);
}

CellGridBuilder::CellGridBuilder(std::shared_ptr<const CellOctree> octree)
    : m_octree(std::move(octree)) {}

CellGridBuilder &CellGridBuilder::levels(std::span<const size_t> levels) {
  m_levels.assign(levels.begin(), levels.end());
  m_levelsSet = true;
  return *this;
}

CellGridBuilder &CellGridBuilder::levels(std::initializer_list<size_t> levels) {
  m_levels.assign(levels.begin(), levels.end());
  m_levelsSet = true;
  return *this;
}

CellGridBuilder &CellGridBuilder::neighborhood(
    std::span<const Vec<std::ptrdiff_t, 3>> directions) {
  m_neighborhood.assign(directions.begin(), directions.end());
  return *this;
}

CellGridBuilder &CellGridBuilder::neighborhood(
    std::initializer_list<Vec<std::ptrdiff_t, 3>> directions) {
  m_neighborhood.assign(directions.begin(), directions.end());
  return *this;
}

std::vector<size_t> CellGridBuilder::determineLevelsToEnumerate() const {
  if (m_levelsSet) {
    return m_levels;
  }

  size_t const numLevels = m_octree->numberOfLevels();
  std::vector<size_t> levels;
  levels.reserve(numLevels);
  for (size_t i = 0; i < numLevels; ++i) {
    levels.push_back(i);
  }
  return levels;
}

void CellGridBuilder::enumerateCells(const std::vector<size_t> &levels,
                                     CellGrid &grid) {
  for (size_t const level : levels) {
    auto range = m_octree->horizontalRange(level);
    for (const auto &cell : range) {
      grid.m_mortonIndices.push_back(cell.mortonIndex());
      grid.m_streamToEnumeration.emplace(cell.streamIndex(),
                                         grid.m_mortonIndices.size() - 1);
    }
  }
}

size_t
CellGridBuilder::findNeighborIndex(const CellGrid &grid, const MortonIndex &mi,
                                   const Vec<std::ptrdiff_t, 3> &offset) const {
  size_t const level = mi.level();
  Vec<size_t, 3> const coords = mi.gridCoordinates();
  long long const maxCoord = 1LL << level;

  Vec<long long, 3> nextCoords{};
  for (size_t d = 0; d < 3; ++d) {
    nextCoords[d] = static_cast<long long>(coords[d]) + offset[d];
  }

  // Apply periodicity mapping
  nextCoords = m_periodicityMapper(nextCoords, maxCoord);

  // Check if mapping returned invalid marker
  if (nextCoords[0] < 0 || nextCoords[1] < 0 || nextCoords[2] < 0) {
    return CellGrid::NO_NEIGHBOR;
  }

  MortonIndex const nextMI = MortonIndex::fromGridCoordinates(
      level,
      {static_cast<size_t>(nextCoords[0]), static_cast<size_t>(nextCoords[1]),
       static_cast<size_t>(nextCoords[2])});

  if (auto const cellOpt = m_octree->getCell(nextMI); cellOpt) {
    return grid.getEnumerationIndex(cellOpt->streamIndex());
  }

  return CellGrid::NO_NEIGHBOR;
}

void CellGridBuilder::buildAdjacencyLists(CellGrid &grid) {
  if (m_neighborhood.empty()) {
    return;
  }

  grid.m_directions = m_neighborhood;
  grid.m_adjacencyLists.resize(m_neighborhood.size());

  for (size_t i = 0; i < m_neighborhood.size(); ++i) {
    grid.m_adjacencyLists[i].reserve(grid.m_mortonIndices.size());

    for (const auto &mi : grid.m_mortonIndices) {
      grid.m_adjacencyLists[i].push_back(
          findNeighborIndex(grid, mi, m_neighborhood[i]));
    }
  }
}

CellGrid CellGridBuilder::build() {
  CellGrid grid(m_octree);
  enumerateCells(determineLevelsToEnumerate(), grid);
  buildAdjacencyLists(grid);

  return grid;
}

} // namespace oktal