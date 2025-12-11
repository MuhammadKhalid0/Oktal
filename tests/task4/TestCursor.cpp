#include "advpt/testing/Testutils.hpp"

#include "oktal/octree/CellOctree.hpp"

#include <concepts>
#include <iterator>
#include <ranges>

#define TEST_BASIC_INTERFACE true
#define TEST_OBSERVERS true
#define TEST_TRAVERSAL true

namespace {

using namespace oktal;

void testBasicInterface() {
#if TEST_BASIC_INTERFACE

  static_assert(std::same_as<decltype(std::declval<OctreeCursor>().octree()),
                             const CellOctree *>);
  static_assert(std::same_as<decltype(std::declval<OctreeCursor>().path()),
                             std::span<const size_t>>);

  {
    const OctreeCursor emptyCursor;
    advpt::testing::assert_equal(emptyCursor.octree(),
                                 static_cast<const CellOctree *>(nullptr));
    advpt::testing::assert_equal(emptyCursor.path().size(), 0uz);
  }

  {
    const CellOctree octree;
    const OctreeCursor cursor{octree};
    advpt::testing::assert_equal(cursor.octree(), &octree);
    advpt::testing::assert_range_equal(cursor.path(), std::array{0uz});
  }

  {
    const CellOctree octree;
    const OctreeCursor cursor{octree, std::array{0uz}};
    advpt::testing::assert_equal(cursor.octree(), &octree);
    advpt::testing::assert_range_equal(cursor.path(), std::array{0uz});
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|........");
    const OctreeCursor cursor{ot, std::array{0uz, 1uz}};
    advpt::testing::assert_equal(cursor.octree(), &ot);
    advpt::testing::assert_range_equal(cursor.path(), std::array{0uz, 1uz});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testObservers() {
#if TEST_OBSERVERS
  {
    /* Empty cursor */
    const OctreeCursor emptyCursor;
    advpt::testing::assert_equal(emptyCursor.empty(), true);
  }

  {
    /* End cursor */
    const CellOctree octree;
    const OctreeCursor endCursor{octree, {}};
    advpt::testing::assert_equal(endCursor.end(), true);
  }

  {
    /* Root */
    const CellOctree octree;
    const OctreeCursor cursor{octree};
    advpt::testing::assert_equal(cursor.empty(), false);
    advpt::testing::assert_equal(cursor.end(), false);
    advpt::testing::assert_equal(cursor.currentLevel(), 0uz);
    advpt::testing::assert_equal(cursor.currentStreamIndex(), 0uz);

    const auto cellOpt = cursor.currentCell();
    advpt::testing::assert_equal(cellOpt.has_value(), true);
    const auto cell =
        cellOpt.value(); // NOLINT(bugprone-unchecked-optional-access)
    advpt::testing::assert_equal(cell.mortonIndex().getBits(), 01uz);

    advpt::testing::assert_equal(cursor.firstSibling(), true);
    advpt::testing::assert_equal(cursor.lastSibling(), true);

    advpt::testing::assert_equal(cursor.mortonIndex().getBits(), 01uz);
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|X.......|........");
    const OctreeCursor cursor{ot, std::array{0uz, 1uz}};
    advpt::testing::assert_equal(cursor.empty(), false);
    advpt::testing::assert_equal(cursor.end(), false);
    advpt::testing::assert_equal(cursor.currentLevel(), 1uz);
    advpt::testing::assert_equal(cursor.currentStreamIndex(), 1uz);

    advpt::testing::assert_equal(cursor.currentCell().has_value(), false);

    advpt::testing::assert_equal(cursor.firstSibling(), true);
    advpt::testing::assert_equal(cursor.lastSibling(), false);

    advpt::testing::assert_equal(cursor.mortonIndex().getBits(), 010uz);
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|X.......|........");
    const OctreeCursor cursor{ot, std::array{0uz, 8uz}};
    advpt::testing::assert_equal(cursor.empty(), false);
    advpt::testing::assert_equal(cursor.end(), false);
    advpt::testing::assert_equal(cursor.currentLevel(), 1uz);
    advpt::testing::assert_equal(cursor.currentStreamIndex(), 8uz);

    const auto cellOpt = cursor.currentCell();
    advpt::testing::assert_equal(cellOpt.has_value(), true);

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto cell = cellOpt.value();
    advpt::testing::assert_equal(cell.mortonIndex().getBits(), 017uz);

    advpt::testing::assert_equal(cursor.firstSibling(), false);
    advpt::testing::assert_equal(cursor.lastSibling(), true);

    advpt::testing::assert_equal(cursor.mortonIndex().getBits(), 017uz);
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");
    const OctreeCursor cursor{ot, std::array{0uz, 1uz, 11uz}};
    advpt::testing::assert_equal(cursor.empty(), false);
    advpt::testing::assert_equal(cursor.end(), false);
    advpt::testing::assert_equal(cursor.currentLevel(), 2uz);
    advpt::testing::assert_equal(cursor.currentStreamIndex(), 11uz);

    const auto cellOpt = cursor.currentCell();
    advpt::testing::assert_equal(cellOpt.has_value(), true);

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto cell = cellOpt.value();
    advpt::testing::assert_equal(cell.mortonIndex().getBits(), 0102uz);

    advpt::testing::assert_equal(cursor.firstSibling(), false);
    advpt::testing::assert_equal(cursor.lastSibling(), false);

    advpt::testing::assert_equal(cursor.mortonIndex().getBits(), 0102uz);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testEqualityOperators() {
#if TEST_OBSERVERS
  {
    const OctreeCursor cursor1;
    const OctreeCursor cursor2;

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_true(cursor1 == cursor1);
    advpt::testing::assert_true(cursor1 == cursor2);

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_false(cursor1 != cursor1);
    advpt::testing::assert_false(cursor1 != cursor2);
  }

  {
    const CellOctree octree;
    const OctreeCursor cursor1{octree, {}};
    const OctreeCursor cursor2{octree, {}};

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_true(cursor1 == cursor1);
    advpt::testing::assert_true(cursor1 == cursor2);

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_false(cursor1 != cursor1);
    advpt::testing::assert_false(cursor1 != cursor2);
  }

  {
    const CellOctree octree;

    const OctreeCursor cursor1{octree, std::array{0uz, 1uz}};
    const OctreeCursor cursor2{octree, std::array{0uz, 1uz}};
    const OctreeCursor cursor3{octree, std::array{0uz, 2uz}};

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_true(cursor1 == cursor1);
    advpt::testing::assert_true(cursor1 == cursor2);

    // NOLINTNEXTLINE(misc-redundant-expression)
    advpt::testing::assert_false(cursor1 != cursor1);
    advpt::testing::assert_false(cursor1 != cursor2);

    advpt::testing::assert_false(cursor1 == cursor3);
    advpt::testing::assert_true(cursor1 != cursor3);
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testAscendDescend() {
#if TEST_TRAVERSAL
  {
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");
    OctreeCursor c{ot, std::array{0uz, 1uz, 11uz}};

    advpt::testing::assert_equal(c.currentLevel(), 2uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 11uz);

    c.ascend();
    advpt::testing::assert_equal(c.currentLevel(), 1uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 1uz);

    c.ascend();
    advpt::testing::assert_equal(c.currentLevel(), 0uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 0uz);

    c.descend();
    advpt::testing::assert_equal(c.currentLevel(), 1uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 1uz);

    c.descend();
    advpt::testing::assert_equal(c.currentLevel(), 2uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 9uz);

    c.ascend();
    c.ascend();
    c.ascend();
    advpt::testing::assert_equal(c.end(), true);
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");
    OctreeCursor c{ot, std::array{0uz, 1uz}};

    c.descend(1uz);
    advpt::testing::assert_equal(c.currentLevel(), 2uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 10uz);

    c.ascend();
    c.descend(7uz);
    advpt::testing::assert_equal(c.currentLevel(), 2uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 16uz);

    advpt::testing::throws<std::out_of_range>([&c]() { c.descend(8uz); });
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|........");
    OctreeCursor c{ot, std::array{0uz, 2uz}};

    c.descend();
    advpt::testing::assert_equal(c.currentLevel(), 1uz);
    advpt::testing::assert_equal(c.currentStreamIndex(), 2uz);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testMoveToSiblings() {
#if TEST_TRAVERSAL
  {
    const auto ot = CellOctree::fromDescriptor("R|........");

    OctreeCursor c{ot};
    advpt::testing::assert_equal(c.currentStreamIndex(), 0uz);

    c.previousSibling();
    advpt::testing::assert_equal(c.currentStreamIndex(), 0uz);

    c.nextSibling();
    advpt::testing::assert_equal(c.currentStreamIndex(), 0uz);
  }

  {
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");

    OctreeCursor c{ot, std::array{0uz, 1uz, 9uz}};
    advpt::testing::assert_equal(c.currentStreamIndex(), 9uz);

    for (auto i : std::views::iota(10uz, 17uz)) {
      c.nextSibling();
      advpt::testing::assert_equal(c.currentStreamIndex(), i);
    }

    for (auto i : std::views::iota(0uz, 7uz)) {
      c.previousSibling();
      advpt::testing::assert_equal(c.currentStreamIndex(), 15uz - i);
    }

    for (auto i : std::views::iota(0uz, 8uz)) {
      c.toSibling(i);
      advpt::testing::assert_equal(c.currentStreamIndex(), 9uz + i);
    }

    advpt::testing::throws<std::out_of_range>([&c]() { c.toSibling(8uz); });
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testToEnd() {
#if TEST_TRAVERSAL
  {
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");
    OctreeCursor c{ot, std::array{0uz, 1uz, 11uz}};

    advpt::testing::assert_equal(c.end(), false);

    c.toEnd();
    advpt::testing::assert_equal(c.end(), true);
  }
#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testBasicInterface", &testBasicInterface},
      {"testObservers", &testObservers},
      {"testEqualityOperators", &testEqualityOperators},
      {"testAscendDescend", &testAscendDescend},
      {"testMoveToSiblings", &testMoveToSiblings},
      {"testToEnd", &testToEnd}}
      .run(argc, argv);
}
