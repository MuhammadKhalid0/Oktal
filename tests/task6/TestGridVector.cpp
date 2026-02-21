#include <algorithm>
#include <print>

#include "advpt/testing/Testutils.hpp"

#include "oktal/data/GridVector.hpp"
#include "oktal/io/VtkExport.hpp"
#include "oktal/octree/CellGrid.hpp"
#include "oktal/octree/CellOctree.hpp"

#define TEST_TEMPLATE_INTERFACE true
#define TEST_CONTAINER_SEMANTICS true
#define TEST_GRID_VECTOR_VIEW true
#define TEST_SCALAR_FIELDS true
#define TEST_VIEW_CONVERSION true


namespace {
using namespace oktal;

#if TEST_TEMPLATE_INTERFACE
template <typename Tt, size_t Qv>
concept IsValidGridVector = requires() { typename GridVector<Tt, Qv>; };
#endif

void testTemplateInterface() {
#if TEST_TEMPLATE_INTERFACE
  static_assert(IsValidGridVector<int, 1>);
  static_assert(IsValidGridVector<int, 3>);
  static_assert(IsValidGridVector<double, 1>);

  class A {};

  static_assert(IsValidGridVector<A, 1>);

  static_assert(!IsValidGridVector<int, 0>);
  static_assert(!IsValidGridVector<const int, 1>);
  static_assert(!IsValidGridVector<int &, 1>);

  class B {
  public:
    B() = delete;
  };

  static_assert(!IsValidGridVector<B, 1>);

  static_assert(std::same_as<GridVector<double, 3>::value_type, double>);
  static_assert(GridVector<double, 3>::NUM_COMPONENTS == 3);

  static_assert(std::same_as<GridVector<A, 2>::value_type, A>);
  static_assert(GridVector<A, 2>::NUM_COMPONENTS == 2);
#else
  advpt::testing::dont_compile();
#endif
}

#if TEST_CONTAINER_SEMANTICS

//  NOLINTBEGIN(*)
class AllocCounter {
public:
  AllocCounter() { AllocCounter::constructedObjects_++; }

  AllocCounter(const AllocCounter &) { AllocCounter::constructedObjects_++; }

  AllocCounter(AllocCounter &&) { AllocCounter::constructedObjects_++; }

  AllocCounter &operator=(const AllocCounter &) {
    AllocCounter::copyAssignments_++;
    return *this;
  }

  AllocCounter &operator=(AllocCounter &&) {
    AllocCounter::moveAssignments_++;
    return *this;
  }

  ~AllocCounter() { AllocCounter::destroyedObjects_++; }

  static size_t constructedObjects() { return constructedObjects_; }

  static size_t destroyedObjects() { return destroyedObjects_; }

  static size_t copyAssignments() { return copyAssignments_; }

  static size_t moveAssignments() { return moveAssignments_; }

  static size_t assignments() { return moveAssignments_ + copyAssignments_; }

  static void reset() {
    constructedObjects_ = 0uz;
    destroyedObjects_ = 0uz;
    copyAssignments_ = 0uz;
    moveAssignments_ = 0uz;
  }

private:
  static inline size_t constructedObjects_{0uz};
  static inline size_t destroyedObjects_{0uz};
  static inline size_t copyAssignments_{0uz};
  static inline size_t moveAssignments_{0uz};
};
//  NOLINTEND(*)

#endif

void testContainerSemantics() {
#if TEST_CONTAINER_SEMANTICS
  //  Constructible from cell grid
  static_assert(std::same_as<decltype(GridVector<double, 3>(
                                 std::declval<const CellGrid &>())),
                             GridVector<double, 3>>);

  //  Copyable and movable (includes copy and move assignment)
  static_assert(std::movable<GridVector<double, 3>>);
  static_assert(std::copyable<GridVector<double, 3>>);

  //  Swappable
  static_assert(std::swappable<GridVector<double, 3>>);

  {
    auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("X|........"));
    auto cells = CellGrid::create(ot).levels({1uz}).build();

    {
      AllocCounter::reset();

      {
        const GridVector<AllocCounter, 1> vec{cells};

        //  Expect eight objects to be constructed in-place
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

        //  Expect zero objects to have been destroyed
        advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 0uz);
      }

      //  vec went out of scope -> expect its eight entries to have been
      //  destroyed
      advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 8uz);
    }

    {
      AllocCounter::reset();
      {

        const GridVector<AllocCounter, 1> vec{cells};

        //  Expect eight objects to be constructed in-place
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

        {
          const GridVector<AllocCounter, 1> vec2{vec}; // NOLINT

          //  Expect copy constructor to construct eight new objects
          advpt::testing::assert_equal(AllocCounter::constructedObjects(),
                                       16uz);
        }

        //  Copy goes out of scope -> eight objects destroyed
        advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 8uz);
      }

      //  Original goes out of scope -> eight more objects destroyed
      advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 16uz);
    }

    {
      AllocCounter::reset();
      {

        GridVector<AllocCounter, 1> vec{cells};

        //  Expect eight objects to be constructed in-place
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

        {
          const AllocCounter * const dataPointer = vec.data();

          //  Move construction
          GridVector<AllocCounter, 1> vec2{std::move(vec)};

          //  vec2 takes over data pointer from vec
          advpt::testing::assert_equal((const AllocCounter *) vec2.data(), dataPointer);

          //  Expect move constructor to construct no new objects
          advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

          //  Also, no actual objects must have been copied or moved
          advpt::testing::assert_equal(AllocCounter::assignments(), 0uz);
        }

        //  Moved-into object goes out of scope -> eight objects destroyed
        advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 8uz);
      }

      //  Original goes out of scope -> no more objects destroyed, since they
      //  were moved out
      advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 8uz);
    }

    {
      AllocCounter::reset();
      {
        const GridVector<AllocCounter, 1> vec{cells};

        //  Expect eight objects to be constructed in-place in vec
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

        GridVector<AllocCounter, 1> vec2{cells};

        //  Expect eight more objects to be constructed in-place in vec2
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 16uz);

        const AllocCounter * const dataPointer2 = vec2.data();

        //  Copy assignment
        vec2 = vec;

        //  Copy assignment reuses existing storage of vec2 -> no new objects
        //  are created during the copy operation
        advpt::testing::assert_equal((const AllocCounter *) vec2.data(), dataPointer2);
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 16uz);
        advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 0uz);

        //  Existing objects from vec have been copy-assigned into memory of
        //  vec2
        advpt::testing::assert_equal(AllocCounter::copyAssignments(), 8uz);
      }

      //  vec and vec2 go out of scope -> sixteen objects destroyed
      advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 16uz);
    }

    {
      AllocCounter::reset();
      {
        GridVector<AllocCounter, 1> vec{cells};

        //  Expect eight objects to be constructed in-place in vec
        advpt::testing::assert_equal(AllocCounter::constructedObjects(), 8uz);

        {
          GridVector<AllocCounter, 1> vec2{cells};

          //  Expect eight more objects to be constructed in-place in vec2
          advpt::testing::assert_equal(AllocCounter::constructedObjects(),
                                       16uz);

          AllocCounter *dataPointer = vec.data();

          //  Move assignment
          vec2 = std::move(vec);

          advpt::testing::assert_equal(vec2.data(), dataPointer);

          //  Move assignment never creates new objects
          advpt::testing::assert_equal(AllocCounter::constructedObjects(),
                                       16uz);

          //  Also, it does not copy or move any existing objects
          advpt::testing::assert_equal(AllocCounter::assignments(), 0uz);

          //  At least eight objects are still alive, don't know about the
          //  previous contents of vec
          advpt::testing::assert_less_equal(AllocCounter::destroyedObjects(),
                                            8uz);
        }

        //  vec2 goes out of scope -> at least eight objects destroyed
        advpt::testing::assert_greater_equal(AllocCounter::destroyedObjects(),
                                             8uz);
      }

      //  vec also goes out of scope -> now, sixteen objects must have been
      //  destroyed
      advpt::testing::assert_equal(AllocCounter::destroyedObjects(), 16uz);
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

#if TEST_GRID_VECTOR_VIEW
template <typename Tt, size_t Qv>
concept IsValidGridVectorView = requires() { typename GridVectorView<Tt, Qv>; };
#endif

void testGridVectorViewType() {
#if TEST_GRID_VECTOR_VIEW
  static_assert(IsValidGridVectorView<int, 1>);
  static_assert(IsValidGridVectorView<int, 3>);
  static_assert(IsValidGridVectorView<double, 1>);

  static_assert(IsValidGridVectorView<const int, 1>);
  static_assert(IsValidGridVectorView<const int, 3>);
  static_assert(IsValidGridVectorView<const double, 1>);

  class A {};

  static_assert(IsValidGridVectorView<A, 1>);

  static_assert(!IsValidGridVectorView<int, 0>);
  static_assert(!IsValidGridVectorView<int &, 1>);

  class B {
  public:
    B() = delete;
  };

  static_assert(!IsValidGridVectorView<B, 1>);
#else
  advpt::testing::dont_compile();
#endif
}

void testGridVectorViews() {
#if TEST_GRID_VECTOR_VIEW
  static_assert(
      std::same_as<decltype(std::declval<GridVector<double, 3> &>().view()),
                   GridVectorView<double, 3>>);

  static_assert(std::same_as<
                decltype(std::declval<const GridVector<double, 3> &>().view()),
                GridVectorView<const double, 3>>);

  static_assert(std::same_as<
                decltype(std::declval<GridVector<double, 3> &>().const_view()),
                GridVectorView<const double, 3>>);

  {
    auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("X|........"));
    auto cells = CellGrid::create(ot).levels({1uz}).build();

    GridVector<double, 3> u{cells};
    auto uView = u.view();

    for (auto cell : cells) {
      const size_t cellIdx{cell};

      //  Check for correct linearization strides
      advpt::testing::assert_equal(std::addressof(uView[cellIdx, 1uz]) -
                                       std::addressof(uView[cellIdx, 0uz]),
                                   std::ptrdiff_t(uView.extent(0)));

      advpt::testing::assert_equal(std::addressof(uView[cellIdx, 2uz]) -
                                       std::addressof(uView[cellIdx, 1uz]),
                                   std::ptrdiff_t(uView.extent(0)));

      if (cellIdx > 1) {
        advpt::testing::assert_equal(
            std::addressof(uView[cellIdx, 0]) -
                std::addressof(uView[cellIdx - 1uz, 0]),
            std::ptrdiff_t(1));
      }

      //  Populate vector
      uView[size_t(cell), 0] = cell.center()[0];
      uView[size_t(cell), 1] = cell.center()[1];
      uView[size_t(cell), 2] = cell.center()[2];
    }

    //  Check underlying memory entries
    const std::span<const double> uLinearized{u.data(), u.allocSize()};

    std::vector<double> expected(cells.size() * 3uz, 0.0);
    for (auto c : cells) {
      expected.at(size_t(c) + 0 * cells.size()) = c.center()[0];
      expected.at(size_t(c) + 1 * cells.size()) = c.center()[1];
      expected.at(size_t(c) + 2 * cells.size()) = c.center()[2];
    }

    advpt::testing::assert_range_equal(uLinearized, expected);
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testScalarFields() {
#if TEST_SCALAR_FIELDS

  static_assert(GridVectorView<int, 1>::rank() == 1uz);
  static_assert(GridVectorView<double, 1>::rank() == 1uz);

  {
    auto ot =
        std::make_shared<CellOctree>(CellOctree::fromDescriptor("X|........"));
    auto cells = CellGrid::create(ot).levels({1uz}).build();

    GridVector<double, 1> f{cells};
    auto fView = f.view();

    for (auto c : cells) {
      fView[size_t(c)] = c.center()[0];
    }

    auto fConstView = f.const_view();
    for (auto c : cells) {
      advpt::testing::assert_equal(fConstView[size_t(c)], c.center()[0]);
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testViewConversion() {
#if TEST_VIEW_CONVERSION
  static_assert(
      std::convertible_to<GridVector<double, 3>, GridVectorView<double, 3>>);

  static_assert(std::convertible_to<GridVector<double, 3>,
                                    GridVectorView<const double, 3>>);

  static_assert(
      std::convertible_to<GridVector<int, 1>, GridVectorView<int, 1>>);

  static_assert(
      std::convertible_to<GridVector<int, 1>, GridVectorView<const int, 1>>);
#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testTemplateInterface", &testTemplateInterface},
      {"testContainerSemantics", &testContainerSemantics},
      {"testGridVectorViewType", &testGridVectorViewType},
      {"testGridVectorViews", &testGridVectorViews},
      {"testScalarFields", &testScalarFields},
      {"testViewConversion", &testViewConversion},
  }
      .run(argc, argv);
}
