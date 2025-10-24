#include "advpt/testing/Testutils.hpp"

#include "oktal/geometry/PeriodicBox.hpp"

namespace {

void testConstructor() {
  using namespace oktal;

  {
    const PeriodicBox pbox{{0., 0., 0.}, {1., 1., 1.}, {false, false, false}};

    advpt::testing::assert_equal(pbox.minCorner(), {0., 0., 0.});
    advpt::testing::assert_equal(pbox.maxCorner(), {1., 1., 1.});
    advpt::testing::assert_equal(pbox.periodicity(), {false, false, false});
  }

  {
    const PeriodicBox pbox{{3.1, 2.2, 1.2}, {4., 3.7, 2.}, {true, false, true}};

    advpt::testing::assert_equal(pbox.minCorner(), {3.1, 2.2, 1.2});
    advpt::testing::assert_equal(pbox.maxCorner(), {4., 3.7, 2.});
    advpt::testing::assert_equal(pbox.periodicity(), {true, false, true});
  }
}

void testMapFullyPeriodic() {
  using namespace oktal;

  {
    PeriodicBox pbox{{0., 0., 0.}, {1., 1., 1.}, {true, true, true}};

    advpt::testing::assert_equal(pbox.mapIntoBox({0.2, 0.35, 0.7}),
                                 {0.2, 0.35, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({1.2, 0.35, 0.7}), std::array{0.2, 0.35, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({1.2, 0.35, -0.3}), std::array{0.2, 0.35, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({-0.54, 2.35, 0.3}), std::array{0.46, 0.35, 0.3});
  }

  {
    PeriodicBox pbox{{1., 1.5, 2.1}, {1.5, 2.7, 3.1}, {true, true, true}};

    advpt::testing::assert_equal(pbox.mapIntoBox({1.3, 1.75, 2.25}),
                                 {1.3, 1.75, 2.25});
    advpt::testing::assert_equal(pbox.mapIntoBox({1., 1.5, 2.1}),
                                 {1., 1.5, 2.1});
    advpt::testing::assert_equal(pbox.mapIntoBox({1.5, 2.7, 3.1}),
                                 {1., 1.5, 2.1});
  }
}

void testMapPartlyPeriodic() {
  using namespace oktal;

  {
    PeriodicBox pbox{{0., 0., 0.}, {1., 1., 1.}, {true, false, true}};

    advpt::testing::assert_equal(pbox.mapIntoBox({0.2, 0.35, 0.7}),
                                 {0.2, 0.35, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({1.2, 0.35, 0.7}), std::array{0.2, 0.35, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({1.2, 2.7, -0.3}), std::array{0.2, 2.7, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({-0.54, -5.35, 0.3}), std::array{0.46, -5.35, 0.3});
  }

  {
    PeriodicBox pbox{{-1., 0.5, -0.25}, {2., 2., 0.}, {true, true, false}};

    advpt::testing::assert_equal(pbox.mapIntoBox({-1., 0.7, -0.2}),
                                 {-1., 0.7, -0.2});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({-11.2, 1.2, 0.7}), std::array{0.8, 1.2, 0.7});
    advpt::testing::with_tolerance{1e-15, 0.}.assert_allclose(
        pbox.mapIntoBox({-1., 3.1, -0.25}), std::array{-1., 1.6, -0.25});
  }
}

void testPeriodicDistance() {
  using namespace oktal;

  {
    PeriodicBox pbox{{0., 0., 0.}, {1., 1., 1.}, {true, true, true}};

    advpt::testing::with_tolerance{1e-15, 0.}.assert_close(
        pbox.sqrDistance({0., 0., 0.}, {0.2, 0.3, 0.4}),
        0.2 * 0.2 + 0.3 * 0.3 + 0.4 * 0.4);

    advpt::testing::with_tolerance{1e-15, 0.}.assert_close(
        pbox.sqrDistance({0., 0., 0.}, {0.2, 0.3, 0.9}),
        0.2 * 0.2 + 0.3 * 0.3 + 0.1 * 0.1);

    advpt::testing::with_tolerance{1e-15, 0.}.assert_close(
        pbox.sqrDistance({0., 0., 0.}, {0.8, 0.6, 0.5}),
        0.2 * 0.2 + 0.4 * 0.4 + 0.5 * 0.5);

    advpt::testing::with_tolerance{1e-15, 0.}.assert_close(
        pbox.sqrDistance({0.1, 0.8, 0.25}, {0.9, 0.1, 0.75}),
        0.2 * 0.2 + 0.3 * 0.3 + 0.5 * 0.5);
  }

  {
    PeriodicBox pbox{{1., -1.5, -0.5}, {2., -0.5, 1.5}, {true, true, true}};

    advpt::testing::with_tolerance{1e-15, 0.}.assert_close(
        pbox.sqrDistance({1., -1.5, -0.5}, {1.2, -1.2, -0.1}),
        0.2 * 0.2 + 0.3 * 0.3 + 0.4 * 0.4);
  }
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testConstructor", &testConstructor},
      {"testMapFullyPeriodic", &testMapFullyPeriodic},
      {"testMapPartlyPeriodic", &testMapPartlyPeriodic},
      {"testPeriodicDistance", &testPeriodicDistance}}
      .run(argc, argv);
}
