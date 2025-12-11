#include "advpt/testing/Testutils.hpp"

#include "oktal/octree/CellOctree.hpp"

#include <concepts>
#include <iterator>
#include <ranges>

#define TEST_POLICY_CONCEPT true
#define TEST_ITERATOR_TEMPLATE true
#define TEST_RANGE_TEMPLATE true
#define TEST_DFS_POLICY true
#define TEST_HORIZONTAL_POLICY true

namespace {
using namespace oktal;

void testPolicyConcept() {
#if TEST_POLICY_CONCEPT

  {
    class Empty {};

    static_assert(!OctreeIteratorPolicy<Empty>);
  }

  {
    //  NOLINTBEGIN
    class NotSemiregular {
    public:
      NotSemiregular() = default;
      NotSemiregular(const NotSemiregular &) = delete;

      void advance(OctreeCursor &) const {}
    };
    //  NOLINTEND

    static_assert(!OctreeIteratorPolicy<NotSemiregular>);
  }

  {
    class MissingAdvance {
    public:
      MissingAdvance() = default;
    };

    static_assert(!OctreeIteratorPolicy<MissingAdvance>);
  }

  {
    class WrongAdvance {
    public:
      WrongAdvance() = default;

      //  NOLINTNEXTLINE
      int advance(OctreeCursor &) const { return 42; }
    };

    static_assert(!OctreeIteratorPolicy<WrongAdvance>);
  }

  {
    class CorrectPolicy {
    public:
      CorrectPolicy() = default;

      //  NOLINTNEXTLINE
      void advance(OctreeCursor &) const {}
    };

    static_assert(OctreeIteratorPolicy<CorrectPolicy>);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testIteratorTemplate() {
#if TEST_ITERATOR_TEMPLATE
  class DummyPolicy {
  public:
    //  NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void advance(OctreeCursor &c) const {
      if (!c.lastSibling()) {
        c.nextSibling();
      } else {
        c.toEnd();
      }
    }
  };

  using DummyIterator = OctreeIterator<DummyPolicy>;

  static_assert(std::forward_iterator<DummyIterator>);

  auto ot = CellOctree::fromDescriptor("R|........");

  const OctreeCursor start{ot, std::array{0uz, 1uz}};
  const OctreeCursor end{ot, {}};

  size_t currentIdx = 1uz;
  for (auto it = DummyIterator{start, DummyPolicy{}};
       it != DummyIterator{end, DummyPolicy{}}; ++it) {
    auto cell = *it;
    advpt::testing::assert_equal(cell.streamIndex(), currentIdx);
    currentIdx++;
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testRangeTemplate() {
#if TEST_RANGE_TEMPLATE
  class DummyPolicy {
  public:
    //  NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void advance(OctreeCursor &c) const {
      if (!c.lastSibling()) {
        c.nextSibling();
      } else {
        c.toEnd();
      }
    }
  };

  using DummyRange = OctreeCellsRange<DummyPolicy>;
  static_assert(std::ranges::forward_range<DummyRange>);

  auto ot = CellOctree::fromDescriptor("R|........");
  const DummyRange range{OctreeCursor{ot, std::array{0uz, 1uz}},
                         OctreeCursor{ot, {}}, DummyPolicy{}};

  for (auto [idx, cell] : std::views::zip(std::views::iota(1uz, 9uz), range)) {
    advpt::testing::assert_equal(cell.streamIndex(), idx);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testPreOrderDepthFirst() {
#if TEST_DFS_POLICY
  static_assert(
      std::ranges::forward_range<decltype(std::declval<oktal::CellOctree &>()
                                              .preOrderDepthFirstRange())>);

  auto mapToDfsIndices = std::views::transform(
      [](const CellOctree::CellView &c) { return c.mortonIndex().getBits(); });

  {
    //  Empty tree
    const CellOctree ot{};
    auto expectedOrder = {01};
    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

  {
    //  One refinement level
    const auto ot = CellOctree::fromDescriptor("R|........");
    const auto expectedOrder = {01, 010, 011, 012, 013, 014, 015, 016, 017};
    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

  {
    //  Two levels, partially refined
    const auto ot = CellOctree::fromDescriptor("R|R.......|........");

    const auto expectedOrder = {01,
                                // first child of root
                                010,
                                // its children
                                0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
                                // remaining children of root
                                011, 012, 013, 014, 015, 016, 017};

    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

  {
    //  Three levels, rightmost refined
    const auto ot = CellOctree::fromDescriptor("R|.......R|.......R|........");
    const auto expectedOrder = {01,
                                //  First level
                                010, 011, 012, 013, 014, 015, 016, 017,
                                //  Second level
                                0170, 0171, 0172, 0173, 0174, 0175, 0176, 0177,
                                //  Third level
                                01770, 01771, 01772, 01773, 01774, 01775, 01776,
                                01777};

    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

  {
    //  Three levels, partially refined
    const auto ot = CellOctree::fromDescriptor(
        "R|...R...R|.....R.........R|................");

    const auto expectedOrder = {
        01,
        //
        010, 011, 012, 013,
        //
        0130, 0131, 0132, 0133, 0134, 0135,
        //
        01350, 01351, 01352, 01353, 01354, 01355, 01356, 01357,
        //
        0136, 0137,
        //
        014, 015, 016, 017,
        //
        0170, 0171, 0172, 0173, 0174, 0175, 0176, 0177,
        //
        01770, 01771, 01772, 01773, 01774, 01775, 01776, 01777};

    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testPreOrderDepthFirstWithPhantoms() {
#if TEST_DFS_POLICY
  auto mapToDfsIndices = std::views::transform(
      [](const CellOctree::CellView &c) { return c.mortonIndex().getBits(); });

  {
    //  Only root, root is phantom
    const auto ot = CellOctree::fromDescriptor("P");
    advpt::testing::assert_true(
        std::ranges::empty(ot.preOrderDepthFirstRange()));
  }

  {
    //  One refinement level
    const auto ot = CellOctree::fromDescriptor("X|PP....PP");
    const auto expectedOrder = {012, 013, 014, 015};
    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

  {
    //  Two levels, partially refined
    const auto ot = CellOctree::fromDescriptor("X|X.....PP|....PP..");

    const auto expectedOrder = {0100, 0101, 0102, 0103, 0106, 0107,
                                011,  012,  013,  014,  015};

    advpt::testing::assert_range_equal(
        ot.preOrderDepthFirstRange() | mapToDfsIndices, expectedOrder);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testHorizontal() {
#if TEST_HORIZONTAL_POLICY
  static_assert(std::ranges::forward_range<
                decltype(std::declval<CellOctree &>().horizontalRange(
                    std::declval<size_t>()))>);

  auto mapToDfsIndices = std::views::transform(
      [](const CellOctree::CellView &c) { return c.mortonIndex().getBits(); });

  {
    //  Empty tree
    const CellOctree ot{};

    const auto expectedOrder = {01};
    advpt::testing::assert_range_equal(
        ot.horizontalRange(0uz) | mapToDfsIndices, expectedOrder);

    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(1uz)));
    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(2uz)));
    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(3uz)));
  }

  {
    //  One refinement level
    auto ot = CellOctree::fromDescriptor("R|........");

    {
      const auto expectedOrder = {01};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(0) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {010, 011, 012, 013, 014, 015, 016, 017};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(1uz) | mapToDfsIndices, expectedOrder);
    }

    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(2uz)));
    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(3uz)));
  }

  {
    //  Two refinement levels
    const auto ot = CellOctree::fromDescriptor("R|R......R|................");

    {
      const auto expectedOrder = {01};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(0) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {010, 011, 012, 013, 014, 015, 016, 017};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(1) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {0100, 0101, 0102, 0103, 0104, 0105,
                                  0106, 0107, 0170, 0171, 0172, 0173,
                                  0174, 0175, 0176, 0177};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(2) | mapToDfsIndices, expectedOrder);
    }

    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(3)));
  }

  {
    //  Three refinement levels
    const auto ot = CellOctree::fromDescriptor(
        "R|R.R.R.R.|.....R.....................R....|................");

    {
      const auto expectedOrder = {01};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(0) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {010, 011, 012, 013, 014, 015, 016, 017};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(1) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {
          0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107, 0120, 0121, 0122,
          0123, 0124, 0125, 0126, 0127, 0140, 0141, 0142, 0143, 0144, 0145,
          0146, 0147, 0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167,
      };
      advpt::testing::assert_range_equal(
          ot.horizontalRange(2) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {01050, 01051, 01052, 01053, 01054, 01055,
                                  01056, 01057, 01630, 01631, 01632, 01633,
                                  01634, 01635, 01636, 01637};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(3) | mapToDfsIndices, expectedOrder);
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testHorizontalWithPhantoms() {
#if TEST_HORIZONTAL_POLICY
  auto mapToDfsIndices = std::views::transform(
      [](const CellOctree::CellView &c) { return c.mortonIndex().getBits(); });

  {
    //  Only root, root is phantom
    const auto ot = CellOctree::fromDescriptor("P");
    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(0uz)));
  }

  {
    //  One refinement level
    auto ot = CellOctree::fromDescriptor("R|..PP..PP");

    {
      const auto expectedOrder = {010, 011, 014, 015};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(1uz) | mapToDfsIndices, expectedOrder);
    }

    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(2uz)));
    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(3uz)));
  }

  {
    //  Two refinement levels
    const auto ot = CellOctree::fromDescriptor("X|X..PP..X|P.....PP.P.P.P.P");

    {
      advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(0uz)));
    }

    {
      const auto expectedOrder = {011, 012, 015, 016};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(1) | mapToDfsIndices, expectedOrder);
    }

    {
      const auto expectedOrder = {0101, 0102, 0103, 0104, 0105,
                                  0170, 0172, 0174, 0176};
      advpt::testing::assert_range_equal(
          ot.horizontalRange(2) | mapToDfsIndices, expectedOrder);
    }

    advpt::testing::assert_true(std::ranges::empty(ot.horizontalRange(3)));
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testPolicyConcept", &testPolicyConcept},
      {"testIteratorTemplate", &testIteratorTemplate},
      {"testRangeTemplate", &testRangeTemplate},
      {"testPreOrderDepthFirst", &testPreOrderDepthFirst},
      {"testPreOrderDepthFirstWithPhantoms",
       &testPreOrderDepthFirstWithPhantoms},
      {"testHorizontal", &testHorizontal},
      {"testHorizontalWithPhantoms", &testHorizontalWithPhantoms}}
      .run(argc, argv);
}
