#include "advpt/testing/Testutils.hpp"

#include "oktal/geometry/Box.hpp"

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <utility>

#define TEST_BASIC_INTERFACE true
#define TEST_OBSERVERS true


namespace {

using namespace oktal;

void testConstructorsAndGetters() {
#if TEST_BASIC_INTERFACE
  static_assert(std::same_as<Box<>, Box<double>>);

  {
    const Box b{{0., 0., 0.}, {1., 1., 1.}};
    advpt::testing::assert_equal(b.minCorner(), Vec<double, 3>(0.));
    advpt::testing::assert_equal(b.maxCorner(), Vec<double, 3>(1.));
  }

  {
    const Box<float> b{{-1.f, -2.f, -3.f}, {0.f, 2.f, 4.f}};
    advpt::testing::assert_equal(b.minCorner(),
                                 Vec<float, 3>{-1.f, -2.f, -3.f});
    advpt::testing::assert_equal(b.maxCorner(), Vec<float, 3>{0.f, 2.f, 4.f});
  }

  {
    const Box<uint64_t> b{{14UL, 17UL, 9UL}, {21UL, 20UL, 12UL}};
    advpt::testing::assert_equal(b.minCorner(),
                                 Vec<uint64_t, 3>{14UL, 17UL, 9UL});
    advpt::testing::assert_equal(b.maxCorner(),
                                 Vec<uint64_t, 3>{21UL, 20UL, 12UL});
  }

  {
    const Box b{};
    advpt::testing::assert_equal(b.minCorner(), Vec<double, 3>(0.));
    advpt::testing::assert_equal(b.maxCorner(), Vec<double, 3>(0.));
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testSetters() {
#if TEST_BASIC_INTERFACE
  static_assert(std::same_as<Box<>, Box<double>>);
  static_assert(std::same_as<Box<double>::vector_type, Vec<double, 3>>);
  static_assert(std::same_as<Box<float>::vector_type, Vec<float, 3>>);
  static_assert(std::same_as<Box<uint16_t>::vector_type, Vec<uint16_t, 3>>);

  {
    Box b{{0., 0., 0.}, {1., 1., 1.}};

    b.minCorner() = {0.25, -0.5, 0.75};
    b.maxCorner() = {3.1, 2.5, 1.2};

    advpt::testing::assert_equal(b.minCorner(),
                                 Vec<double, 3>{0.25, -0.5, 0.75});
    advpt::testing::assert_equal(b.maxCorner(), Vec<double, 3>{3.1, 2.5, 1.2});
  }

  {
    Box<float> b{{-1.f, -2.f, -3.f}, {0.f, 2.f, 4.f}};

    b.minCorner() = {0.25f, -0.5f, 0.75f};
    b.maxCorner() = {3.1f, 2.5f, 1.2f};

    advpt::testing::assert_equal(b.minCorner(),
                                 Vec<float, 3>{0.25f, -0.5f, 0.75f});
    advpt::testing::assert_equal(b.maxCorner(),
                                 Vec<float, 3>{3.1f, 2.5f, 1.2f});
  }

  {
    Box<uint64_t> b{{14UL, 17UL, 9UL}, {21UL, 20UL, 12UL}};

    b.minCorner() = {12UL, 5UL, 3UL};
    b.maxCorner() = {16UL, 8UL, 10UL};

    advpt::testing::assert_equal(b.minCorner(),
                                 Vec<uint64_t, 3>{12UL, 5UL, 3UL});
    advpt::testing::assert_equal(b.maxCorner(),
                                 Vec<uint64_t, 3>{16UL, 8UL, 10UL});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testObservers() {
#if TEST_OBSERVERS

  {
    const Box b;

    advpt::testing::assert_equal(b.center(), Vec<double, 3>(0.));
    advpt::testing::assert_equal(b.extents(), Vec<double, 3>(0.));
    advpt::testing::assert_equal(b.volume(), 0.);
  }

  {
    const Box b{{0., 0., 0.}, {3., 4., 5.}};

    advpt::testing::assert_equal(b.center(), Vec<double, 3>{1.5, 2.0, 2.5});
    advpt::testing::assert_equal(b.extents(), Vec<double, 3>{3., 4., 5.});
    advpt::testing::assert_equal(b.volume(), 60.);
  }

  {
    const Box<float> b{{1.5f, 2.3f, 3.1f}, {2.1f, 4.5f, 5.f}};

    advpt::testing::with_tolerance{0., 1e-6}.assert_allclose(
        b.center(), Vec<float, 3>{1.8f, 3.4f, 4.05f});
    advpt::testing::with_tolerance{0., 1e-6}.assert_allclose(
        b.extents(), Vec<float, 3>{0.6f, 2.2f, 1.9f});
    advpt::testing::with_tolerance{0., 1e-6}.assert_close(b.volume(), 2.508f);
  }

  {
    const Box<int32_t> b{{3, 6, 7}, {10, 12, 10}};

    advpt::testing::assert_equal(b.extents(), Vec<int32_t, 3>{7, 6, 3});
    advpt::testing::assert_equal(b.volume(), 126);
  }

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testConstructorsAndGetters", &testConstructorsAndGetters},
      {"testSetters", &testSetters},
      {"testObservers", &testObservers}}
      .run(argc, argv);
}
