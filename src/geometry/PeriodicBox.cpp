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
      periodicity_[0] ? mapToInterval(point[0], minCorner_[0], maxCorner_[0])
                      : point[0],
      periodicity_[1] ? mapToInterval(point[1], minCorner_[1], maxCorner_[1])
                      : point[1],
      periodicity_[2] ? mapToInterval(point[2], minCorner_[2], maxCorner_[2])
                      : point[2],
  };
}

double PeriodicBox::sqrDistance(std::array<double, 3> pointA,
                                std::array<double, 3> pointB) const {
  auto periodicDistance = [&, this](size_t coord) {
    if (!periodicity_.at(coord)) {
      return pointA.at(coord) - pointB.at(coord);
    }

    double const lower{minCorner_.at(coord)};
    double const upper{maxCorner_.at(coord)};
    double const p0{pointA.at(coord)};
    double const p1{pointB.at(coord)};

    return std::min({std::abs(p0 - p1), std::abs(p0 - (lower - (upper - p1))),
                     std::abs(p0 - (upper + (p1 - lower)))});
  };

  double const dx{periodicDistance(0)};
  double const dy{periodicDistance(1)};
  double const dz{periodicDistance(2)};

  return dx * dx + dy * dy + dz * dz;
}

} // namespace oktal
