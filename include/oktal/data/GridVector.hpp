#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <experimental/mdspan>
#include <oktal/octree/CellGrid.hpp>
#include <utility>
#include <vector>

namespace oktal {

template <typename T, size_t Q>
concept GridVectorViewElement =
    (Q > 0) && std::semiregular<std::remove_cv_t<T>> && !std::is_reference_v<T>;

template <typename T, size_t Q>
  requires GridVectorViewElement<T, Q>
using GridVectorView = std::conditional_t<
    (Q == 1),
    std::experimental::mdspan<T, std::experimental::extents<size_t, std::experimental::dynamic_extent>>,
    std::experimental::mdspan<T,
                              std::experimental::extents<size_t, std::experimental::dynamic_extent, Q>,
                              std::experimental::layout_stride>>;

template <std::semiregular T, size_t Q>
  requires(Q > 0)
class GridVector {
public:
  using value_type = T;
  static constexpr size_t NUM_COMPONENTS = Q;

  explicit GridVector(const CellGrid& cells)
      : numCells_(cells.size()),
        data_(numCells_ * Q) {}

  ~GridVector() = default;

  GridVector(const GridVector& other) = default;

  GridVector& operator=(const GridVector& other) {
    if (this == &other) {
      return *this;
    }
    const size_t n = other.data_.size();
    if (n <= data_.size()) {
      std::copy_n(other.data_.cbegin(), n, data_.begin());
      data_.resize(n);
    } else {
      data_ = other.data_;
    }
    numCells_ = other.numCells_;
    return *this;
  }

  GridVector(GridVector&& other) noexcept = default;

  GridVector& operator=(GridVector&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    data_ = std::move(other.data_);
    numCells_ = other.numCells_;
    other.numCells_ = 0;
    return *this;
  }

  [[nodiscard]] size_t allocSize() const { return data_.size(); }

  [[nodiscard]] T* data() { return data_.data(); }
  [[nodiscard]] const T* data() const { return data_.data(); }

  [[nodiscard]] GridVectorView<T, Q> view() {
    if constexpr (Q == 1) {
      return GridVectorView<T, Q>(data_.data(), numCells_);
    } else {
      using Extents = std::experimental::extents<size_t, std::experimental::dynamic_extent, Q>;
      const std::array<size_t, 2> strides{1, numCells_};
      const std::experimental::layout_stride::mapping<Extents> map(
          Extents(numCells_, Q), strides);
      return GridVectorView<T, Q>(data_.data(), map);
    }
  }

  [[nodiscard]] GridVectorView<const T, Q> view() const {
    if constexpr (Q == 1) {
      return GridVectorView<const T, Q>(data_.data(), numCells_);
    } else {
      using Extents = std::experimental::extents<size_t, std::experimental::dynamic_extent, Q>;
      const std::array<size_t, 2> strides{1, numCells_};
      const std::experimental::layout_stride::mapping<Extents> map(
          Extents(numCells_, Q), strides);
      return GridVectorView<const T, Q>(data_.data(), map);
    }
  }

  [[nodiscard]] GridVectorView<const T, Q> const_view() const { return view(); }

  operator GridVectorView<T, Q>() { return view(); }
  operator GridVectorView<const T, Q>() const { return view(); }

  [[nodiscard]] decltype(auto) operator[](size_t cellIdx) const
    requires(Q == 1) {
    return data_.at(cellIdx);
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx)
    requires(Q == 1) {
    return data_.at(cellIdx);
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx, size_t compIdx) const
    requires(Q > 1) {
    return data_.at(compIdx * numCells_ + cellIdx);
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx, size_t compIdx)
    requires(Q > 1) {
    return data_.at(compIdx * numCells_ + cellIdx);
  }

private:
  size_t numCells_;
  std::vector<T> data_;
};

}  // namespace oktal
