#include "advpt/testing/Testutils.hpp"

#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <algorithm>

#define TEST_UNIFORM_GRID true
#define TEST_VTK_EXPORT true

namespace {
using namespace oktal;

void testUniformGrid() {
#if TEST_UNIFORM_GRID
  static_assert(std::same_as<decltype(CellOctree::createUniformGrid(3uz)),
                             std::shared_ptr<const CellOctree>>);

  auto getMortonBits = std::views::transform(
      [](CellOctree::CellView cell) { return cell.mortonIndex().getBits(); });

  for (auto level : std::views::iota(0uz, 6uz)) {
    auto octree = CellOctree::createUniformGrid(level);
    advpt::testing::assert_equal(octree->numberOfLevels(), level + 1uz);
    advpt::testing::assert_range_equal(
        octree->preOrderDepthFirstRange() | getMortonBits,
        std::views::iota(01uz << (3uz * level), 02uz << (3uz * level)));
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testVtkExport() {
#if TEST_VTK_EXPORT
  const auto tmpDir = advpt::testing::tmp_dir();

  {
    const auto filename = tmpDir / "tree1.vtkhdf";
    {
      const auto ot = std::make_shared<CellOctree>(
          CellOctree::fromDescriptor("X|XX......|................"));
      auto cells = CellGrid::create(ot).build();
      const auto values = std::ranges::to<std::vector<int32_t>>(
          std::views::iota(0uz, cells.size()));

      io::vtk::exportCellGrid(cells, filename)
          .writeGridVector<int32_t>("values", values);
    }

    {
      const HighFive::File h5file(filename, HighFive::File::ReadOnly);
      std::vector<int32_t> readValues;
      h5file.getDataSet("VTKHDF/CellData/values").read(readValues);

      std::vector<int32_t> expectedValues(25uz, 0);
      std::ranges::iota(std::views::drop(expectedValues, 3uz), 0);
      advpt::testing::assert_range_equal(readValues, expectedValues);
    }
  }

  {
    const auto filename = tmpDir / "tree2.vtkhdf";
    {

      const auto ot = std::make_shared<CellOctree>(
          CellOctree::fromDescriptor("R|R......R|................"));
      auto cells = CellGrid::create(ot).levels({2uz}).build();
      const auto values = std::ranges::to<std::vector<int32_t>>(
          std::views::iota(0uz, cells.size()));

      io::vtk::exportCellGrid(cells, filename)
          .writeGridVector<int32_t>("values", values);
    }

    {
      const HighFive::File h5file(filename, HighFive::File::ReadOnly);
      std::vector<int32_t> readValues;
      h5file.getDataSet("VTKHDF/CellData/values").read(readValues);

      std::vector<int32_t> expectedValues(25uz, 0);
      std::ranges::iota(std::views::drop(expectedValues, 9uz), 0);
      advpt::testing::assert_range_equal(readValues, expectedValues);
    }
  }

  {
    const auto filename = tmpDir / "tree3.vtkhdf";
    {

      const auto ot = std::make_shared<CellOctree>(
          CellOctree::fromDescriptor("R|R......R|................"));
      auto cells = CellGrid::create(ot).levels({2uz}).build();
      const auto values = std::ranges::to<std::vector<int32_t>>(
          std::views::iota(0uz, cells.size()));
      const std::vector<float> floats(cells.size(), 42.5f);

      io::vtk::exportCellGrid(cells, filename)
          .writeGridVector<int32_t>("values", values)
          .writeGridVector<float>("floats", floats);
    }

    {
      const HighFive::File h5file(filename, HighFive::File::ReadOnly);

      advpt::testing::assert_true(h5file.exist("VTKHDF/CellData/values"));

      std::vector<float> readFloats;
      h5file.getDataSet("VTKHDF/CellData/floats").read(readFloats);

      std::vector<float> expectedFloats(25uz, 0.f);
      std::ranges::fill(std::views::drop(expectedFloats, 9uz), 42.5f);
      advpt::testing::assert_range_equal(readFloats, expectedFloats);
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}
} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testUniformGrid", &testUniformGrid},
      {"testVtkExport", &testVtkExport},
  }
      .run(argc, argv);
}
