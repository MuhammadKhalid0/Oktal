#include "oktal/io/VtkExport.hpp"
#include "oktal/lbm/D3Q19.hpp"
#include "oktal/lbm/LbmKernels.hpp"
#include "oktal/lbm/TaylorGreen.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char **argv) try {
  const std::span<char *> args(argv, static_cast<std::size_t>(argc));
  if (argc < 3) {
    std::cerr << std::format(
        "Usage: {} <refinement-level> <output-directory>\n", args[0]);
    return 1;
  }
  std::size_t refinementLevel{};
  std::string outputDirectory{};
  try {
    refinementLevel = std::stoull(args[1]);
    if (refinementLevel < 5) {
      std::cerr << std::format(
          "Invalid refinement value \"{}\", should be at least 5\n",
          refinementLevel);
      return 1;
    }
    outputDirectory = args[2];
  } catch (const std::exception &) {
    std::cerr << "Error: Invalid command line arguments\n";
    std::cerr << std::format(
        "Usage: {} <refinement-level> <output-directory>\n", args[0]);
    return 1;
  }

  {
    std::filesystem::path const outPath{outputDirectory};
    std::error_code ec;
    std::filesystem::create_directories(outPath, ec);
    if (ec && !std::filesystem::is_directory(outPath)) {
      std::cerr << std::format("Failed to create output directory \"{}\": {}\n",
                               outputDirectory, ec.message());
      return 1;
    }
  }

  const oktal::lbm::TaylorGreen taylorGreen{refinementLevel, 1.0 / 6.0};

  auto octree = oktal::CellOctree::createUniformGrid(taylorGreen.geometry(),
                                                     refinementLevel);

  auto cells = oktal::CellGrid::create(octree)
                   .levels({refinementLevel})
                   .neighborhood(oktal::lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(oktal::Torus({true, true, true}))
                   .build();

  oktal::lbm::D3Q19Lattice f{cells};
  oktal::lbm::D3Q19Lattice fTmp{cells};
  oktal::GridVector<double, 1> rho{cells};
  oktal::GridVector<double, 3> u{cells};
  oktal::GridVector<double, 3> uErr{cells};

  for (auto cell : cells) {
    const std::size_t idx{cell};
    const auto pos = cell.center();
    rho.view()[idx] = taylorGreen.rho(pos, 0.0);
    const auto vel = taylorGreen.u(pos, 0.0);
    u.view()[idx, 0uz] = vel[0];
    u.view()[idx, 1uz] = vel[1];
    u.view()[idx, 2uz] = vel[2];
  }

  oktal::lbm::InitializePdfs initPdfs;
  std::ranges::for_each(cells, [&](auto cell) {
    initPdfs(cell, f.view(), rho.const_view(), u.const_view());
  });

  oktal::lbm::ComputeMacroscopicQuantities computeMacro;
  oktal::lbm::Collide collide{taylorGreen.omega()};
  oktal::lbm::Stream stream;

  const std::size_t numTimesteps = taylorGreen.numberOfTimesteps();
  constexpr std::size_t reportInterval = 50;

  std::cout << std::format(
      "TGV simulation: refinement level {}, {} timesteps\n", refinementLevel,
      numTimesteps);

  const auto wallStart = std::chrono::steady_clock::now();

  // main loop
  for (std::size_t t = 0; t < numTimesteps; ++t) {
    std::ranges::for_each(cells, [&](auto cell) {
      computeMacro(cell, f.const_view(), rho.view(), u.view());
    });

    std::ranges::for_each(cells, [&](auto cell) {
      collide(cell, f.view(), rho.const_view(), u.const_view());
    });

    std::ranges::for_each(
        cells, [&](auto cell) { stream(cell, fTmp.view(), f.const_view()); });

    std::swap(f, fTmp);

    if ((t + 1) % reportInterval == 0 || t + 1 == numTimesteps) {
      const auto now = std::chrono::steady_clock::now();
      const double elapsed =
          std::chrono::duration<double>(now - wallStart).count();
      const double fraction = double(t + 1) / double(numTimesteps);
      const double remaining =
          (fraction > 0.0) ? elapsed * (1.0 - fraction) / fraction : 0.0;

      std::cout << std::format("Step {}/{} | Elapsed: {:.1f}s | ETA: {:.1f}s\n",
                               t + 1, numTimesteps, elapsed, remaining);

      const std::string vtkPath = std::format(
          "{}/step{}.vtkhdf", outputDirectory, (t + 1) / reportInterval);
      oktal::io::vtk::exportCellGrid(cells, vtkPath)
          .writeGridVector<double, 1>("rho", rho.const_view())
          .writeGridVector<double, 3>("u", u.const_view());
    }
  }

  const auto wallEnd = std::chrono::steady_clock::now();
  const double totalTime =
      std::chrono::duration<double>(wallEnd - wallStart).count();
  std::cout << std::format("Simulation complete in {:.2f}s\n", totalTime);

  std::ranges::for_each(cells, [&](auto cell) {
    computeMacro(cell, f.const_view(), rho.view(), u.view());
  });

  const double physicalTime = double(numTimesteps) * taylorGreen.dt();

  double sumErrSqX = 0.0;
  double sumErrSqY = 0.0;
  double sumRefSqX = 0.0;
  double sumRefSqY = 0.0;
  {
    auto uView = u.const_view();
    auto uErrView = uErr.view();
    for (auto cell : cells) {
      const std::size_t idx{cell};
      const auto pos = cell.center();
      const auto uTgv = taylorGreen.u(pos, physicalTime);

      const double ex = uView[idx, 0uz] - uTgv[0];
      const double ey = uView[idx, 1uz] - uTgv[1];
      const double ez = uView[idx, 2uz] - uTgv[2];

      uErrView[idx, 0uz] = ex;
      uErrView[idx, 1uz] = ey;
      uErrView[idx, 2uz] = ez;

      sumErrSqX += ex * ex;
      sumErrSqY += ey * ey;
      sumRefSqX += uTgv[0] * uTgv[0];
      sumRefSqY += uTgv[1] * uTgv[1];
    }
  }

  const double eX = std::sqrt(sumErrSqX / sumRefSqX);
  const double eY = std::sqrt(sumErrSqY / sumRefSqY);

  std::cout << std::format("Relative L2 errors: ex={:.6f}, ey={:.6f}\n", eX,
                           eY);

  // Write errors to file
  {
    std::ofstream errFile{outputDirectory + "/errors.txt"};
    errFile << std::format("{} {}\n", eX, eY);
  }

  // Final VTK export with error field
  oktal::io::vtk::exportCellGrid(cells, outputDirectory + "/final.vtkhdf")
      .writeGridVector<double, 1>("rho", rho.const_view())
      .writeGridVector<double, 3>("u", u.const_view())
      .writeGridVector<double, 3>("u_err", uErr.const_view());

  return 0;
} catch (const std::exception &e) {
  std::cerr << std::format("Fatal error: {}\n", e.what());
  return 1;
} catch (...) {
  std::cerr << "Fatal error: unknown exception\n";
  return 1;
}