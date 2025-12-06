#include "advpt/testing/Testutils.hpp"

#include "oktal/geometry/Vec.hpp"

#include <concepts>
#include <cstdint>
#include <ranges>
#include <span>
#include <utility>

/**
 * ENABLE/DISABLE TESTS
 *
 * After finishing a part of the implementation,
 * enable the corresponding tests by setting the
 * appropriate macro to `true`:
 */

#define TEST_BASIC_INTERFACE true
#define TEST_CONSTRUCTORS true
#define TEST_ELEMENT_ACCESS true
#define TEST_EQUALITY true
#define TEST_RANGE true
#define TEST_VECTOR_OPS true
#define TEST_AUGMENTED_ASSIGNMENTS true
#define TEST_MAGNITUDE true
#define TEST_CONVERTING_CTOR true
#define TEST_TYPE_ALIASES true

namespace {

using namespace oktal;

void testStackAllocation() {
#if TEST_BASIC_INTERFACE
  static_assert(sizeof(oktal::Vec<int32_t, 1>) == sizeof(int32_t));
  static_assert(sizeof(oktal::Vec<int32_t, 3>) == 3 * sizeof(int32_t));
  static_assert(sizeof(oktal::Vec<double, 1>) == sizeof(double));
  static_assert(sizeof(oktal::Vec<double, 4>) == 4 * sizeof(double));
  static_assert(sizeof(oktal::Vec<float, 1>) == sizeof(float));
  static_assert(sizeof(oktal::Vec<float, 4>) == 4 * sizeof(float));

  static_assert(std::is_trivially_copy_constructible_v<oktal::Vec<int32_t, 3>>);
  static_assert(std::is_trivially_move_constructible_v<oktal::Vec<int32_t, 3>>);

  static_assert(std::is_trivially_copy_constructible_v<oktal::Vec<float, 3>>);
  static_assert(std::is_trivially_move_constructible_v<oktal::Vec<float, 3>>);

  static_assert(std::is_trivially_copy_constructible_v<oktal::Vec<double, 3>>);
  static_assert(std::is_trivially_move_constructible_v<oktal::Vec<double, 3>>);
#else
  advpt::testing::dont_compile();
#endif
}

void testSizeAndData() {
#if TEST_BASIC_INTERFACE
  static_assert(
      std::same_as<decltype(std::declval<oktal::Vec<float, 3>>().data()),
                   float *>);

  static_assert(
      std::same_as<decltype(std::declval<oktal::Vec<double, 3>>().data()),
                   double *>);

  static_assert(
      std::same_as<decltype(std::declval<const oktal::Vec<float, 3>>().data()),
                   const float *>);

  static_assert(
      std::same_as<decltype(std::declval<const oktal::Vec<double, 3>>().data()),
                   const double *>);

  {
    const Vec<int32_t, 1> v;
    advpt::testing::assert_equal(v.size(), 1uz);
    advpt::testing::assert_equal((void *)v.data(), (void *)&v);
  }

  {
    const Vec<int32_t, 2> v;
    advpt::testing::assert_equal(v.size(), 2uz);
    advpt::testing::assert_equal((void *)v.data(), (void *)&v);
  }

  {
    const Vec<int32_t, 3> v;
    advpt::testing::assert_equal(v.size(), 3uz);
    advpt::testing::assert_equal((void *)v.data(), (void *)&v);
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testDefaultCtor() {
#if TEST_CONSTRUCTORS
  {
    Vec<int32_t, 1> vec;
    advpt::testing::assert_range_equal(std::span(vec.data(), 1),
                                       std::array{int32_t(0)});
  }

  {
    Vec<uint64_t, 3> vec;
    advpt::testing::assert_range_equal(
        std::span(vec.data(), 3),
        std::array{uint64_t(0), uint64_t(0), uint64_t(0)});
  }

  {
    Vec<uint8_t, 6> vec;
    advpt::testing::assert_range_equal(std::span(vec.data(), 6),
                                       std::array{uint8_t(0), uint8_t(0),
                                                  uint8_t(0), uint8_t(0),
                                                  uint8_t(0), uint8_t(0)});
  }

  {
    Vec<float, 2> vec;
    advpt::testing::assert_range_equal(std::span(vec.data(), 2),
                                       std::array{0.f, 0.f});
  }

  {
    Vec<double, 4> vec;
    advpt::testing::assert_range_equal(std::span(vec.data(), 4),
                                       std::array{0., 0., 0., 0.});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testConstantCtor() {
#if TEST_CONSTRUCTORS
  {
    Vec<int64_t, 1> vec(971);
    advpt::testing::assert_range_equal(std::span(vec.data(), 1),
                                       std::array{int64_t(971)});
  }

  {
    Vec<uint32_t, 3> vec(311);
    advpt::testing::assert_range_equal(
        std::span(vec.data(), 3),
        std::array{uint32_t(311), uint32_t(311), uint32_t(311)});
  }

  {
    Vec<double, 6> vec(-3.12);
    advpt::testing::assert_range_equal(
        std::span(vec.data(), 6),
        std::array{-3.12, -3.12, -3.12, -3.12, -3.12, -3.12});
  }

  {
    Vec<float, 4> vec(17.23f);
    advpt::testing::assert_range_equal(
        std::span(vec.data(), 4), std::array{17.23f, 17.23f, 17.23f, 17.23f});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testInitListCtor() {
#if TEST_CONSTRUCTORS
  {
    Vec<int32_t, 1> v{12};
    advpt::testing::assert_range_equal(std::span(v.data(), 1),
                                       std::array{int32_t(12)});
  }

  {
    Vec<int16_t, 3> v{14, 31, 9};
    advpt::testing::assert_range_equal(
        std::span(v.data(), 3),
        std::array{int16_t(14), int16_t(31), int16_t(9)});
  }

  {
    Vec<float, 5> v{3.12f, 5.1f, -14.9f, 9.9f, 2.1f};
    advpt::testing::assert_range_equal(
        std::span(v.data(), 5), std::array{3.12f, 5.1f, -14.9f, 9.9f, 2.1f});
  }

  {
    Vec<float, 5> v{3.12f, -14.9f, 2.1f};
    advpt::testing::assert_range_equal(
        std::span(v.data(), 5), std::array{3.12f, -14.9f, 2.1f, 0.f, 0.f});
  }

  {
    Vec<float, 4> v{3.12f, 5.1f, -14.9f, 9.9f, 2.1f, 17.2f, -918.67f};
    advpt::testing::assert_range_equal(std::span(v.data(), 4),
                                       std::array{3.12f, 5.1f, -14.9f, 9.9f});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testElementAccess() {
#if TEST_ELEMENT_ACCESS

  {
    Vec<int64_t, 1> v{13};
    advpt::testing::assert_equal(v[0], int64_t(13));
  }

  {
    Vec<uint64_t, 4> v{5, 92, 3, 11};
    advpt::testing::assert_equal(v[0], uint64_t(5));
    advpt::testing::assert_equal(v[1], uint64_t(92));
    advpt::testing::assert_equal(v[2], uint64_t(3));
    advpt::testing::assert_equal(v[3], uint64_t(11));
  }

  {
    const Vec<uint64_t, 4> v{5, 92, 3, 11};
    advpt::testing::assert_equal(v[0], uint64_t(5));
    advpt::testing::assert_equal(v[1], uint64_t(92));
    advpt::testing::assert_equal(v[2], uint64_t(3));
    advpt::testing::assert_equal(v[3], uint64_t(11));
  }

  {
    Vec<uint64_t, 4> v{5, 92, 3, 11};

    v[0] = 27;
    v[1] = 13;
    v[2] = 55;
    v[3] = 0;

    advpt::testing::assert_equal(v[0], uint64_t(27));
    advpt::testing::assert_equal(v[1], uint64_t(13));
    advpt::testing::assert_equal(v[2], uint64_t(55));
    advpt::testing::assert_equal(v[3], uint64_t(0));
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testEquality() {
#if TEST_EQUALITY

  advpt::testing::assert_true(Vec<int32_t, 2>{4, 5} == Vec<int32_t, 2>{4, 5});
  advpt::testing::assert_false(Vec<int32_t, 2>{4, 5} == Vec<int32_t, 2>{3, 5});
  advpt::testing::assert_false(Vec<int32_t, 2>{4, 5} == Vec<int32_t, 2>{4, -5});

  advpt::testing::assert_true(Vec<double, 3>{0.1, 0.3, -1.2} ==
                              Vec<double, 3>{0.1, 0.3, -1.2});
  advpt::testing::assert_false(Vec<double, 3>{0.1, 0.3, -1.2} ==
                               Vec<double, 3>{0.11, 0.3, -1.2});

  advpt::testing::assert_false(Vec<int32_t, 2>{4, 5} != Vec<int32_t, 2>{4, 5});
  advpt::testing::assert_true(Vec<int32_t, 2>{4, 5} != Vec<int32_t, 2>{3, 5});
  advpt::testing::assert_true(Vec<int32_t, 2>{4, 5} != Vec<int32_t, 2>{4, -5});

  advpt::testing::assert_false(Vec<double, 3>{0.1, 0.3, -1.2} !=
                               Vec<double, 3>{0.1, 0.3, -1.2});
  advpt::testing::assert_true(Vec<double, 3>{0.1, 0.3, -1.2} !=
                              Vec<double, 3>{0.11, 0.3, -1.2});

#else
  advpt::testing::dont_compile();
#endif
}

void testRange() {
#if TEST_RANGE

  static_assert(std::ranges::contiguous_range<Vec<double, 3>>);
  static_assert(std::ranges::sized_range<Vec<double, 3>>);

  {
    Vec<int32_t, 3> v;

    advpt::testing::assert_equal(v.size(), 3uz);

    for (const auto x : v) {
      advpt::testing::assert_equal(x, 0);
    }

    for (auto &x : v) {
      x = 3;
    }

    for (const auto x : v) {
      advpt::testing::assert_equal(x, 3);
    }
  }

  {
    const Vec<int32_t, 5> v(14);

    for (auto x : v) {
      advpt::testing::assert_equal(x, 14);
    }
  }

  {
    Vec<double, 4> v;

    double y{1.2};
    for (auto &x : v) {
      x = y;
      y += 1.;
    }

    double y2{1.2};
    for (const auto &x : v) {
      advpt::testing::assert_equal(x, y2);
      y2 += 1.;
    }
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testAdditiveOps() {
#if TEST_VECTOR_OPS

  {
    const Vec<int32_t, 3> v{0, 1, 2};
    const Vec<int32_t, 3> w{0, 1, 2};

    advpt::testing::assert_equal(-v, Vec<int32_t, 3>{0, -1, -2});
    advpt::testing::assert_equal(v + w, Vec<int32_t, 3>{0, 2, 4});
    advpt::testing::assert_equal(v - w, Vec<int32_t, 3>{0, 0, 0});
  }

  {
    const Vec<int64_t, 4> v{-3, 9, 2, 0};
    const Vec<int64_t, 4> w{11, -2, 3, 5};

    advpt::testing::assert_equal(-v, Vec<int64_t, 4>{3, -9, -2, 0});
    advpt::testing::assert_equal(v + w, Vec<int64_t, 4>{8, 7, 5, 5});
    advpt::testing::assert_equal(v - w, Vec<int64_t, 4>{-14, 11, -1, -5});
  }

  {
    const Vec<double, 3> v{0.25, -1.3, 15.2};
    const Vec<double, 3> w{-4.6, 7.31, -9.1};

    advpt::testing::assert_equal(-v, Vec<double, 3>{-0.25, 1.3, -15.2});
    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v + w, Vec<double, 3>{-4.35, 6.01, 6.1});
    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v - w, Vec<double, 3>{4.85, -8.61, 24.3});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testMultiplicativeOps() {
#if TEST_VECTOR_OPS

  {
    const Vec<int32_t, 3> v{0, 1, 2};

    advpt::testing::assert_equal(-1 * v, Vec<int32_t, 3>{0, -1, -2});
    advpt::testing::assert_equal(2 * v, Vec<int32_t, 3>{0, 2, 4});
    advpt::testing::assert_equal(6 * v, Vec<int32_t, 3>{0, 6, 12});
  }

  {
    const Vec<int64_t, 4> v{-3, 9, 2, 0};

    advpt::testing::assert_equal(3 * v, Vec<int64_t, 4>{-9, 27, 6, 0});
    advpt::testing::assert_equal(-2 * v, Vec<int64_t, 4>{6, -18, -4, 0});

    advpt::testing::assert_equal(v * 3, Vec<int64_t, 4>{-9, 27, 6, 0});
    advpt::testing::assert_equal(v * (-2), Vec<int64_t, 4>{6, -18, -4, 0});

    advpt::testing::assert_equal(v / 2, Vec<int64_t, 4>{-1, 4, 1, 0});
    advpt::testing::assert_equal(v / 3, Vec<int64_t, 4>{-1, 3, 0, 0});
  }

  {
    const Vec<float, 3> v{2.2f, -3.1f, 5.4f};

    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        1.5f * v, Vec<float, 3>{3.3f, -4.65f, 8.1f});
    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        -1.5f * v, Vec<float, 3>{-3.3f, 4.65f, -8.1f});

    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v * 1.5f, Vec<float, 3>{3.3f, -4.65f, 8.1f});
    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v * (-1.5f), Vec<float, 3>{-3.3f, 4.65f, -8.1f});

    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v / 2.f, Vec<float, 3>{1.1f, -1.55f, 2.7f});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testAugmentedAssignments() {
#if TEST_AUGMENTED_ASSIGNMENTS

  {
    Vec<int32_t, 3> v{0, 1, 2};
    const Vec<int32_t, 3> w{0, 1, 2};

    v += w;
    advpt::testing::assert_equal(v, Vec<int32_t, 3>{0, 2, 4});
  }

  {
    Vec<int32_t, 3> v{0, 1, 2};
    const Vec<int32_t, 3> w{0, 1, 2};

    v -= w;
    advpt::testing::assert_equal(v, Vec<int32_t, 3>{0, 0, 0});
  }

  {
    Vec<double, 3> v{0.25, -1.3, 15.2};
    const Vec<double, 3> w{-4.6, 7.31, -9.1};
    v += w;
    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v, Vec<double, 3>{-4.35, 6.01, 6.1});
  }

  {
    Vec<double, 3> v{0.25, -1.3, 15.2};
    const Vec<double, 3> w{-4.6, 7.31, -9.1};
    v -= w;
    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v, Vec<double, 3>{4.85, -8.61, 24.3});
  }

  {
    Vec<int32_t, 3> v{0, 1, 2};
    v *= 6;
    advpt::testing::assert_equal(v, Vec<int32_t, 3>{0, 6, 12});
  }

  {
    Vec<int64_t, 4> v{-3, 9, 2, 0};
    v /= 3;
    advpt::testing::assert_equal(v, Vec<int64_t, 4>{-1, 3, 0, 0});
  }

  {
    Vec<float, 3> v{2.2f, -3.1f, 5.4f};
    v *= 1.5f;
    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v, Vec<float, 3>{3.3f, -4.65f, 8.1f});
  }

  {
    Vec<float, 3> v{2.2f, -3.1f, 5.4f};
    v /= 2.f;
    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v, Vec<float, 3>{1.1f, -1.55f, 2.7f});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testAugmentedAssignments2() {
#if TEST_AUGMENTED_ASSIGNMENTS

  {
    Vec<int32_t, 3> v{0, 1, 2};
    const Vec<int32_t, 3> w{0, 1, 2};

    advpt::testing::assert_equal(v += w, Vec<int32_t, 3>{0, 2, 4});
  }

  {
    Vec<int32_t, 3> v{0, 1, 2};
    const Vec<int32_t, 3> w{0, 1, 2};

    advpt::testing::assert_equal(v -= w, Vec<int32_t, 3>{0, 0, 0});
  }

  {
    Vec<double, 3> v{0.25, -1.3, 15.2};
    const Vec<double, 3> w{-4.6, 7.31, -9.1};

    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v += w, Vec<double, 3>{-4.35, 6.01, 6.1});
  }

  {
    Vec<double, 3> v{0.25, -1.3, 15.2};
    const Vec<double, 3> w{-4.6, 7.31, -9.1};

    advpt::testing::with_tolerance{1e-14, 0.}.assert_allclose(
        v -= w, Vec<double, 3>{4.85, -8.61, 24.3});
  }

  {
    Vec<int32_t, 3> v{0, 1, 2};

    advpt::testing::assert_equal(v *= 6, Vec<int32_t, 3>{0, 6, 12});
  }

  {
    Vec<int64_t, 4> v{-3, 9, 2, 0};

    advpt::testing::assert_equal(v /= 3, Vec<int64_t, 4>{-1, 3, 0, 0});
  }

  {
    Vec<float, 3> v{2.2f, -3.1f, 5.4f};

    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v *= 1.5f, Vec<float, 3>{3.3f, -4.65f, 8.1f});
  }

  {
    Vec<float, 3> v{2.2f, -3.1f, 5.4f};

    advpt::testing::with_tolerance{1e-6, 0.}.assert_allclose(
        v /= 2.f, Vec<float, 3>{1.1f, -1.55f, 2.7f});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testMagnitude() {
#if TEST_MAGNITUDE

  {
    const Vec<double, 1> v{3.5};
    advpt::testing::assert_equal(v.sqrMagnitude(), 12.25);
    advpt::testing::with_tolerance{1e-14, 0.}.assert_close(v.magnitude(), 3.5);
  }

  {
    const Vec<double, 2> v{1., 2.};
    advpt::testing::assert_equal(v.sqrMagnitude(), 5.);
    advpt::testing::with_tolerance{1e-14, 0.}.assert_close(v.magnitude(),
                                                           2.236067977499790);
  }

  {
    const Vec<double, 2> v{3., 4.};
    advpt::testing::assert_equal(v.sqrMagnitude(), 25.);
    advpt::testing::with_tolerance{1e-14, 0.}.assert_close(v.magnitude(), 5.);
  }

  {
    const Vec<double, 3> v{1.3, 3.2, -4.1};
    advpt::testing::with_tolerance{1e-14, 0.}.assert_close(v.sqrMagnitude(),
                                                           28.74);
    advpt::testing::with_tolerance{1e-14, 0.}.assert_close(v.magnitude(),
                                                           5.360970061472084);
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testConvertingCtor() {
#if TEST_CONVERTING_CTOR

  {
    const Vec<uint32_t, 3> v32{12, 23, 21};
    const Vec<uint64_t, 3> v64(v32);
    advpt::testing::assert_equal(v64, Vec<uint64_t, 3>{12UL, 23UL, 21UL});
  }

  {
    const Vec<float, 3> v32{3.1f, 9.2f, -3.1f};
    const Vec<double, 3> v64(v32);
    advpt::testing::assert_equal(v64, Vec<double, 3>{3.1f, 9.2f, -3.1f});
  }

  {
    const Vec<uint32_t, 3> v32{12, 24, 21};
    auto v64 = static_cast<Vec<uint64_t, 3>>(v32);
    advpt::testing::assert_equal(v64, Vec<uint64_t, 3>{12UL, 24UL, 21UL});
  }

  {
    const Vec<float, 3> v32{3.1f, 9.2f, -3.1f};
    auto v64 = static_cast<Vec<double, 3>>(v32);
    advpt::testing::assert_equal(v64, Vec<double, 3>{3.1f, 9.2f, -3.1f});
  }

#else
  advpt::testing::dont_compile();
#endif
}

void testTypeAliases() {
#if TEST_TYPE_ALIASES

  static_assert(std::same_as<Vec3F, Vec<float, 3>>);
  static_assert(std::same_as<Vec3D, Vec<double, 3>>);

#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testStackAllocation", &testStackAllocation},
      {"testSizeAndData", &testSizeAndData},
      {"testDefaultCtor", &testDefaultCtor},
      {"testConstantCtor", &testConstantCtor},
      {"testInitListCtor", &testInitListCtor},
      {"testElementAccess", &testElementAccess},
      {"testEquality", &testEquality},
      {"testRange", &testRange},
      {"testAdditiveOps", &testAdditiveOps},
      {"testMultiplicativeOps", &testMultiplicativeOps},
      {"testAugmentedAssignments", &testAugmentedAssignments},
      {"testAugmentedAssignments2", &testAugmentedAssignments2},
      {"testMagnitude", &testMagnitude},
      {"testConvertingCtor", &testConvertingCtor},
      {"testTypeAliases", &testTypeAliases}}
      .run(argc, argv);
}
