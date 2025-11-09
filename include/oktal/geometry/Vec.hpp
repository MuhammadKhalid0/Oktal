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

  template <typename S, std::size_t S_DIM>
  Vec(Vec<S, S_DIM> const &v) noexcept
    requires std::convertible_to<S, T>
  {
    std::copy_n(v.begin(), DIM, v_.begin());
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return DIM; }

  constexpr T *data() noexcept { return v_.data(); }

  [[nodiscard]] constexpr const T *data() const noexcept { return v_.data(); }

  T &operator[](std::size_t index) { return v_.at(index); };

  T const &operator[](std::size_t index) const { return v_.at(index); };

  bool operator==(Vec const &rhs) const {
    if (size() != rhs.size()) {
      return false;
    }
    for (std::size_t ix = 0; ix < size(); ++ix) {
      if (v_.at(ix) != rhs[ix]) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(Vec const &rhs) const {
    if (size() == rhs.size()) {
      for (std::size_t ix = 0; ix < size(); ++ix) {
        if (v_.at(ix) != rhs[ix]) {
          return true;
        }
      }
      return false;
    }
    return true;
  }

  T *begin() { return v_.begin(); }
  [[nodiscard]] const T *begin() const { return v_.begin(); }

  T *end() { return v_.end(); }
  [[nodiscard]] const T *end() const { return v_.end(); }

  Vec<T, DIM> operator-() const {
    Vec<T, DIM> v3;

    for (std::size_t ix = 0; ix < size(); ++ix) {
      v3[ix] = -v_.at(ix);
    }
    return v3;
  }

  Vec<T, DIM> operator+(Vec const &v2) const {
    Vec<T, DIM> v3;
    if (v2.size() != size()) {
      return v3;
    }
    for (std::size_t ix = 0; ix < size(); ++ix) {
      v3[ix] = v_.at(ix) + v2[ix];
    }
    return v3;
  }

  Vec<T, DIM> &operator+=(Vec const &v2) {
    if (v2.size() != size()) {
      return *this;
    }
    std::size_t ix = 0;
    for (auto &e : v_) {
      e += v2[ix];
      ++ix;
    }
    return *this;
  }

  Vec<T, DIM> operator-(Vec const &v2) const
    requires(!std::is_unsigned_v<T>)
  {
    Vec<T, DIM> v3;
    if (v2.size() != size()) {
      return v3;
    }
    for (std::size_t ix = 0; ix < size(); ++ix) {
      v3[ix] = v_.at(ix) - v2[ix];
    }
    return v3;
  }

  Vec<T, DIM> &operator-=(Vec const &v2) {
    if (v2.size() != size()) {
      return *this;
    }
    std::size_t ix = 0;
    for (auto &e : v_) {
      e -= v2[ix];
      ++ix;
    }
    return *this;
  }

  Vec<T, DIM> operator*(T const &alpha) const {
    Vec<T, DIM> v3;
    for (std::size_t ix = 0; ix < size(); ++ix) {
      v3[ix] = v_.at(ix) * alpha;
    }
    return v3;
  }

  Vec<T, DIM> &operator*=(T const &alpha) {
    for (auto &e : v_) {
      e *= alpha;
    }
    return *this;
  }

  Vec<T, DIM> operator/(T const &alpha) const {
    Vec<T, DIM> v3;
    for (std::size_t ix = 0; ix < size(); ++ix) {
      v3[ix] = v_.at(ix) / alpha;
    }
    return v3;
  }

  Vec<T, DIM> &operator/=(T const &alpha) {
    for (auto &e : v_) {
      e /= alpha;
    }
    return *this;
  }

  [[nodiscard]] T sqrMagnitude() const {
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
};

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
inline Vec<T, DIM> operator*(Scalar const &alpha, Vec<T, DIM> const &v) {
  Vec<T, DIM> v3;
  for (std::size_t ix = 0; ix < v.size(); ++ix) {
    v3[ix] = v[ix] * alpha;
  }
  return v3;
}

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
inline Vec<T, DIM> operator*=(Scalar const &alpha, Vec<T, DIM> const &v) {
  return alpha * v;
}

template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
inline Vec<T, DIM> operator/(Scalar const &alpha, Vec<T, DIM> const &v) {
  Vec<T, DIM> v3;
  for (std::size_t ix = 0; ix < v.size(); ++ix) {
    v3[ix] = alpha / v[ix];
  }
  return v3;
}
template <typename Scalar, typename T, std::size_t DIM>
  requires std::convertible_to<Scalar, T>
inline Vec<T, DIM> operator/=(Scalar const &alpha, Vec<T, DIM> const &v) {
  return alpha / v;
}

using Vec3F = Vec<float, 3>;
using Vec3D = Vec<double, 3>;

} // namespace oktal