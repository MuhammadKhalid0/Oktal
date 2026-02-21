#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <experimental/mdspan>
#include <memory>
#include <oktal/octree/CellGrid.hpp>
#include <utility>

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

  GridVector(const CellGrid& cells)
      : numCells_(cells.size()),
        data_(std::allocator<T>().allocate(cells.size() * Q)) {
    for (size_t i = 0; i < numCells_ * Q; ++i) {
      std::construct_at(data_ + i);
    }
  }

  ~GridVector() {
    if (data_) {
      for (size_t i = 0; i < numCells_ * Q; ++i) {
        std::destroy_at(data_ + i);
      }
      std::allocator<T>().deallocate(data_, numCells_ * Q);
    }
  }

  GridVector(const GridVector& other)
      : numCells_(other.numCells_),
        data_(std::allocator<T>().allocate(other.numCells_ * Q)) {
    for (size_t i = 0; i < numCells_ * Q; ++i) {
      std::construct_at(data_ + i, other.data_[i]);
    }
  }

  GridVector& operator=(const GridVector& other) {
    if (this == &other) {
      return *this;
    }
    const size_t n = other.numCells_ * Q;
    if (n <= numCells_ * Q) {
      for (size_t i = 0; i < n; ++i) {
        data_[i] = other.data_[i];
      }
      numCells_ = other.numCells_;
      return *this;
    }
    T* newData = std::allocator<T>().allocate(n);
    for (size_t i = 0; i < n; ++i) {
      std::construct_at(newData + i, other.data_[i]);
    }
    for (size_t i = 0; i < numCells_ * Q; ++i) {
      std::destroy_at(data_ + i);
    }
    std::allocator<T>().deallocate(data_, numCells_ * Q);
    data_ = newData;
    numCells_ = other.numCells_;
    return *this;
  }

  GridVector(GridVector&& other) noexcept
      : numCells_(other.numCells_),
        data_(other.data_) {
    other.numCells_ = 0;
    other.data_ = nullptr;
  }

  GridVector& operator=(GridVector&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    if (data_) {
      for (size_t i = 0; i < numCells_ * Q; ++i) {
        std::destroy_at(data_ + i);
      }
      std::allocator<T>().deallocate(data_, numCells_ * Q);
    }
    numCells_ = other.numCells_;
    data_ = other.data_;
    other.numCells_ = 0;
    other.data_ = nullptr;
    return *this;
  }

  [[nodiscard]] size_t allocSize() const { return numCells_ * Q; }

  [[nodiscard]] T* data() { return data_; }
  [[nodiscard]] const T* data() const { return data_; }

  [[nodiscard]] GridVectorView<T, Q> view() {
    if constexpr (Q == 1) {
      return GridVectorView<T, Q>(data_, numCells_);
    } else {
      std::experimental::extents<size_t, std::experimental::dynamic_extent, Q> ext(numCells_, Q);
      std::array<size_t, 2> strides{1, numCells_};
      std::experimental::layout_stride::mapping<decltype(ext)> map(ext, strides);
      return GridVectorView<T, Q>(data_, map);
    }
  }

  [[nodiscard]] GridVectorView<const T, Q> view() const {
    if constexpr (Q == 1) {
      return GridVectorView<const T, Q>(data_, numCells_);
    } else {
      std::experimental::extents<size_t, std::experimental::dynamic_extent, Q> ext(numCells_, Q);
      std::array<size_t, 2> strides{1, numCells_};
      std::experimental::layout_stride::mapping<decltype(ext)> map(ext, strides);
      return GridVectorView<const T, Q>(data_, map);
    }
  }

  [[nodiscard]] GridVectorView<const T, Q> const_view() const { return view(); }

  operator GridVectorView<T, Q>() { return view(); }
  operator GridVectorView<const T, Q>() const { return view(); }

  [[nodiscard]] decltype(auto) operator[](size_t cellIdx) const
    requires(Q == 1) {
    return data_[cellIdx];
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx)
    requires(Q == 1) {
    return data_[cellIdx];
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx, size_t compIdx) const
    requires(Q > 1) {
    return data_[compIdx * numCells_ + cellIdx];
  }
  [[nodiscard]] decltype(auto) operator[](size_t cellIdx, size_t compIdx)
    requires(Q > 1) {
    return data_[compIdx * numCells_ + cellIdx];
  }

private:
  size_t numCells_;
  T* data_;
};

}  // namespace oktal
