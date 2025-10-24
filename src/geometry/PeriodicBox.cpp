#include "oktal/geometry/PeriodicBox.hpp"

#include <algorithm>

namespace oktal {


std::array<double, 3>
PeriodicBox::mapIntoBox(std::array<double, 3> point) const {

  auto mapToInterval = [](double t, double lower, double upper) {
    const double intervalSize{upper - lower};
    const double tNormalized{(t - lower) / intervalSize};
    const double tInUnitInterval{tNormalized - std::floor(tNormalized)};
    return lower + tInUnitInterval * intervalSize;
  };

  return {
      mapToInterval(point[0], minCorner_[0], maxCorner_[0]),
      mapToInterval(point[1], minCorner_[1], maxCorner_[1]),
      mapToInterval(point[2], minCorner_[2], maxCorner_[2]),
  };
}

double PeriodicBox::sqrDistance(std::array<double, 3> pointA,
                                std::array<double, 3> pointB) {
  auto periodicDistance = [&, this](size_t coord) {
    if(!periodicity_[coord]){
      return pointA[coord] - pointB[coord];
    }

    double lower{ minCorner_[coord] };
    double upper{ maxCorner_[coord] };
    double p0{ pointA[coord] };
    double p1{ pointB[coord] };

    return std::min({std::abs(p0 - p1),
                     std::abs(p0 - (lower - (upper - p1))),
                     std::abs(p0 - (upper + (p1 - lower)))});
  };

  double dx{ periodicDistance(0) };
  double dy{ periodicDistance(1) };
  double dz{ periodicDistance(2) };

  return dx * dx + dy * dy + dz * dz;

}


} // namespace oktal
