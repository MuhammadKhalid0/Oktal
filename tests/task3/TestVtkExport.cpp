#include "advpt/testing/Testutils.hpp"

#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <concepts>
#include <utility>

#include <highfive/highfive.hpp>

#define TEST_TREE_EXPORT true
#define TEST_LEVEL_CELL_DATA true


namespace {

using namespace oktal;

void testTrivialTree() {
#if TEST_TREE_EXPORT
  const CellOctree ot(OctreeGeometry({0.5, 0.3, 1.2}, 1.4));
  const auto tmpDir = advpt::testing::tmp_dir();
  const auto filename = tmpDir / "trivialTree.vtkhdf";
  io::vtk::exportOctree(ot, filename);

  const HighFive::File h5file(filename, HighFive::File::ReadOnly);

  std::vector<double> coords;

  h5file.getDataSet("VTKHDF/XCoordinates").read(coords);
  advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
      coords, std::array{0.5, 1.9});

  h5file.getDataSet("VTKHDF/YCoordinates").read(coords);
  advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
      coords, std::array{0.3, 1.7});

  h5file.getDataSet("VTKHDF/ZCoordinates").read(coords);
  advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
      coords, std::array{1.2, 2.6});

  std::vector<uint8_t> descr;
  h5file.getDataSet("VTKHDF/Descriptors").read(descr);
  advpt::testing::assert_true(descr.empty());

  std::vector<uint8_t> mask;
  h5file.getDataSet("VTKHDF/Mask").read(mask);
  advpt::testing::assert_range_equal(mask, std::array{uint8_t(0)});
#else
  advpt::testing::dont_compile();
#endif
}

void testSimpleTree() {
#if TEST_TREE_EXPORT
  const auto ot = CellOctree::fromDescriptor("R|R......R|................");
  const auto tmpDir = advpt::testing::tmp_dir();
  const auto filename = tmpDir / "simpleTree.vtkhdf";
  io::vtk::exportOctree(ot, filename);

  const HighFive::File h5file(filename, HighFive::File::ReadOnly);

  std::vector<double> coords;

  h5file.getDataSet("VTKHDF/XCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  h5file.getDataSet("VTKHDF/YCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  h5file.getDataSet("VTKHDF/ZCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  std::vector<uint8_t> descr;
  h5file.getDataSet("VTKHDF/Descriptors").read(descr);
  advpt::testing::assert_range_equal(descr,
                                     std::array{uint8_t(192), uint8_t(128)});

  std::vector<uint8_t> mask;
  h5file.getDataSet("VTKHDF/Mask").read(mask);
  advpt::testing::assert_range_equal(
      mask, std::array{uint8_t(0), uint8_t(0), uint8_t(0), uint8_t(0)});
#else
  advpt::testing::dont_compile();
#endif
}

void testComplexTree() {
#if TEST_TREE_EXPORT
  const auto ot =
      CellOctree::fromDescriptor("X|.X...XP.|PPPPRR......PPPP|............RRRR|"
                                 "................................");

  const auto tmpDir = advpt::testing::tmp_dir();
  const auto filename = tmpDir / "complexTree.vtkhdf";
  io::vtk::exportOctree(ot, filename);

  const HighFive::File h5file(filename, HighFive::File::ReadOnly);

  std::vector<double> coords;

  h5file.getDataSet("VTKHDF/XCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  h5file.getDataSet("VTKHDF/YCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  h5file.getDataSet("VTKHDF/ZCoordinates").read(coords);
  advpt::testing::assert_range_equal(coords, std::array{0., 1.});

  std::vector<uint8_t> descr;
  h5file.getDataSet("VTKHDF/Descriptors").read(descr);
  advpt::testing::assert_range_equal(
      descr, std::array{uint8_t(162), uint8_t(6), uint8_t(0), uint8_t(0),
                        uint8_t(7), uint8_t(128)});

  std::vector<uint8_t> mask;
  h5file.getDataSet("VTKHDF/Mask").read(mask);
  advpt::testing::assert_range_equal(
      mask,
      std::array{uint8_t(1), uint8_t(120), uint8_t(7), uint8_t(128), uint8_t(0),
                 uint8_t(0), uint8_t(0), uint8_t(0), uint8_t(0), uint8_t(0)});
#else
  advpt::testing::dont_compile();
#endif
}

void testLevelCellData() {
#if TEST_LEVEL_CELL_DATA
  {
    const CellOctree ot(OctreeGeometry({0.5, 0.3, 1.2}, 1.4));
    const auto tmpDir = advpt::testing::tmp_dir();
    const auto filename = tmpDir / "trivialTree.vtkhdf";
    io::vtk::exportOctree(ot, filename);

    const HighFive::File h5file(filename, HighFive::File::ReadOnly);

    std::vector<size_t> levels;
    h5file.getDataSet("VTKHDF/CellData/level").read(levels);
    advpt::testing::assert_range_equal(levels, std::array{0uz});
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|R......R|................");
    const auto tmpDir = advpt::testing::tmp_dir();
    const auto filename = tmpDir / "simpleTree.vtkhdf";
    io::vtk::exportOctree(ot, filename);

    const HighFive::File h5file(filename, HighFive::File::ReadOnly);

    std::vector<size_t> levels;
    h5file.getDataSet("VTKHDF/CellData/level").read(levels);
    advpt::testing::assert_range_equal(
        levels, std::array{0uz, 1uz, 1uz, 1uz, 1uz, 1uz, 1uz, 1uz, 1uz,
                           2uz, 2uz, 2uz, 2uz, 2uz, 2uz, 2uz, 2uz, 2uz,
                           2uz, 2uz, 2uz, 2uz, 2uz, 2uz, 2uz});
  }
#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{{"testTrivialTree", &testTrivialTree},
                                     {"testSimpleTree", &testSimpleTree},
                                     {"testComplexTree", &testComplexTree},
                                     {"testLevelCellData", &testLevelCellData}}
      .run(argc, argv);
}
