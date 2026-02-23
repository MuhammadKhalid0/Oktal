#include <algorithm>
#include <print>

#include "advpt/testing/Testutils.hpp"

#include "oktal/data/GridVector.hpp"
#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#define TEST_WRITE_GRID_VECTOR true


namespace {
using namespace oktal;

void testWriteGridVector() {
#if TEST_WRITE_GRID_VECTOR
  const auto tmpDir = advpt::testing::tmp_dir();
  const auto filename = tmpDir / "gridVectorExport1.vtkhdf";

  //  scalar field
  {
    std::vector<double> expectedValues(73uz, 0.0);
    const std::span<double> expectedValuesLevel2(&expectedValues[9uz], 64uz);

    {
      auto ot = CellOctree::createUniformGrid(2uz);
      auto cells = CellGrid::create(ot).build();

      GridVector<double, 1> v(cells);
      auto vView = v.view();

      for (auto c : cells) {
        const size_t cellIdx{c};
        vView[cellIdx] = expectedValuesLevel2[cellIdx] = c.center()[0];
      }

      io::vtk::exportCellGrid(cells, filename)
          .writeGridVector<double, 1>("v", v.const_view());
    }

    {
      const HighFive::File h5file(filename, HighFive::File::ReadOnly);
      std::vector<double> readValues(expectedValues.size(), -1.0);
      h5file.getDataSet("VTKHDF/CellData/v").read_raw(readValues.data());
      advpt::testing::with_tolerance{0., 1e-12}.assert_allclose(readValues,
                                                                expectedValues);
    }
  }

  //  3D vector field
  {
    std::vector<double> expectedValues(73uz * 3uz, 0.0);
    const advpt::htgfile::MdCellDataView<double, 2> expectedValuesLevel2(
        &expectedValues[27uz], 64uz, 3uz);

    {
      auto ot = CellOctree::createUniformGrid(2uz);
      auto cells = CellGrid::create(ot).build();

      GridVector<double, 3> v(cells);
      auto vView = v.view();

      for (auto c : cells) {
        const size_t cellIdx{c};
        vView[cellIdx, 0] = expectedValuesLevel2[cellIdx, 0] = c.center()[0];
        vView[cellIdx, 1] = expectedValuesLevel2[cellIdx, 1] = c.center()[1];
        vView[cellIdx, 2] = expectedValuesLevel2[cellIdx, 2] = c.center()[2];
      }

      io::vtk::exportCellGrid(cells, filename)
          .writeGridVector<double, 3>("v", v.const_view());
    }

    {
      const HighFive::File h5file(filename, HighFive::File::ReadOnly);
      std::vector<double> readValues(expectedValues.size(), -1.0);
      h5file.getDataSet("VTKHDF/CellData/v").read_raw(readValues.data());
      advpt::testing::with_tolerance{0., 1e-12}.assert_allclose(readValues,
                                                                expectedValues);
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testWriteGridVector", &testWriteGridVector},
  }
      .run(argc, argv);
}
