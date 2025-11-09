#include "advpt/testing/Testutils.hpp"

#include "oktal/octree/MortonIndex.hpp"

#include <concepts>
#include <iostream>
#include <numeric>
#include <ranges>
#include <type_traits>

#define TEST_BASIC_INTERFACE true
#define TEST_PATH_CONVERSION true
#define TEST_POSITION_QUERIES true
#define TEST_TRAVERSAL true
#define TEST_PARTIAL_ORDER true
#define TEST_GRID_COORDINATES true

namespace {

using namespace oktal;

void testBasicInterface() {
#if TEST_BASIC_INTERFACE
  static_assert(std::same_as<oktal::morton_bits_t, std::uint64_t>);
  static_assert(sizeof(oktal::MortonIndex) == sizeof(uint64_t));
  static_assert(oktal::MortonIndex::MAX_DEPTH == 21);

  advpt::testing::assert_equal(MortonIndex().getBits(), morton_bits_t(0b1));
  advpt::testing::assert_equal(MortonIndex(017513).getBits(),
                               morton_bits_t(017513));
  advpt::testing::assert_equal(MortonIndex(000102).getBits(),
                               morton_bits_t(000102));
  advpt::testing::assert_equal(MortonIndex(0100).getBits(),
                               morton_bits_t(0100));
#else
  advpt::testing::dont_compile();
#endif
}

void testFromPath() {
#if TEST_PATH_CONVERSION

  advpt::testing::assert_equal(MortonIndex::fromPath({}).getBits(),
                               std::uint64_t(0b1));
  advpt::testing::assert_equal(MortonIndex::fromPath({0}).getBits(),
                               std::uint64_t(0b1000));
  advpt::testing::assert_equal(MortonIndex::fromPath({1}).getBits(),
                               std::uint64_t(0b1001));
  advpt::testing::assert_equal(MortonIndex::fromPath({6}).getBits(),
                               std::uint64_t(0b1110));
  advpt::testing::assert_equal(MortonIndex::fromPath({1, 3, 1}).getBits(),
                               std::uint64_t{0b1001011001});
  advpt::testing::assert_equal(MortonIndex::fromPath({7, 2, 5}).getBits(),
                               std::uint64_t{0b1111010101});
#else
  advpt::testing::dont_compile();
#endif
}

void testGetPath() {
#if TEST_PATH_CONVERSION

  advpt::testing::assert_range_equal(MortonIndex(0b1).getPath(),
                                     std::vector<morton_bits_t>{});
  advpt::testing::assert_range_equal(MortonIndex(0b1000000).getPath(),
                                     std::vector<morton_bits_t>{0, 0});
  advpt::testing::assert_range_equal(MortonIndex(0b1101011).getPath(),
                                     std::vector<morton_bits_t>{5, 3});
  advpt::testing::assert_range_equal(MortonIndex(0b1111010110).getPath(),
                                     std::vector<morton_bits_t>{7, 2, 6});
  advpt::testing::assert_range_equal(MortonIndex(0b1001011101111).getPath(),
                                     std::vector<morton_bits_t>{1, 3, 5, 7});
#else
  advpt::testing::dont_compile();
#endif
}

void testPositionQueries() {
#if TEST_POSITION_QUERIES

  {
    const MortonIndex m;
    advpt::testing::assert_true(m.isRoot());
    advpt::testing::assert_true(m.isFirstSibling());
    advpt::testing::assert_true(m.isLastSibling());
    advpt::testing::assert_equal(m.level(), 0uz);
    advpt::testing::assert_equal(m.siblingIndex(), 0uz);
  }

  {
    const MortonIndex m{01043};
    advpt::testing::assert_false(m.isRoot());
    advpt::testing::assert_false(m.isFirstSibling());
    advpt::testing::assert_false(m.isLastSibling());
    advpt::testing::assert_equal(m.level(), 3uz);
    advpt::testing::assert_equal(m.siblingIndex(), 3uz);
  }

  {
    const MortonIndex m{01070};
    advpt::testing::assert_false(m.isRoot());
    advpt::testing::assert_true(m.isFirstSibling());
    advpt::testing::assert_false(m.isLastSibling());
    advpt::testing::assert_equal(m.level(), 3uz);
    advpt::testing::assert_equal(m.siblingIndex(), 0uz);
  }

  {
    const MortonIndex m{017};
    advpt::testing::assert_false(m.isRoot());
    advpt::testing::assert_false(m.isFirstSibling());
    advpt::testing::assert_true(m.isLastSibling());
    advpt::testing::assert_equal(m.level(), 1uz);
    advpt::testing::assert_equal(m.siblingIndex(), 7uz);
  }

  {
    const MortonIndex m{0135};
    advpt::testing::assert_false(m.isRoot());
    advpt::testing::assert_false(m.isFirstSibling());
    advpt::testing::assert_false(m.isLastSibling());
    advpt::testing::assert_equal(m.level(), 2uz);
    advpt::testing::assert_equal(m.siblingIndex(), 5uz);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testTraversal() {
#if TEST_TRAVERSAL
  {
    const MortonIndex root;

    for (auto branch : std::views::iota(0uz, 8uz)) {
      const MortonIndex child = root.child(branch);
      advpt::testing::assert_false(child.isRoot());
      advpt::testing::assert_equal(child.level(), size_t(1));
      advpt::testing::assert_equal(child.siblingIndex(), branch);

      advpt::testing::assert_equal(child.parent().getBits(), root.getBits());
    }
  }

  {
    for (const auto &path : {
             std::vector<morton_bits_t>{1, 5, 2, 7, 0},
             std::vector<morton_bits_t>{0, 0, 3, 6, 1},
             std::vector<morton_bits_t>{4, 0, 2, 0, 7},
         }) {

      MortonIndex m;

      for (auto [level, branch] : path | std::views::enumerate) {
        m = m.child(branch);
        advpt::testing::assert_range_equal(m.getPath(),
                                           path | std::views::take(level + 1));

        if (level > 1) {
          advpt::testing::assert_range_equal(m.parent().getPath(),
                                             path | std::views::take(level));
        }
      }
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testSafeTraversal() {
#if TEST_TRAVERSAL

  {
    const MortonIndex root;
    advpt::testing::throws<std::logic_error>(
        [&]() { auto _ = root.safeParent(); });
  }

  {
    MortonIndex m;
    for (auto level : std::views::iota(0, 21)) {
      m = m.safeChild(0);
      advpt::testing::assert_equal(m.getBits(), 0x1uz << (3 * (level + 1)));
    }

    advpt::testing::assert_equal(m.getBits(), 0x8000000000000000);

    advpt::testing::throws<std::logic_error>(
        [&]() { auto _ = m.safeChild(3); });
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testEquality() {
#if TEST_PARTIAL_ORDER

  advpt::testing::assert_true(MortonIndex() == MortonIndex());
  advpt::testing::assert_false(MortonIndex() != MortonIndex());

  advpt::testing::assert_true(MortonIndex(01023) == MortonIndex(01023));
  advpt::testing::assert_false(MortonIndex(01023) == MortonIndex(01203));
  advpt::testing::assert_false(MortonIndex(01023) != MortonIndex(01023));
  advpt::testing::assert_true(MortonIndex(01023) != MortonIndex(01203));

#else
  advpt::testing::dont_compile();
#endif
}

void testInequalities() {
#if TEST_PARTIAL_ORDER

  advpt::testing::assert_true(MortonIndex() <= MortonIndex());
  advpt::testing::assert_true(MortonIndex() >= MortonIndex());

  advpt::testing::assert_false(MortonIndex() < MortonIndex());
  advpt::testing::assert_false(MortonIndex() > MortonIndex());

  advpt::testing::assert_true(MortonIndex(01023) <= MortonIndex(01023));
  advpt::testing::assert_true(MortonIndex(01023) >= MortonIndex(01023));
  advpt::testing::assert_false(MortonIndex(01023) < MortonIndex(01203));
  advpt::testing::assert_false(MortonIndex(01023) > MortonIndex(01203));

  advpt::testing::assert_true(MortonIndex() > MortonIndex(012));
  advpt::testing::assert_true(MortonIndex() > MortonIndex(0143));
  advpt::testing::assert_true(MortonIndex() >= MortonIndex(012));
  advpt::testing::assert_true(MortonIndex() >= MortonIndex(010301));

  advpt::testing::assert_true(MortonIndex(01201) >= MortonIndex(01201));
  advpt::testing::assert_true(MortonIndex(01201) > MortonIndex(012014));
  advpt::testing::assert_true(MortonIndex(01201) >= MortonIndex(012014));
  advpt::testing::assert_true(MortonIndex(01201) > MortonIndex(01201431));
  advpt::testing::assert_true(MortonIndex(01201) >= MortonIndex(01201431));

  advpt::testing::assert_true(MortonIndex(012) < MortonIndex());
  advpt::testing::assert_true(MortonIndex(0143) < MortonIndex());
  advpt::testing::assert_true(MortonIndex(012) <= MortonIndex());
  advpt::testing::assert_true(MortonIndex(010301) <= MortonIndex());

  advpt::testing::assert_true(MortonIndex(01201) <= MortonIndex(01201));
  advpt::testing::assert_true(MortonIndex(012014) <= MortonIndex(01201));
  advpt::testing::assert_true(MortonIndex(012014) < MortonIndex(01201));
  advpt::testing::assert_true(MortonIndex(01201431) <= MortonIndex(01201));
  advpt::testing::assert_true(MortonIndex(01201431) < MortonIndex(01201));

#else
  advpt::testing::dont_compile();
#endif
}

void testGridCoordinates() {
#if TEST_GRID_COORDINATES
  {
    const MortonIndex m{};
    advpt::testing::assert_equal(m.gridCoordinates(), Vec<size_t, 3>());
  }

  {
    const MortonIndex m{010};
    advpt::testing::assert_equal(m.gridCoordinates(), Vec<size_t, 3>(0));
  }

  {
    const MortonIndex m{0100};
    advpt::testing::assert_equal(m.gridCoordinates(), Vec<size_t, 3>(0));
  }

  {
    const MortonIndex m{0b1110};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{0uz, 1uz, 1uz});
  }

  {
    const MortonIndex m{0b1000110};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{0uz, 1uz, 1uz});
  }

  {
    const MortonIndex m{0b1010000001};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{1uz, 4uz, 0uz});
  }

  {
    const MortonIndex m{0b1101011};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{3uz, 1uz, 2uz});
  }

  {
    const MortonIndex m{0b1000101011};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{3uz, 1uz, 2uz});
  }

  {
    const MortonIndex m{0b1011101101};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{7uz, 4uz, 3uz});
  }

  {
    const MortonIndex m{0b1011001000110};
    advpt::testing::assert_equal(m.gridCoordinates(),
                                 Vec<size_t, 3>{12uz, 9uz, 1uz});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testFromGridCoordinates() {
#if TEST_GRID_COORDINATES
  {
    advpt::testing::assert_equal(
        MortonIndex::fromGridCoordinates(0, Vec<size_t, 3>()).getBits(),
        morton_bits_t(01));
  }

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(1uz, Vec<size_t, 3>(0)).getBits(),
      010uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(2uz, Vec<size_t, 3>(0)).getBits(),
      0100uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(1uz, Vec<size_t, 3>{0uz, 1uz, 1uz})
          .getBits(),
      0b1110uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(2uz, Vec<size_t, 3>{0uz, 1uz, 1uz})
          .getBits(),
      0b1000110uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(3uz, Vec<size_t, 3>{1uz, 4uz, 0uz})
          .getBits(),
      0b1010000001uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(2uz, Vec<size_t, 3>{3uz, 1uz, 2uz})
          .getBits(),
      0b1101011uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(3uz, Vec<size_t, 3>{3uz, 1uz, 2uz})
          .getBits(),
      0b1000101011uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(3uz, Vec<size_t, 3>{7uz, 4uz, 3uz})
          .getBits(),
      0b1011101101uz);

  advpt::testing::assert_equal(
      MortonIndex::fromGridCoordinates(4uz, Vec<size_t, 3>{12uz, 9uz, 1uz})
          .getBits(),
      0b1011001000110uz);

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testBasicInterface", &testBasicInterface},
      {"testFromPath", &testFromPath},
      {"testGetPath", &testGetPath},
      {"testPositionQueries", &testPositionQueries},
      {"testTraversal", &testTraversal},
      {"testSafeTraversal", &testSafeTraversal},
      {"testEquality", &testEquality},
      {"testInequalities", &testInequalities},
      {"testGridCoordinates", &testGridCoordinates},
      {"testFromGridCoordinates", &testFromGridCoordinates}}
      .run(argc, argv);
}
