#include <algorithm>
#include <string>

#include "advpt/testing/Testutils.hpp"

#include "oktal/data/GridVector.hpp"
#include "oktal/lbm/D3Q19.hpp"
#include "oktal/lbm/LbmKernels.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <highfive/highfive.hpp>

#define TEST_INITIALIZATION true
#define TEST_MACROSCOPIC_QUANTITIES true
#define TEST_COLLISION true
#define TEST_STREAMING true


namespace {
using namespace oktal;

constexpr std::string_view DATAFILE{DATAFILE_LOCATION};

void testInitialization1() {
#if TEST_INITIALIZATION

  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};
  lbm::D3Q19Lattice pdfsExpected{cells};

  GridVector<double, 1> rho{cells};
  GridVector<double, 3> u{cells};

  auto rhoView = rho.view();
  auto uView = u.view();

  for (auto [i, cell] : std::views::enumerate(cells)) {
    const size_t cellIdx{cell};
    rhoView[cellIdx] = 1.0 + double(i) / 100.;
    uView[cellIdx, 0uz] = uView[cellIdx, 1uz] = uView[cellIdx, 2uz] = 0.;

    for (auto q : std::views::iota(0uz, lbm::D3Q19::Q)) {
      pdfsExpected[cellIdx, q] = rhoView[cellIdx] * lbm::D3Q19::W[q];
    }
  }

  lbm::InitializePdfs initPdfs;
  std::ranges::for_each(cells,
                        [&](auto cell) { initPdfs(cell, pdfs.view(), rho.const_view(), u.const_view()); });

  advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
      std::span(pdfs.data(), pdfs.allocSize()),
      std::span(pdfsExpected.data(), pdfsExpected.allocSize()));

#else
  advpt::testing::dont_compile();
#endif
}

void testInitialization2() {
#if TEST_INITIALIZATION
  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};
  lbm::D3Q19Lattice pdfsExpected{cells};

  GridVector<double, 1> rho{cells};
  GridVector<double, 3> u{cells};

  const HighFive::File datafile{std::filesystem::path(DATAFILE),
                          HighFive::File::ReadOnly};
  const size_t numChunks{datafile.getAttribute("numChunks").read<size_t>()};

  auto rhoDset = datafile.getDataSet("rho");
  auto uDset = datafile.getDataSet("u");
  auto equilibriumPdfsDset = datafile.getDataSet("equilibriumPdfs");

  for (auto chunkIdx : std::views::iota(0uz, numChunks)) {
    rhoDset.select({chunkIdx, 0uz}, {1uz, 8uz}).read_raw(rho.data());
    uDset.select({chunkIdx, 0uz, 0uz}, {1uz, 3uz, 8uz}).read_raw(u.data());
    equilibriumPdfsDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfsExpected.data());

    lbm::InitializePdfs initPdfs;
    std::ranges::for_each(cells,
                          [&](auto cell) { initPdfs(cell, pdfs.view(), rho.const_view(), u.const_view()); });

    advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
        std::span(pdfs.data(), pdfs.allocSize()),
        std::span(pdfsExpected.data(), pdfsExpected.allocSize()));
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testMacroscopicQuantities1() {
#if TEST_MACROSCOPIC_QUANTITIES
  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};

  GridVector<double, 1> rho{cells};
  GridVector<double, 3> u{cells};

  GridVector<double, 1> rhoExpected{cells};
  GridVector<double, 3> uExpected{cells};

  auto rhoView = rhoExpected.view();
  auto uView = uExpected.view();

  for (auto [i, cell] : std::views::enumerate(cells)) {
    const size_t cellIdx{cell};
    const double rho{1.0 + double(i) / 100.};

    rhoView[cellIdx] = rho;
    uView[cellIdx, 0uz] = uView[cellIdx, 1uz] = uView[cellIdx, 2uz] = 0.;

    for (auto q : std::views::iota(0uz, lbm::D3Q19::Q)) {
      pdfs[cellIdx, q] = rho * lbm::D3Q19::W[q];
    }
  }

  lbm::ComputeMacroscopicQuantities computeMQs;
  std::ranges::for_each(cells,
                        [&](auto cell) { computeMQs(cell, pdfs.const_view(), rho.view(), u.view()); });

  advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
      std::span(rho.data(), rho.allocSize()),
      std::span(rhoExpected.data(), rhoExpected.allocSize()));

  advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
      std::span(u.data(), u.allocSize()),
      std::span(uExpected.data(), uExpected.allocSize()));
#else
  advpt::testing::dont_compile();
#endif
} // namespace

void testMacroscopicQuantities2() {
#if TEST_MACROSCOPIC_QUANTITIES

  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};

  GridVector<double, 1> rho{cells};
  GridVector<double, 3> u{cells};

  GridVector<double, 1> rhoExpected{cells};
  GridVector<double, 3> uExpected{cells};

  const HighFive::File datafile{std::filesystem::path(DATAFILE),
                          HighFive::File::ReadOnly};
  const size_t numChunks{datafile.getAttribute("numChunks").read<size_t>()};

  auto rhoDset = datafile.getDataSet("rho");
  auto uDset = datafile.getDataSet("u");
  auto pdfsDset = datafile.getDataSet("preCollisionPdfs");

  for (auto chunkIdx : std::views::iota(0uz, numChunks)) {
    rhoDset.select({chunkIdx, 0uz}, {1uz, 8uz}).read_raw(rhoExpected.data());
    uDset.select({chunkIdx, 0uz, 0uz}, {1uz, 3uz, 8uz})
        .read_raw(uExpected.data());
    pdfsDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfs.data());

    lbm::ComputeMacroscopicQuantities computeMQs;
    std::ranges::for_each(cells,
                          [&](auto cell) { computeMQs(cell, pdfs.const_view(), rho.view(), u.view()); });

    advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
        std::span(rho.data(), rho.allocSize()),
        std::span(rhoExpected.data(), rhoExpected.allocSize()));

    advpt::testing::with_tolerance{1e-14, 1e-10}.assert_allclose(
        std::span(u.data(), u.allocSize()),
        std::span(uExpected.data(), uExpected.allocSize()));
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCollision() {
#if TEST_COLLISION
  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};
  lbm::D3Q19Lattice pdfsExpected{cells};

  GridVector<double, 1> rho{cells};
  GridVector<double, 3> u{cells};

  const HighFive::File datafile{std::filesystem::path(DATAFILE),
                          HighFive::File::ReadOnly};
  const size_t numChunks{datafile.getAttribute("numChunks").read<size_t>()};

  auto rhoDset = datafile.getDataSet("rho");
  auto uDset = datafile.getDataSet("u");
  auto pdfsDset = datafile.getDataSet("preCollisionPdfs");
  auto pdfsExpectedDset = datafile.getDataSet("postCollisionPdfs");
  auto omegaDset = datafile.getDataSet("omega");

  for (auto chunkIdx : std::views::iota(0uz, numChunks)) {
    rhoDset.select({chunkIdx, 0uz}, {1uz, 8uz}).read_raw(rho.data());
    uDset.select({chunkIdx, 0uz, 0uz}, {1uz, 3uz, 8uz}).read_raw(u.data());
    pdfsDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfs.data());
    pdfsExpectedDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfsExpected.data());

    auto omega = omegaDset.select({chunkIdx}, {1uz}).read<double>();

    lbm::Collide collide{omega};
    std::ranges::for_each(cells,
                          [&](auto cell) { collide(cell, pdfs.view(), rho.const_view(), u.const_view()); });

    advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
        std::span(pdfs.data(), pdfs.allocSize()),
        std::span(pdfsExpected.data(), pdfsExpected.allocSize()));
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testStreaming() {
#if TEST_STREAMING
  auto octree = CellOctree::createUniformGrid(1uz);
  auto cells = CellGrid::create(octree)
                   .levels({1uz})
                   .neighborhood(lbm::D3Q19::CS_NO_CENTER)
                   .periodicityMapper(Torus({true, true, true}))
                   .build();

  lbm::D3Q19Lattice pdfs{cells};
  lbm::D3Q19Lattice pdfsTmp{cells};
  lbm::D3Q19Lattice pdfsExpected{cells};

  const HighFive::File datafile{std::filesystem::path(DATAFILE),
                          HighFive::File::ReadOnly};
  const size_t numChunks{datafile.getAttribute("numChunks").read<size_t>()};

  auto pdfsDset = datafile.getDataSet("postCollisionPdfs");
  auto pdfsExpectedDset = datafile.getDataSet("postStreamingPdfs");

  for (auto chunkIdx : std::views::iota(0uz, numChunks)) {
    pdfsDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfs.data());
    pdfsExpectedDset.select({chunkIdx, 0uz, 0uz}, {1uz, 19uz, 8uz})
        .read_raw(pdfsExpected.data());

    lbm::Stream stream;
    std::ranges::for_each(cells,
                          [&](auto cell) { stream(cell, pdfsTmp.view(), pdfs.const_view()); });

    advpt::testing::with_tolerance{0., 1e-10}.assert_allclose(
        std::span(pdfsTmp.data(), pdfsTmp.allocSize()),
        std::span(pdfsExpected.data(), pdfsExpected.allocSize()));
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testInitialization1", &testInitialization1},
      {"testInitialization2", &testInitialization2},
      {"testMacroscopicQuantities1", &testMacroscopicQuantities1},
      {"testMacroscopicQuantities2", &testMacroscopicQuantities2},
      {"testCollision", &testCollision},
      {"testStreaming", &testStreaming},
  }
      .run(argc, argv);
}
