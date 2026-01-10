#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double PI = std::numbers::pi;

double sourceFunction(double x, double y, double z) {
  return 3.0 * PI * PI * std::cos(PI * x) * std::cos(PI * y) * std::cos(PI * z);
}

double boundaryCondition(double x, double y, double z) {
  return std::cos(PI * x) * std::cos(PI * y) * std::cos(PI * z);
}

double computeL2Norm(const std::vector<double> &residual, std::size_t ncells) {
  double sum = 0.0;
  for (const auto &r : residual) {
    sum += r * r;
  }
  return std::sqrt(sum / static_cast<double>(ncells));
}

} // namespace

int main(int argc, char **argv) {
  try {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const std::span<char *> args(argv, static_cast<std::size_t>(argc));
    
    if (argc != 5) {
      std::cerr << std::format("Usage: {} <refinementLevel> <max-iterations> "
                               "<epsilon> <output-file>\n",
                               args[0]);
      return 1;
    }

    std::size_t refinementLevel{};
    std::size_t maxIterations{};
    double epsilon{};

    try {
      refinementLevel = std::stoull(args[1]);
      maxIterations = std::stoull(args[2]);
      epsilon = std::stod(args[3]);
    } catch (const std::exception &) {
      std::cerr << "Error: Invalid command line arguments\n";
      std::cerr << std::format("Usage: {} <refinementLevel> <max-iterations> "
                               "<epsilon> <output-file>\n",
                               args[0]);
      return 1;
    }

    const std::filesystem::path outputFile{args[4]};

    auto octree = oktal::CellOctree::createUniformGrid(refinementLevel);

    auto cells = oktal::CellGrid::create(octree)
                     .neighborhood({{-1, 0, 0},
                                    {1, 0, 0},
                                    {0, -1, 0},
                                    {0, 1, 0},
                                    {0, 0, -1},
                                    {0, 0, 1}})
                     .build();

    const std::size_t ncells = cells.size();
    const double h = octree->geometry().dx(refinementLevel);
    const double h2 = h * h;

    std::vector<double> u(ncells, 0.0);
    std::vector<double> u_tmp(ncells, 0.0);
    std::vector<double> f(ncells);
    std::vector<double> residual(ncells);

    // Initialize source term
    for (auto cell : cells) {
      const auto center = cell.center();
      f[cell] = sourceFunction(center[0], center[1], center[2]);
    }

    std::size_t iteration = 0;
    double residualNorm = 0.0;

    while (iteration < maxIterations) {
      // Jacobi update step
      for (auto cell : cells) {
        auto nW = cell.neighbor({-1, 0, 0});
        auto nE = cell.neighbor({1, 0, 0});
        auto nS = cell.neighbor({0, -1, 0});
        auto nN = cell.neighbor({0, 1, 0});
        auto nB = cell.neighbor({0, 0, -1});
        auto nT = cell.neighbor({0, 0, 1});

        if (nW && nE && nS && nN && nB && nT) {
          // Interior: apply stencil
          u_tmp[cell] = (1.0 / 6.0) *
                        (h2 * f[cell] + u[nW] + u[nE] + u[nS] + u[nN] + u[nB] + u[nT]);
        } else {
          // Boundary: apply BC
          const auto c = cell.center();
          u_tmp[cell] = boundaryCondition(c[0], c[1], c[2]);
        }
      }

      std::swap(u, u_tmp);

      // Compute residual
      for (auto cell : cells) {
        auto nW = cell.neighbor({-1, 0, 0});
        auto nE = cell.neighbor({1, 0, 0});
        auto nS = cell.neighbor({0, -1, 0});
        auto nN = cell.neighbor({0, 1, 0});
        auto nB = cell.neighbor({0, 0, -1});
        auto nT = cell.neighbor({0, 0, 1});

        if (nW && nE && nS && nN && nB && nT) {
          const double laplacian = (1.0 / h2) *
              (-6.0 * u[cell] + u[nW] + u[nE] + u[nS] + u[nN] + u[nB] + u[nT]);
          residual[cell] = f[cell] + laplacian;
        } else {
          residual[cell] = 0.0;
        }
      }

      residualNorm = computeL2Norm(residual, ncells);
      iteration++;

      if (residualNorm < epsilon) {
        break;
      }
    }

    std::cout << std::format("Converged after {} iterations\n", iteration);
    std::cout << std::format("Final residual L2 norm: {:.6e}\n", residualNorm);

    oktal::io::vtk::exportCellGrid(cells, outputFile)
        .writeGridVector<double>("u", u)
        .writeGridVector<double>("f", f)
        .writeGridVector<double>("residual", residual);

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}

