#pragma once

#include "oktal/octree/CellOctree.hpp"
#include "oktal/octree/MortonIndex.hpp"
#include <array>
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

namespace oktal {

/**
 * @brief No periodicity
 */
struct NoPeriodicity {
  [[nodiscard]] Vec<long long, 3> operator()(Vec<long long, 3> coords,
                                             long long maxCoord) const {
    for (size_t d = 0; d < 3; ++d) {
      if (coords[d] < 0 || coords[d] >= maxCoord) {
        return {-1, -1, -1}; // Invalid marker
      }
    }
    return coords;
  }
};

/**
 * @brief Torus - opposite sides of the octree’s encompassing cube are glued
 * together.
 */
struct Torus {

  explicit Torus(std::array<bool, 3> periodic) : periodic(periodic) {}

  [[nodiscard]] Vec<long long, 3> operator()(Vec<long long, 3> coords,
                                             long long maxCoord) const {
    for (size_t d = 0; d < 3; ++d) {
      if (coords[d] < 0 || coords[d] >= maxCoord) {
        if (periodic.at(d)) {
          // Wrap around
          coords[d] = (coords[d] + maxCoord) % maxCoord;
          if (coords[d] < 0) {
            coords[d] += maxCoord;
          }
        } else {
          return {-1, -1, -1};
        }
      }
    }
    return coords;
  }

private:
  std::array<bool, 3> periodic;
};

using PeriodicityMapper =
    std::function<Vec<long long, 3>(Vec<long long, 3>, long long)>;

/**
 * @brief grid data structure which includes the enumeration of the grid's cells
 * and their neighborhood adjacency lists.
 */
class CellGrid {
public:
  class CellView;
  class Iterator;

  /**
   * @brief The value of NOT_ENUMERATED shall be the maximum value of the data
   * type size_t.
   */
  static constexpr size_t NOT_ENUMERATED = std::numeric_limits<size_t>::max();
  static constexpr size_t NO_NEIGHBOR = NOT_ENUMERATED;

  CellGrid(std::shared_ptr<const CellOctree> octree,
           std::vector<MortonIndex> mortonIndices = {});
  /**
   * @brief single constructor taking a shared pointer to a const CellOctree
   * returns an instance of the CellGridBuilder constructed
   */
  static class CellGridBuilder create(std::shared_ptr<const CellOctree> octree);
  /**
   * @brief Returns a const reference to the parent octree
   */
  [[nodiscard]] const CellOctree &octree() const;

  /**
   * @brief returns a const view onto the morton index array as an std::span.
   */
  [[nodiscard]] std::span<const MortonIndex> mortonIndices() const;

  /**
   * @brief In addition to listing the grid's cells, the CellGrid should be able
   * to translate a cell's stream index (from the parent octree) to its
   * enumeration index.
   */
  [[nodiscard]] size_t getEnumerationIndex(size_t streamIndex) const;
  [[nodiscard]] size_t
  getEnumerationIndex(const CellOctree::CellView &cell) const;

  [[nodiscard]] std::span<const size_t>
  neighborIndices(Vec<std::ptrdiff_t, 3> direction) const;

  [[nodiscard]] CellView operator[](size_t index) const;
  /**
   * @brief  define the public member function size() to make it a sized range
   */
  [[nodiscard]] size_t size() const;
  [[nodiscard]] Iterator begin() const;
  [[nodiscard]] Iterator end() const;

private:
  // data member to store its cell enumeration
  std::unordered_map<size_t, size_t> m_streamToEnumeration;
  // data member pointing at the grid's parent octre
  std::shared_ptr<const CellOctree> m_octree;
  // dynamically allocated array to store the cell's Morton indices
  std::vector<MortonIndex> m_mortonIndices;

  std::vector<Vec<std::ptrdiff_t, 3>> m_directions;
  std::vector<std::vector<size_t>> m_adjacencyLists;

  friend class CellGridBuilder;
};

class CellGrid::CellView {
public:
  CellView(const CellGrid *grid, size_t enumerationIndex)
      : m_grid(grid), m_enumerationIndex(enumerationIndex) {}

  [[nodiscard]] const MortonIndex &mortonIndex() const;

  [[nodiscard]] size_t enumerationIndex() const;

  [[nodiscard]] bool isValid() const;

  [[nodiscard]] operator size_t() const { return m_enumerationIndex; }

  [[nodiscard]] operator bool() const { return isValid(); }

  [[nodiscard]] size_t level() const;

  [[nodiscard]] Vec3D center() const;

  [[nodiscard]] Box<double> boundingBox() const;

  [[nodiscard]] CellView neighbor(Vec<std::ptrdiff_t, 3> direction) const;

private:
  const CellGrid *m_grid;
  size_t m_enumerationIndex;
};

class CellGrid::Iterator {
public:
  using value_type = CellView;
  using difference_type = std::ptrdiff_t;
  using pointer = CellView *;
  using reference = CellView;

  Iterator() : m_grid(nullptr), m_index(0) {}

  Iterator(const CellGrid *grid, size_t index) : m_grid(grid), m_index(index) {}

  [[nodiscard]] reference operator*() const;

  Iterator &operator++();

  Iterator operator++(int);

  [[nodiscard]] bool operator==(const Iterator &other) const;

  [[nodiscard]] bool operator!=(const Iterator &other) const;

private:
  const CellGrid *m_grid;
  size_t m_index;
};

class CellGridBuilder {
public:
  /**
   * single constructor taking a shared pointer to a const CellOctree
   */
  explicit CellGridBuilder(std::shared_ptr<const CellOctree> octree);

  CellGridBuilder &levels(std::span<const size_t> levels);
  CellGridBuilder &levels(std::initializer_list<size_t> levels);

  CellGridBuilder &
  neighborhood(std::span<const Vec<std::ptrdiff_t, 3>> directions);
  CellGridBuilder &
  neighborhood(std::initializer_list<Vec<std::ptrdiff_t, 3>> directions);

  template <typename PeriodicityMapperType>
  CellGridBuilder &periodicityMapper(const PeriodicityMapperType &mapper) {
    m_periodicityMapper = mapper;
    return *this;
  }

  CellGrid build();

private:
  [[nodiscard]] std::vector<size_t> determineLevelsToEnumerate() const;
  void enumerateCells(const std::vector<size_t> &levels, CellGrid &grid);
  void buildAdjacencyLists(CellGrid &grid);
  [[nodiscard]] size_t
  findNeighborIndex(const CellGrid &grid, const MortonIndex &mi,
                    const Vec<std::ptrdiff_t, 3> &offset) const;

  std::shared_ptr<const CellOctree> m_octree;
  std::vector<size_t> m_levels;
  bool m_levelsSet = false;

  std::vector<Vec<std::ptrdiff_t, 3>> m_neighborhood;
  PeriodicityMapper m_periodicityMapper = NoPeriodicity();
};

} // namespace oktal
