#pragma once

#include "oktal/geometry/Vec.hpp"

namespace oktal {

template <typename T = double>
  requires is_arithmetic_type<T>
class Box {
public:
    using vector_type = Vec<T,3>;

    Box():minCorner_(T{}),maxCorner_(T{}) {}

    Box(vector_type min, vector_type max):minCorner_(min),maxCorner_(max){}

    template <class Self>
    auto&& minCorner(this Self&& self) {
        return std::forward<Self>(self).minCorner_;
    }

    template <class Self>
    auto&& maxCorner(this Self&& self) {
        return std::forward<Self>(self).maxCorner_;
    }
    
    [[nodiscard]] constexpr vector_type extents() const{
      return maxCorner_ - minCorner_;
    }
      
    [[nodiscard]] constexpr vector_type center() const
      requires std::is_floating_point_v<T> {
      return (minCorner_ + maxCorner_) / T{2};
    }
    
    [[nodiscard]] constexpr T volume() const{
      auto e = extents();
      return e[0] * e[1] * e[2];
    }

private:
    vector_type minCorner_;
    vector_type maxCorner_;
};

} // namespace oktal