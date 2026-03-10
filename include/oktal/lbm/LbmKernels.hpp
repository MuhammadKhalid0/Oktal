#pragma once

#include "oktal/lbm/D3Q19.hpp"
#include "oktal/octree/CellGrid.hpp"

namespace oktal::lbm {

/**
 * @brief Computes the equilibrium distribution function f_i^eq(rho, u)
 *        for a given density, velocity, and lattice index.
 *
 * f_i^eq = w_i * rho * (1 + (c_i . u) / cs^2
 *          - (u . u) / (2 * cs^2)
 *          + (c_i . u)^2 / (2 * cs^4))
 */
inline double equilibrium(size_t q, double density, double ux, double uy,
                          double uz, double uSqr) {
  const auto cx = static_cast<double>(D3Q19::C[q][0]);
  const auto cy = static_cast<double>(D3Q19::C[q][1]);
  const auto cz = static_cast<double>(D3Q19::C[q][2]);

  const double ciDotU = cx * ux + cy * uy + cz * uz;

  return D3Q19::W[q] * density *
         (1.0 + 3.0 * ciDotU + 4.5 * ciDotU * ciDotU - 1.5 * uSqr);
}

/**
 * @brief Initialization kernel: sets the PDF field to its local equilibrium
 *        given the density and velocity fields.
 *
 *   f(x, 0) = f^eq(rho(x, 0), u(x, 0))
 */
class InitializePdfs {
public:
  void operator()(CellGrid::CellView cell, D3Q19LatticeView pdfs,
                  GridVectorView<const double, 1> rho,
                  GridVectorView<const double, 3> u) const {
    const size_t idx{cell};
    const double density = rho[idx];
    const double ux = u[idx, 0uz];
    const double uy = u[idx, 1uz];
    const double uz = u[idx, 2uz];
    const double uSqr = ux * ux + uy * uy + uz * uz;

    for (size_t q = 0; q < D3Q19::Q; ++q) {
      pdfs[idx, q] = equilibrium(q, density, ux, uy, uz, uSqr);
    }
  }
};

/**
 * @brief Computes macroscopic density and velocity from the PDF field.
 *
 *   rho(x, t) = sum_i f_i(x, t)
 *   u(x, t)   = (1 / rho) * sum_i c_i * f_i(x, t)
 */
class ComputeMacroscopicQuantities {
public:
  void operator()(CellGrid::CellView cell, D3Q19LatticeConstView pdfs,
                  GridVectorView<double, 1> rho,
                  GridVectorView<double, 3> u) const {
    const size_t idx{cell};

    double density = 0.0;
    double momX = 0.0;
    double momY = 0.0;
    double momZ = 0.0;

    for (size_t q = 0; q < D3Q19::Q; ++q) {
      const double fi = pdfs[idx, q];
      density += fi;
      momX += static_cast<double>(D3Q19::C[q][0]) * fi;
      momY += static_cast<double>(D3Q19::C[q][1]) * fi;
      momZ += static_cast<double>(D3Q19::C[q][2]) * fi;
    }

    rho[idx] = density;
    const double invRho = 1.0 / density;
    u[idx, 0uz] = momX * invRho;
    u[idx, 1uz] = momY * invRho;
    u[idx, 2uz] = momZ * invRho;
  }
};

/**
 * @brief Collision kernel using BGK approximation.
 *
 *   f*(x, t) = f(x, t) + omega * (f^eq(rho, u) - f(x, t))
 *
 * Overwrites the PDF field in-place with post-collision values.
 * Assumes that rho and u already contain the correct macroscopic quantities.
 */
class Collide {
public:
  explicit Collide(double omega) : omega_{omega} {}

  void operator()(CellGrid::CellView cell, D3Q19LatticeView pdfs,
                  GridVectorView<const double, 1> rho,
                  GridVectorView<const double, 3> u) const {
    const size_t idx{cell};
    const double density = rho[idx];
    const double ux = u[idx, 0uz];
    const double uy = u[idx, 1uz];
    const double uz = u[idx, 2uz];
    const double uSqr = ux * ux + uy * uy + uz * uz;

    for (size_t q = 0; q < D3Q19::Q; ++q) {
      const double feq = equilibrium(q, density, ux, uy, uz, uSqr);
      pdfs[idx, q] += omega_ * (feq - pdfs[idx, q]);
    }
  }

private:
  double omega_;
};

/**
 * @brief Streaming kernel: moves post-collision populations to
 *        neighboring cells according to their velocity directions.
 *
 *   f_i(x + c_i, t+1) = f_i*(x, t)
 *
 * Uses a two-array approach: reads from pdfsSrc, writes to pdfsDst.
 */
class Stream {
public:
  void operator()(CellGrid::CellView cell, D3Q19LatticeView pdfsDst,
                  D3Q19LatticeConstView pdfsSrc) const {
    const size_t srcIdx{cell};

    // Center velocity (i=0): particle stays in the same cell
    pdfsDst[srcIdx, 0uz] = pdfsSrc[srcIdx, 0uz];

    // Non-zero velocities (i=1..18): move to neighbor cell
    for (size_t q = 1; q < D3Q19::Q; ++q) {
      const auto &ci = D3Q19::C[q];
      const auto neighbor = cell.neighbor({ci[0], ci[1], ci[2]});
      const size_t dstIdx{neighbor};
      pdfsDst[dstIdx, q] = pdfsSrc[srcIdx, q];
    }
  }
};

}  // namespace oktal::lbm

