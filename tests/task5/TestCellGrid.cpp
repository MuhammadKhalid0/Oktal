#include "advpt/testing/Testutils.hpp"

#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#include <algorithm>
#include <ranges>

#define TEST_ENUMERATION_INTERFACE true
#define TEST_BUILDER true
#define TEST_ADJACENCY true
#define TEST_PERIODICITY true
#define TEST_RANGE true

namespace {

using namespace oktal;

void testEnumerationInterface() {
#if TEST_ENUMERATION_INTERFACE
  static_assert(
      std::same_as<decltype(std::declval<const CellGrid &>().octree()),
                   const CellOctree &>);

  static_assert(
      std::same_as<decltype(std::declval<const CellGrid &>().mortonIndices()),
                   std::span<const MortonIndex>>);

  static_assert(
      std::same_as<decltype(std::declval<const CellGrid &>()
                                .getEnumerationIndex(std::declval<size_t>())),
                   size_t>);

  static_assert(std::same_as<
                decltype(std::declval<const CellGrid &>().getEnumerationIndex(
                    std::declval<const CellOctree::CellView &>())),
                size_t>);
#else
  advpt::testing::dont_compile();
#endif
}

void testBuilderInterface() {
#if TEST_BUILDER
  static_assert(
      std::same_as<decltype(CellGrid::create(
                       std::declval<std::shared_ptr<const CellOctree>>())),
                   CellGridBuilder>);

  static_assert(std::same_as<decltype(std::declval<CellGridBuilder &>().levels(
                                 std::declval<std::span<const size_t>>())),
                             CellGridBuilder &>);

  static_assert(
      std::same_as<decltype(std::declval<CellGridBuilder &>().levels(
                       std::declval<std::initializer_list<size_t>>())),
                   CellGridBuilder &>);

  static_assert(
      std::same_as<decltype(std::declval<CellGridBuilder &>().build()),
                   CellGrid>);
#else
  advpt::testing::dont_compile();
#endif
}

void testEnumerateNoPhantoms() {
#if TEST_BUILDER
  const auto ot = std::make_shared<CellOctree>(
      CellOctree::fromDescriptor("R|RR......|................"));

  {
    const auto cells = CellGrid::create(ot).build();

    const std::vector<morton_bits_t> expected{
        // root
        01,
        // level 1
        010,
        011,
        012,
        013,
        014,
        015,
        016,
        017,
        // level 2
        0100,
        0101,
        0102,
        0103,
        0104,
        0105,
        0106,
        0107,
        0110,
        0111,
        0112,
        0113,
        0114,
        0115,
        0116,
        0117,
    };

    advpt::testing::assert_range_equal(
        cells.mortonIndices() |
            std::views::transform([](auto m) { return m.getBits(); }),
        expected);
  }

  {
    const auto cells =
        CellGrid::create(ot).levels(std::array{1uz, 2uz}).build();

    const std::vector<morton_bits_t> expected{
        // level 1
        010,
        011,
        012,
        013,
        014,
        015,
        016,
        017,
        // level 2
        0100,
        0101,
        0102,
        0103,
        0104,
        0105,
        0106,
        0107,
        0110,
        0111,
        0112,
        0113,
        0114,
        0115,
        0116,
        0117,
    };

    advpt::testing::assert_range_equal(
        cells.mortonIndices() |
            std::views::transform([](auto m) { return m.getBits(); }),
        expected);
  }

  {
    const auto cells = CellGrid::create(ot).levels({0uz, 2uz}).build();

    const std::vector<morton_bits_t> expected{
        // root
        01,
        // level 2
        0100,
        0101,
        0102,
        0103,
        0104,
        0105,
        0106,
        0107,
        0110,
        0111,
        0112,
        0113,
        0114,
        0115,
        0116,
        0117,
    };

    advpt::testing::assert_range_equal(
        cells.mortonIndices() |
            std::views::transform([](auto m) { return m.getBits(); }),
        expected);
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testEnumerateWithPhantoms() {
#if TEST_BUILDER
  {
    const auto ot = std::make_shared<CellOctree>(
        CellOctree::fromDescriptor("R|XX....PP|................"));
    const auto cells = CellGrid::create(ot).build();

    const std::vector<morton_bits_t> expected{
        // root
        01,
        // level 1

        012,
        013,
        014,
        015,

        // level 2
        0100,
        0101,
        0102,
        0103,
        0104,
        0105,
        0106,
        0107,
        0110,
        0111,
        0112,
        0113,
        0114,
        0115,
        0116,
        0117,
    };

    advpt::testing::assert_range_equal(
        cells.mortonIndices() |
            std::views::transform([](auto m) { return m.getBits(); }),
        expected);
  }

  {
    const auto ot = std::make_shared<CellOctree>(CellOctree::fromDescriptor(
        "X|XXXXPPPP|...PPPPP..P.PPPP.P..PPPP.P.PPPPP"));
    const auto cells = CellGrid::create(ot).levels({2uz}).build();

    const std::vector<morton_bits_t> expected{0100uz, 0101uz, 0102uz,

                                              0110uz, 0111uz, 0113uz,

                                              0120uz, 0122uz, 0123uz,

                                              0130uz, 0132uz};

    advpt::testing::assert_range_equal(
        cells.mortonIndices() |
            std::views::transform([](auto m) { return m.getBits(); }),
        expected);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testAdjacencyInterface() {
#if TEST_ADJACENCY
  static_assert(
      std::same_as<decltype(std::declval<const CellGrid &>().neighborIndices(
                       std::declval<Vec<std::ptrdiff_t, 3>>())),
                   std::span<const size_t>>);

  {
    const auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("R|........"));
    const auto cells = CellGrid::create(ot).build();

    advpt::testing::throws<std::out_of_range>(
        [&cells]() { auto _ = cells.neighborIndices({-1, 0, 0}); });

    advpt::testing::throws<std::out_of_range>(
        [&cells]() { auto _ = cells.neighborIndices({1, 0, 0}); });
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testNeighborIndices() {
#if TEST_ADJACENCY
  {
    const auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("R|........"));
    const auto cells = CellGrid::create(ot)
                           .neighborhood({{-1, 0, 0},
                                          {1, 0, 0},
                                          {0, -1, 0},
                                          {0, 1, 0},
                                          {0, 0, -1},
                                          {0, 0, 1}})
                           .build();

    auto getNeighborsMortonIndices =
        std::views::transform([&cells](auto cellIdx) {
          if (cellIdx == CellGrid::NO_NEIGHBOR) {
            return 0uz;
          }
          return cells.mortonIndices()[cellIdx].getBits();
        });

    advpt::testing::assert_range_equal(
        cells.neighborIndices({-1, 0, 0}) | getNeighborsMortonIndices,
        std::vector{0uz, 0uz, 0b1000uz, 0uz, 0b1010uz, 0uz, 0b1100uz, 0uz,
                    0b1110uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({1, 0, 0}) | getNeighborsMortonIndices,
        std::vector{0uz, 0b1001uz, 0uz, 0b1011uz, 0uz, 0b1101uz, 0uz, 0b1111uz,
                    0uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, -1, 0}) | getNeighborsMortonIndices,
        std::vector{0uz, 0uz, 0uz, 0b1000uz, 0b1001uz, 0uz, 0uz, 0b1100uz,
                    0b1101uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 1, 0}) | getNeighborsMortonIndices,
        std::vector{0uz, 0b1010uz, 0b1011uz, 0uz, 0uz, 0b1110uz, 0b1111uz, 0uz,
                    0uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, -1}) | getNeighborsMortonIndices,
        std::vector{0uz, 0uz, 0uz, 0uz, 0uz, 0b1000uz, 0b1001uz, 0b1010uz,
                    0b1011uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, 1}) | getNeighborsMortonIndices,
        std::vector{0uz, 0b1100uz, 0b1101uz, 0b1110uz, 0b1111uz, 0uz, 0uz, 0uz,
                    0uz});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testNoPeriodicity() {
#if TEST_PERIODICITY
  static_assert(std::semiregular<NoPeriodicity>);

  static_assert(
      std::same_as<decltype(std::declval<CellGridBuilder &>().periodicityMapper(
                       std::declval<const NoPeriodicity &>())),
                   CellGridBuilder &>);

  {
    const auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("R|........"));
    const auto cells = CellGrid::create(ot)
                           .levels({1uz})
                           .neighborhood({{-1, -1, 0}, {1, 1, 0}})
                           .periodicityMapper(NoPeriodicity())
                           .build();

    auto getNeighborsMortonIndices =
        std::views::transform([&cells](auto cellIdx) {
          if (cellIdx == CellGrid::NO_NEIGHBOR) {
            return 0uz;
          }
          return cells.mortonIndices()[cellIdx].getBits();
        });

    advpt::testing::assert_range_equal(
        cells.neighborIndices({-1, -1, 0}) | getNeighborsMortonIndices,
        std::array{0uz, 0uz, 0uz, 010uz, 0uz, 0uz, 0uz, 014uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({1, 1, 0}) | getNeighborsMortonIndices,
        std::array{013uz, 0uz, 0uz, 0uz, 017uz, 0uz, 0uz, 0uz});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testTorus() {
#if TEST_PERIODICITY
  static_assert(
      std::same_as<decltype(std::declval<CellGridBuilder &>().periodicityMapper(
                       std::declval<const Torus &>())),
                   CellGridBuilder &>);

  {
    const auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("R|........"));
    const auto cells =
        CellGrid::create(ot)
            .levels({1uz})
            .neighborhood({{-1, -1, 0}, {1, 1, 0}, {0, 0, 1}, {0, 0, -1}})
            .periodicityMapper(Torus({true, true, false}))
            .build();

    auto getNeighborsMortonIndices =
        std::views::transform([&cells](auto cellIdx) {
          if (cellIdx == CellGrid::NO_NEIGHBOR) {
            return 0uz;
          }
          return cells.mortonIndices()[cellIdx].getBits();
        });

    advpt::testing::assert_range_equal(
        cells.neighborIndices({-1, -1, 0}) | getNeighborsMortonIndices,
        std::array{013uz, 012uz, 011uz, 010uz, 017uz, 016uz, 015uz, 014uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({1, 1, 0}) | getNeighborsMortonIndices,
        std::array{013uz, 012uz, 011uz, 010uz, 017uz, 016uz, 015uz, 014uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, 1}) | getNeighborsMortonIndices,
        std::array{014uz, 015uz, 016uz, 017uz, 0uz, 0uz, 0uz, 0uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, -1}) | getNeighborsMortonIndices,
        std::array{0uz, 0uz, 0uz, 0uz, 010uz, 011uz, 012uz, 013uz});
  }

  {

    const auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("R|........"));
    const auto cells = CellGrid::create(ot)
                           .neighborhood({{-1, 0, 0},
                                          {1, 0, 0},
                                          {0, -1, 0},
                                          {0, 1, 0},
                                          {0, 0, -1},
                                          {0, 0, 1}})
                           .periodicityMapper(Torus({true, true, true}))
                           .build();

    auto getNeighborsMortonIndices =
        std::views::transform([&cells](auto cellIdx) {
          if (cellIdx == CellGrid::NO_NEIGHBOR) {
            return 0uz;
          }
          return cells[cellIdx].mortonIndex().getBits();
        });

    advpt::testing::assert_range_equal(
        cells.neighborIndices({-1, 0, 0}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1001uz, 0b1000uz, 0b1011uz, 0b1010uz, 0b1101uz,
                    0b1100uz, 0b1111uz, 0b1110uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({1, 0, 0}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1001uz, 0b1000uz, 0b1011uz, 0b1010uz, 0b1101uz,
                    0b1100uz, 0b1111uz, 0b1110uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, -1, 0}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1010uz, 0b1011uz, 0b1000uz, 0b1001uz, 0b1110uz,
                    0b1111uz, 0b1100uz, 0b1101uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 1, 0}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1010uz, 0b1011uz, 0b1000uz, 0b1001uz, 0b1110uz,
                    0b1111uz, 0b1100uz, 0b1101uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, -1}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1100uz, 0b1101uz, 0b1110uz, 0b1111uz, 0b1000uz,
                    0b1001uz, 0b1010uz, 0b1011uz});
    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, 1}) | getNeighborsMortonIndices,
        std::vector{0b1uz, 0b1100uz, 0b1101uz, 0b1110uz, 0b1111uz, 0b1000uz,
                    0b1001uz, 0b1010uz, 0b1011uz});
  }

  {
    const auto ot = std::make_shared<CellOctree>(CellOctree::fromDescriptor(
        "X|XXXXPPPP|...PPPPP..P.PPPP.P..PPPP.P.PPPPP"));
    const auto cells = CellGrid::create(ot)
                           .levels({2uz})
                           .neighborhood({{-1, 0, 0},
                                          {1, 0, 0},
                                          {0, -1, 0},
                                          {0, 1, 0},
                                          {0, 0, -1},
                                          {0, 0, 1}})
                           .periodicityMapper(Torus({true, true, false}))
                           .build();

    auto getNeighborsMortonIndices =
        std::views::transform([&cells](auto cellIdx) {
          if (cellIdx == CellGrid::NO_NEIGHBOR) {
            return 0uz;
          }
          return cells.mortonIndices()[cellIdx].getBits();
        });

    advpt::testing::assert_range_equal(cells.neighborIndices({1, 0, 0}) |
                                           getNeighborsMortonIndices,
                                       std::array{0101uz, 0110uz, 0uz,

                                                  0111uz, 0100uz, 0102uz,

                                                  0uz, 0123uz, 0132uz,

                                                  0uz, 0uz});

    advpt::testing::assert_range_equal(cells.neighborIndices({-1, 0, 0}) |
                                           getNeighborsMortonIndices,
                                       std::array{
                                           0111uz,
                                           0100uz,
                                           0113uz,

                                           0101uz,
                                           0110uz,
                                           0uz,

                                           0uz,
                                           0uz,
                                           0122uz,

                                           0uz,
                                           0123uz,
                                       });

    advpt::testing::assert_range_equal(cells.neighborIndices({0, 1, 0}) |
                                           getNeighborsMortonIndices,
                                       std::array{0102uz, 0uz, 0120uz,

                                                  0uz, 0113uz, 0uz,

                                                  0122uz, 0100uz, 0101uz,

                                                  0132uz, 0110uz});

    advpt::testing::assert_range_equal(cells.neighborIndices({0, -1, 0}) |
                                           getNeighborsMortonIndices,
                                       std::array{0122uz, 0123uz, 0100uz,

                                                  0132uz, 0uz, 0111uz,

                                                  0102uz, 0120uz, 0uz,

                                                  0uz, 0130uz});

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, -1}) | getNeighborsMortonIndices,
        std::views::repeat(0uz) | std::views::take(11));

    advpt::testing::assert_range_equal(
        cells.neighborIndices({0, 0, 1}) | getNeighborsMortonIndices,
        std::views::repeat(0uz) | std::views::take(11));
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCellsRange() {
#if TEST_RANGE
  static_assert(std::ranges::forward_range<CellGrid>);
  static_assert(std::ranges::sized_range<CellGrid>);

  using CellType = decltype(*std::declval<CellGrid>().begin());

  static_assert(std::is_convertible_v<CellType, std::size_t>);
  static_assert(std::is_convertible_v<CellType, bool>);

  {
    auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("X|........"));
    auto cells = CellGrid::create(ot).levels({1uz}).build();

    advpt::testing::assert_equal(cells.size(), 8uz);

    for (auto [gridCell, treeCell] :
         std::views::zip(cells, ot->horizontalRange(1uz))) {
      advpt::testing::assert_equal(cells.getEnumerationIndex(treeCell),
                                   size_t(gridCell));
      advpt::testing::assert_equal(gridCell.mortonIndex().getBits(),
                                   treeCell.mortonIndex().getBits());
      advpt::testing::assert_equal(gridCell.level(), treeCell.level());
      advpt::testing::assert_equal(gridCell.center(), treeCell.center());
      advpt::testing::assert_equal(gridCell.boundingBox().minCorner(),
                                   treeCell.boundingBox().minCorner());
      advpt::testing::assert_equal(gridCell.boundingBox().maxCorner(),
                                   treeCell.boundingBox().maxCorner());
    }

    std::vector<Vec3D> centerPoints(cells.size(), Vec3D(0.0));

    for (auto cell : cells) {
      centerPoints[cell] = cell.center();
    }

    for (auto [cp, treeCell] :
         std::views::zip(centerPoints, ot->horizontalRange(1uz))) {
      advpt::testing::assert_equal(cp, treeCell.center());
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCentralDifference() {
#if TEST_RANGE

  auto octree = std::make_shared<CellOctree>(CellOctree::fromDescriptor(
      "X|XXXXPPPP|................................"));
  auto cells = CellGrid::create(octree)
                   .levels({2uz})
                   .neighborhood({{-1, 0, 0}, {1, 0, 0}})
                   .build();

  std::vector<double> c(cells.size());
  for (auto cell : cells) {
    c[cell] = cell.center()[0] * cell.center()[0];
  }

  std::vector<double> c_deriv(cells.size());
  const double h = octree->geometry().dx(2uz);

  for (auto cell : cells) {
    auto neighborWest = cell.neighbor({-1, 0, 0});
    auto neighborEast = cell.neighbor({1, 0, 0});

    //  Can only compute derivative if both neighbors exist
    if (neighborWest && neighborEast) {
      c_deriv[cell] = (c[neighborEast] - c[neighborWest]) / (2. * h);
    }
  }

  for (auto cell : cells) {
    if (cell.neighbor({-1, 0, 0}) && cell.neighbor({1, 0, 0})) {
      advpt::testing::with_tolerance{0., 1e-12}.assert_close(
          c_deriv[cell], 2. * cell.center()[0]);
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testEnumerationInterface", &testEnumerationInterface},
      {"testBuilderInterface", &testBuilderInterface},
      {"testEnumerateNoPhantoms", &testEnumerateNoPhantoms},
      {"testEnumerateWithPhantoms", &testEnumerateWithPhantoms},
      {"testAdjacencyInterface", &testAdjacencyInterface},
      {"testNeighborIndices", &testNeighborIndices},
      {"testNoPeriodicity", &testNoPeriodicity},
      {"testTorus", &testTorus},
      {"testCellsRange", &testCellsRange},
      {"testCentralDifference", &testCentralDifference}}
      .run(argc, argv);
}
