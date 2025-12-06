#include "advpt/testing/Testutils.hpp"

#include "oktal/octree/CellOctree.hpp"

#include <ranges>

#define TEST_NODE true
#define TEST_NODES_STREAM true
#define TEST_FROM_DESCRIPTOR true


namespace {

using namespace oktal;

void testNode() {
#if TEST_NODE
  {
    const CellOctree::Node node;
    advpt::testing::assert_false(node.isRefined());
    advpt::testing::assert_false(node.isPhantom());
  }

  {
    CellOctree::Node node{true, false, 17};

    advpt::testing::assert_true(node.isRefined());
    advpt::testing::assert_false(node.isPhantom());

    advpt::testing::assert_equal(node.childrenStartIndex(), 17uz);
    for (auto i : std::views::iota(0uz, 8uz)) {
      advpt::testing::assert_equal(node.childIndex(i), 17uz + i);
    }

    node.setRefined(false);
    advpt::testing::assert_false(node.isRefined());
    advpt::testing::assert_false(node.isPhantom());

    node.setPhantom(true);
    advpt::testing::assert_false(node.isRefined());
    advpt::testing::assert_true(node.isPhantom());

    node.setRefined(true);
    node.setChildrenStartIndex(25);
    advpt::testing::assert_true(node.isRefined());
    advpt::testing::assert_true(node.isPhantom());

    advpt::testing::assert_equal(node.childrenStartIndex(), 25uz);
    for (auto i : std::views::iota(0uz, 8uz)) {
      advpt::testing::assert_equal(node.childIndex(i), 25uz + i);
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testTrivialTree() {
#if TEST_NODES_STREAM
  static_assert(std::same_as<decltype(std::declval<CellOctree>().nodesStream()),
                             std::span<const CellOctree::Node>>);
  static_assert(std::same_as<decltype(std::declval<CellOctree>().nodesStream(
                                 std::declval<size_t>())),
                             std::span<const CellOctree::Node>>);

  static_assert(
      std::same_as<decltype(std::declval<const CellOctree &>().nodesStream()),
                   std::span<const CellOctree::Node>>);
  static_assert(
      std::same_as<decltype(std::declval<const CellOctree &>().nodesStream(
                       std::declval<size_t>())),
                   std::span<const CellOctree::Node>>);

  const CellOctree ot;

  advpt::testing::assert_equal(ot.numberOfNodes(), 1uz);
  advpt::testing::assert_equal(ot.numberOfNodes(0uz), 1uz);
  advpt::testing::assert_equal(ot.numberOfNodes(1uz), 0uz);

  advpt::testing::assert_equal(ot.numberOfLevels(), 1uz);
  advpt::testing::assert_equal(ot.nodesStream().size(), 1uz);
  advpt::testing::assert_equal(ot.nodesStream(0uz).size(), 1uz);
  advpt::testing::assert_true(ot.nodesStream(1uz).empty());

  advpt::testing::assert_false(ot.nodesStream()[0].isRefined());
  advpt::testing::assert_false(ot.nodesStream()[0].isPhantom());
#else
  advpt::testing::dont_compile();
#endif
}

void testFromDescriptor() {
#if TEST_FROM_DESCRIPTOR
  {
    const std::string_view descr{"."};
    const CellOctree ot = CellOctree::fromDescriptor(descr);

    advpt::testing::assert_equal(ot.numberOfNodes(), 1uz);
    advpt::testing::assert_equal(ot.numberOfNodes(0uz), 1uz);
    advpt::testing::assert_equal(ot.numberOfNodes(1uz), 0uz);

    advpt::testing::assert_equal(ot.numberOfLevels(), 1uz);
    advpt::testing::assert_equal(ot.nodesStream().size(), 1uz);
    advpt::testing::assert_equal(ot.nodesStream(0uz).size(), 1uz);
    advpt::testing::assert_true(ot.nodesStream(1uz).empty());

    advpt::testing::assert_false(ot.nodesStream()[0].isRefined());
    advpt::testing::assert_false(ot.nodesStream()[0].isPhantom());
  }

  {
    const std::string_view descr{"R|........"};
    const CellOctree ot = CellOctree::fromDescriptor(descr);

    advpt::testing::assert_equal(ot.numberOfNodes(), 9uz);
    advpt::testing::assert_equal(ot.numberOfNodes(0uz), 1uz);
    advpt::testing::assert_equal(ot.numberOfNodes(1uz), 8uz);

    advpt::testing::assert_equal(ot.numberOfLevels(), 2uz);
    advpt::testing::assert_equal(ot.nodesStream().size(), 9uz);
    advpt::testing::assert_equal(ot.nodesStream(0uz).size(), 1uz);
    advpt::testing::assert_equal(ot.nodesStream(1uz).size(), 8uz);

    advpt::testing::assert_true(ot.nodesStream()[0].isRefined());
    advpt::testing::assert_false(ot.nodesStream()[0].isPhantom());
    advpt::testing::assert_equal(ot.nodesStream()[0].childrenStartIndex(), 1uz);

    for (auto branch : std::views::iota(0uz, 8uz)) {
      advpt::testing::assert_false(ot.nodesStream(1uz)[branch].isRefined());
      advpt::testing::assert_false(ot.nodesStream(1uz)[branch].isPhantom());
    }
  }

  {
    const std::string_view descr{"X|....R..R|.P.P.P.P.P.P.P.P"};
    const CellOctree ot = CellOctree::fromDescriptor(descr);

    advpt::testing::assert_equal(ot.numberOfNodes(), 25uz);
    advpt::testing::assert_equal(ot.numberOfNodes(0uz), 1uz);
    advpt::testing::assert_equal(ot.numberOfNodes(1uz), 8uz);
    advpt::testing::assert_equal(ot.numberOfNodes(2uz), 16uz);

    advpt::testing::assert_equal(ot.numberOfLevels(), 3uz);
    advpt::testing::assert_equal(ot.nodesStream().size(), 25uz);
    advpt::testing::assert_equal(ot.nodesStream(0uz).size(), 1uz);
    advpt::testing::assert_equal(ot.nodesStream(1uz).size(), 8uz);
    advpt::testing::assert_equal(ot.nodesStream(2uz).size(), 16uz);

    advpt::testing::assert_true(ot.nodesStream()[0].isRefined());
    advpt::testing::assert_true(ot.nodesStream()[0].isPhantom());
    advpt::testing::assert_equal(ot.nodesStream()[0].childrenStartIndex(), 1uz);

    for (auto idx : std::views::iota(0uz, 8uz)) {
      if (idx == 4 || idx == 7) {
        advpt::testing::assert_true(ot.nodesStream(1uz)[idx].isRefined());
        advpt::testing::assert_equal(
            ot.nodesStream(1uz)[idx].childrenStartIndex(),
            idx == 4uz ? 9uz : 17uz);
      } else {
        advpt::testing::assert_false(ot.nodesStream(1uz)[idx].isRefined());
      }

      advpt::testing::assert_false(ot.nodesStream(1uz)[idx].isPhantom());
    }

    for (auto idx : std::views::iota(0uz, 16uz)) {
      if (idx % 2 == 1) {
        advpt::testing::assert_true(ot.nodesStream(2uz)[idx].isPhantom());
      } else {
        advpt::testing::assert_false(ot.nodesStream(2uz)[idx].isPhantom());
      }
      advpt::testing::assert_false(ot.nodesStream(2uz)[idx].isRefined());
    }
  }

  {
    const std::string_view descr{
        "R|R.R.R.R.|........................PPPPPPPX|........"};
    const CellOctree ot = CellOctree::fromDescriptor(descr);

    advpt::testing::assert_equal(ot.numberOfNodes(), 49uz);
    advpt::testing::assert_equal(ot.numberOfNodes(0uz), 1uz);
    advpt::testing::assert_equal(ot.numberOfNodes(1uz), 8uz);
    advpt::testing::assert_equal(ot.numberOfNodes(2uz), 32uz);
    advpt::testing::assert_equal(ot.numberOfNodes(3uz), 8uz);
    advpt::testing::assert_equal(ot.numberOfNodes(4uz), 0uz);

    advpt::testing::assert_equal(ot.numberOfLevels(), 4uz);
    advpt::testing::assert_equal(ot.nodesStream().size(), 49uz);
    advpt::testing::assert_equal(ot.nodesStream(0uz).size(), 1uz);
    advpt::testing::assert_equal(ot.nodesStream(1uz).size(), 8uz);
    advpt::testing::assert_equal(ot.nodesStream(2uz).size(), 32uz);
    advpt::testing::assert_equal(ot.nodesStream(3uz).size(), 8uz);
    advpt::testing::assert_equal(ot.nodesStream(4uz).size(), 0uz);

    advpt::testing::assert_true(ot.nodesStream()[0].isRefined());
    advpt::testing::assert_false(ot.nodesStream()[0].isPhantom());
    advpt::testing::assert_equal(ot.nodesStream()[0].childrenStartIndex(), 1uz);

    advpt::testing::assert_equal(ot.numberOfNodes(), 49uz);

    for (auto idx : std::views::iota(0uz, 8uz)) {
      if (idx % 2 == 0) {
        advpt::testing::assert_true(ot.nodesStream(1uz)[idx].isRefined());
        advpt::testing::assert_equal(
            ot.nodesStream(1uz)[idx].childrenStartIndex(), 9 + 8 * (idx / 2));
      } else {
        advpt::testing::assert_false(ot.nodesStream(1uz)[idx].isRefined());
      }

      advpt::testing::assert_false(ot.nodesStream(1uz)[idx].isPhantom());
    }

    for (auto idx : std::views::iota(0uz, 24uz)) {
      advpt::testing::assert_false(ot.nodesStream(2uz)[idx].isRefined());
      advpt::testing::assert_false(ot.nodesStream(2uz)[idx].isPhantom());
    }

    for (auto idx : std::views::iota(24uz, 31uz)) {
      advpt::testing::assert_false(ot.nodesStream(2uz)[idx].isRefined());
      advpt::testing::assert_true(ot.nodesStream(2uz)[idx].isPhantom());
    }

    advpt::testing::assert_true(ot.nodesStream(2uz)[31uz].isRefined());
    advpt::testing::assert_true(ot.nodesStream(2uz)[31uz].isPhantom());
    advpt::testing::assert_equal(ot.nodesStream(2uz)[31uz].childrenStartIndex(),
                                 41uz);

    for (auto idx : std::views::iota(0uz, 8uz)) {
      advpt::testing::assert_false(ot.nodesStream(3uz)[idx].isRefined());
      advpt::testing::assert_false(ot.nodesStream(3uz)[idx].isPhantom());
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testInvalidDescriptors() {
#if TEST_FROM_DESCRIPTOR

  //  Invalid descriptors
  {
    advpt::testing::throws<std::invalid_argument>(
        []() { auto _ = CellOctree::fromDescriptor("R|......."); });

    advpt::testing::throws<std::invalid_argument>(
        []() { auto _ = CellOctree::fromDescriptor("R|.Z......"); });

    advpt::testing::throws<std::invalid_argument>(
        []() { auto _ = CellOctree::fromDescriptor("X|........|........"); });
  }
#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testNode", &testNode},
      {"testTrivialTree", &testTrivialTree},
      {"testFromDescriptor", &testFromDescriptor},
    {"testInvalidDescriptors", &testInvalidDescriptors}}
      .run(argc, argv);
}
