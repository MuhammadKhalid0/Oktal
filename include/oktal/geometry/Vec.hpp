#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

namespace oktal {

template <typename T>
concept is_arithmetic_type =
    std::is_integral_v<T> || std::is_floating_point_v<T>;

template <typename T, std::size_t DIM>
  requires is_arithmetic_type<T>
class Vec {
public:
  Vec() noexcept { v_.fill(0); };

  Vec(const T value) noexcept { v_.fill(value); }

  Vec(std::initializer_list<T> l) noexcept {
    const std::size_t to_copy = std::min(l.size(), DIM);
    std::copy_n(l.begin(), to_copy, v_.begin());
    if (to_copy < DIM) {
      std::fill(v_.begin() + to_copy, v_.end(), T{});
    }
  };

  template <typename S>
  constexpr Vec(Vec<S, DIM> const &other) noexcept
    requires std::convertible_to<S, T>
  {
    for (size_t i = 0; i < DIM; ++i)
    {
      v_[i] = static_cast<T>(other[i]);
    }
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return DIM; }

  [[nodiscard]] constexpr T *data() noexcept { return v_.data(); }

  [[nodiscard]] constexpr const T *data() const noexcept { return v_.data(); }

  [[nodiscard]] T &operator[](std::size_t index) { return v_.at(index); };

  [[nodiscard]] T const &operator[](std::size_t index) const { return v_.at(index); };

  [[nodiscard]] constexpr T *begin() noexcept { return data(); }
  [[nodiscard]] constexpr const T *begin() const noexcept { return data(); }

  [[nodiscard]] constexpr T *end() { return data() + DIM; }
  [[nodiscard]] constexpr const T *end() const { return data() + DIM; }

  constexpr Vec<T, DIM> operator-() const
  requires (!std::is_unsigned_v<T>)
  {
    Vec<T, DIM> result{};
    for (std::size_t ix = 0; ix < size(); ++ix) {
      result[ix] = -v_[ix];
    }
    return result;
  }

  Vec<T, DIM> &operator+=(Vec const &other) {
    for (size_t i = 0; i < DIM; ++i)
    {
      v_[i] += other[i];
    }
    return *this;
  }

  Vec<T, DIM> &operator-=(Vec const &other) {
    for (size_t i = 0; i < DIM; ++i)
    {
      v_[i] -= other[i];
    }
    return *this;
  }

  Vec<T, DIM> &operator*=(T const &alpha) {
    for (auto &e : v_) {
      e *= alpha;
    }
    return *this;
  }

  Vec<T, DIM> &operator/=(T const &alpha) {
    for (auto &e : v_) {
      e /= alpha;
    }
    return *this;
  }

  [[nodiscard]] constexpr T sqrMagnitude() const noexcept {
    T res = 0;
    for (auto elem : v_) {
      res += elem * elem;
    }
    return res;
  }

  [[nodiscard]] T magnitude() const
    requires std::is_floating_point_v<T>
  {
    return std::sqrt(sqrMagnitude());
  }

private:
  std::array<T, DIM> v_;
  
  [[nodiscard]] friend constexpr bool operator==(Vec const& lhs, Vec const& rhs) noexcept {
    return lhs.v_ == rhs.v_;
  }

  [[nodiscard]] friend constexpr bool operator!=(Vec const& lhs, Vec const& rhs) noexcept {
    return !(lhs == rhs);
  }

  [[nodiscard]] friend constexpr Vec operator+(Vec const& lhs, Vec const& rhs) noexcept {
    Vec result{};
    for (std::size_t ix = 0; ix < lhs.size(); ++ix) {
      result[ix] = lhs[ix] + rhs[ix];
    }
    return result;
  }

  // if T is unsigned values might overflow. Maybe useful for periodics later on?
  [[nodiscard]] friend constexpr Vec operator-(Vec const& lhs, Vec const &rhs) noexcept {
    Vec result{};
    for (std::size_t ix = 0; ix < lhs.size(); ++ix) {
      result[ix] = lhs[ix] - rhs[ix];
    }
    return result;
  }

};

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
[[nodiscard]] constexpr Vec<T, DIM> operator*(Vec<T, DIM> const& v, Scalar const& alpha) noexcept {
  Vec<T, DIM> result{};
  for (std::size_t ix = 0; ix < DIM; ++ix) {
    result[ix] = v[ix] * alpha;
  }
  return result;
}

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
[[nodiscard]] constexpr Vec<T, DIM> operator*(Scalar const& alpha, Vec<T, DIM> const& v) noexcept {
  return v * alpha;
}

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
[[nodiscard]] constexpr Vec<T, DIM> operator/(Vec<T, DIM> const &v, Scalar const &alpha) {
  Vec<T, DIM> result{}; 
  for (std::size_t ix = 0; ix < DIM; ++ix) { 
    result[ix] = v[ix] / alpha; 
  } 
  return result; 
}

using Vec3F = Vec<float, 3>;
using Vec3D = Vec<double, 3>;

} // namespace oktal