#pragma once

#include <array>
#include <cstddef>

#include "oktal/data/GridVector.hpp"
#include "oktal/geometry/Vec.hpp"

namespace oktal::lbm {

/**
 * @brief D3Q19 lattice stencil definitions for the Lattice Boltzmann Method.
 */
struct D3Q19 {
  /// Number of spatial dimensions
  static constexpr size_t D = 3;

  /// Number of discrete velocities
  static constexpr size_t Q = 19;

  /// Lattice speed of sound squared: cs^2 = 1/3
  static constexpr double CS_SQ = 1.0 / 3.0;

  /**
   * Discrete velocity vectors c_i for i = 0, ..., 18.
   *
   * Index 0:      center velocity (0,0,0)
   * Index 1-6:    cardinal directions
   * Index 7-18:   diagonal directions
   */
  static inline const std::array<Vec<std::ptrdiff_t, 3>, Q> C{{
      {0, 0, 0},    // 0  - C  (center)
      {1, 0, 0},    // 1  - W
      {-1, 0, 0},   // 2  - E
      {0, 1, 0},    // 3  - N
      {0, -1, 0},   // 4  - S
      {0, 0, 1},    // 5  - T
      {0, 0, -1},   // 6  - B
      {1, 1, 0},    // 7  - NW
      {-1, -1, 0},  // 8  - SE
      {1, -1, 0},   // 9  - SW
      {-1, 1, 0},   // 10 - NE
      {1, 0, 1},    // 11 - TW
      {-1, 0, -1},  // 12 - BE
      {1, 0, -1},   // 13 - BW
      {-1, 0, 1},   // 14 - TE
      {0, 1, 1},    // 15 - TN
      {0, -1, -1},  // 16 - BS
      {0, 1, -1},   // 17 - BN
      {0, -1, 1},   // 18 - TS
  }};

  /**
   * Lattice weights w_i for i = 0, ..., 18.
   *
   * w_0     = 1/3   (center)
   * w_1..6  = 1/18  (cardinal)
   * w_7..18 = 1/36  (diagonal)
   */
  static constexpr std::array<double, Q> W{{
      1.0 / 3.0,                                                   // center
      1.0 / 18.0, 1.0 / 18.0, 1.0 / 18.0, 1.0 / 18.0, 1.0 / 18.0, 1.0 / 18.0,  // cardinal
      1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0,           // diagonal xy
      1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0,           // diagonal xz
      1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0,           // diagonal yz
  }};

  /**
   * All non-zero velocity directions (indices 1..18), suitable for
   * passing to CellGridBuilder::neighborhood().
   */
  static inline const std::array<Vec<std::ptrdiff_t, 3>, Q - 1> CS_NO_CENTER{{
      C[1],  C[2],  C[3],  C[4],  C[5],  C[6],
      C[7],  C[8],  C[9],  C[10], C[11], C[12],
      C[13], C[14], C[15], C[16], C[17], C[18],
  }};
};

/// Type alias for the PDF field (19-component GridVector of doubles)
using D3Q19Lattice = GridVector<double, D3Q19::Q>;

/// Mutable view onto a D3Q19 PDF field
using D3Q19LatticeView = GridVectorView<double, D3Q19::Q>;

/// Read-only view onto a D3Q19 PDF field
using D3Q19LatticeConstView = GridVectorView<const double, D3Q19::Q>;

}  // namespace oktal::lbm

