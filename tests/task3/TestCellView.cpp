#include "advpt/testing/Testutils.hpp"

#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <concepts>
#include <utility>

#include <highfive/highfive.hpp>

#define TEST_GEOMETRY false
#define TEST_CELL_VIEW false


namespace {

using namespace oktal;

void testGeometry() {
#if TEST_GEOMETRY
  static_assert(std::same_as<decltype(std::declval<CellOctree &>().geometry()),
                             const OctreeGeometry &>);
  static_assert(
      std::same_as<decltype(std::declval<const CellOctree &>().geometry()),
                   const OctreeGeometry &>);

  {
    const CellOctree ot;
    advpt::testing::assert_range_equal(ot.geometry().origin(),
                                       Vec3D{0., 0., 0.});
    advpt::testing::assert_equal(ot.geometry().sidelength(), 1.);
  }

  {
    const CellOctree ot(OctreeGeometry({1., -0.5, 3.2}, 4.1));
    advpt::testing::assert_range_equal(ot.geometry().origin(),
                                       Vec3D{1., -0.5, 3.2});
    advpt::testing::assert_equal(ot.geometry().sidelength(), 4.1);
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCellQueries() {
#if TEST_CELL_VIEW
  {
    const CellOctree ot;
    const auto root =
        ot.getRootCell().value(); // NOLINT (bugprone-unchecked-optional-access)

    advpt::testing::assert_true(root.isRoot());
    advpt::testing::assert_equal(root.mortonIndex().getBits(), 0b1uz);
    advpt::testing::assert_false(root.isRefined());
    advpt::testing::assert_equal(root.level(), 0uz);
    advpt::testing::assert_equal(root.streamIndex(), 0uz);

    advpt::testing::assert_true(ot.cellExists(MortonIndex()));
    advpt::testing::assert_true(ot.getCell(MortonIndex()).has_value());
    advpt::testing::assert_true(
        // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
        ot.getCell(MortonIndex()).value().isRoot());
  }

  {
    const auto ot = CellOctree::fromDescriptor("X|..PP..RX|................");

    advpt::testing::assert_false(ot.getRootCell().has_value());

    for (auto mortonIdx : {0b1000uz, 0b1001uz, 0b1100uz, 0b1101uz, 0b1110uz}) {
      advpt::testing::assert_true(ot.cellExists({mortonIdx}));

      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      const auto cell = ot.getCell({mortonIdx}).value();
      advpt::testing::assert_equal(cell.mortonIndex().getBits(), mortonIdx);
      advpt::testing::assert_equal(cell.level(), 1uz);
      if (mortonIdx == 0b1110uz) {
        advpt::testing::assert_true(cell.isRefined());
      } else {
        advpt::testing::assert_false(cell.isRefined());
      }
    }

    for (auto mortonIdx : {0b1010uz, 0b1011uz, 0b1111uz}) {
      advpt::testing::assert_false(ot.getCell({mortonIdx}).has_value());
      advpt::testing::assert_false(ot.cellExists({mortonIdx}));
    }

    for (auto parent : {0b1110uz, 0b1111uz}) {
      for (auto child : std::views::iota(0uz, 8uz)) {
        const auto mIdx = MortonIndex(parent).child(child);

        advpt::testing::assert_true(ot.cellExists(mIdx));

        // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
        const auto cell = ot.getCell(mIdx).value();
        advpt::testing::assert_equal(cell.mortonIndex().getBits(),
                                     mIdx.getBits());
        advpt::testing::assert_equal(cell.level(), 2uz);
        advpt::testing::assert_false(cell.isRefined());
      }
    }

    for (auto child : std::views::iota(0uz, 8uz)) {
      const auto mIdx = MortonIndex(0b1010uz).child(child);

      advpt::testing::assert_false(ot.cellExists(mIdx));
      advpt::testing::assert_false(ot.getCell(mIdx).has_value());
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testCellGeometry() {
#if TEST_CELL_VIEW
  {
    const CellOctree ot;

    // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
    const auto root = ot.getRootCell().value();

    advpt::testing::assert_range_equal(root.center(), Vec3D(0.5));
    advpt::testing::assert_range_equal(root.boundingBox().minCorner(),
                                       Vec3D(0.0));
    advpt::testing::assert_range_equal(root.boundingBox().maxCorner(),
                                       Vec3D(1.0));
  }

  {
    const auto ot = CellOctree::fromDescriptor("X|..PP..RX|................");

    advpt::testing::assert_false(ot.getRootCell().has_value());

    {
      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      const auto cell = ot.getCell({0b1000uz}).value();
      advpt::testing::assert_range_equal(cell.center(), Vec3D(0.25));
      advpt::testing::assert_range_equal(cell.boundingBox().minCorner(),
                                         Vec3D(0.0));
      advpt::testing::assert_range_equal(cell.boundingBox().maxCorner(),
                                         Vec3D(0.5));
    }

    {
      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      const auto cell = ot.getCell({0b1001uz}).value();
      advpt::testing::assert_range_equal(cell.center(),
                                         Vec3D{0.75, 0.25, 0.25});
      advpt::testing::assert_range_equal(cell.boundingBox().minCorner(),
                                         Vec3D{0.5, 0., 0.});
      advpt::testing::assert_range_equal(cell.boundingBox().maxCorner(),
                                         Vec3D{1.0, 0.5, 0.5});
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{{"testGeometry", &testGeometry},
                                     {"testCellQueries", &testCellQueries},
                                     {"testCellGeometry", &testCellGeometry}}
      .run(argc, argv);
}
