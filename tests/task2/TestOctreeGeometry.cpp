#include "advpt/testing/Testutils.hpp"
#include "oktal/octree/OctreeGeometry.hpp"

#define TEST_BASIC_INTERFACE false
#define TEST_CELL_EXTENTS false
#define TEST_CELL_GEOMETRY false


namespace {

using namespace oktal;

void testBasicInterface() {
#if TEST_BASIC_INTERFACE
  {
    //  Default ctor should create unit cube
    const OctreeGeometry geom;

    advpt::testing::assert_equal(geom.sidelength(), 1.);
    advpt::testing::assert_equal(geom.origin(), Vec3D(0.));
  }

  {
    const OctreeGeometry geom({-1., 0.5, -0.25}, 1.5);

    advpt::testing::assert_equal(geom.sidelength(), 1.5);
    advpt::testing::assert_equal(geom.origin(), Vec3D{-1, 0.5, -0.25});
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCellExtents() {
#if TEST_CELL_EXTENTS
  {
    const OctreeGeometry geom;

    for (auto [level, spacing] :
         std::views::zip(std::array{0uz, 1uz, 2uz, 3uz, 4uz, 5uz},
                         std::array{1., 0.5, 0.25, 0.125, 0.0625, 0.03125})) {
      advpt::testing::assert_equal(geom.dx(level), spacing);
      advpt::testing::assert_equal(geom.cellExtents(level), Vec3D(spacing));
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}

void testCellGeometry() {
#if TEST_CELL_GEOMETRY
  {
    const OctreeGeometry geom;

    {
      const MortonIndex m;

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D(0.));
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D(1.));
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D(0.));
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D(1.));
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D(0.5));
    }

    {
      const MortonIndex m{010};

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D(0.));
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D(0.5));
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D(0.));
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D(0.5));
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D(0.25));
    }

    {
      const MortonIndex m{0b1101};

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D{0.5, 0., 0.5});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{1., 0.5, 1.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{0.5, 0., 0.5});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{1., 0.5, 1.});
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D{0.75, 0.25, 0.75});
    }

    {
      const MortonIndex m{0b1101011};

      advpt::testing::assert_equal(geom.cellMinCorner(m),
                                   Vec3D{0.75, 0.25, 0.5});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{1., 0.5, 0.75});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{0.75, 0.25, 0.5});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{1., 0.5, 0.75});
      advpt::testing::assert_equal(geom.cellCenter(m),
                                   Vec3D{0.875, 0.375, 0.625});
    }
  }

  {
    const OctreeGeometry geom{{2., -1., 1.}, 2.};

    {
      const MortonIndex m;

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D{2., -1., 1.});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{4., 1., 3.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{2., -1., 1.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{4., 1., 3.});
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D{3., 0., 2.});
    }

    {
      const MortonIndex m{010};

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D{2., -1., 1.});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{3., 0., 2.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{2., -1., 1.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{3., 0., 2.});
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D{2.5, -0.5, 1.5});
    }

    {
      const MortonIndex m{0b1101};

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D{3., -1., 2.});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{4., 0., 3.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{3., -1., 2.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{4., 0., 3.});
      advpt::testing::assert_equal(geom.cellCenter(m), Vec3D{3.5, -0.5, 2.5});
    }

    {
      const MortonIndex m{0b1101011};

      advpt::testing::assert_equal(geom.cellMinCorner(m), Vec3D{3.5, -0.5, 2.});
      advpt::testing::assert_equal(geom.cellMaxCorner(m), Vec3D{4., 0., 2.5});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).minCorner(),
                                   Vec3D{3.5, -0.5, 2.});
      advpt::testing::assert_equal(geom.cellBoundingBox(m).maxCorner(),
                                   Vec3D{4., 0., 2.5});
      advpt::testing::assert_equal(geom.cellCenter(m),
                                   Vec3D{3.75, -0.25, 2.25});
    }
  }
#else
  advpt::testing::dont_compile();
#endif
}

} // namespace

int main(int argc, char **argv) {
  return advpt::testing::TestsRunner{
      {"testBasicInterface", &testBasicInterface},
      {"testCellExtents", &testCellExtents},
      {"testCellGeometry", &testCellGeometry}}
      .run(argc, argv);
}
