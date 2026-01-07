target_sources(
    oktal
    PRIVATE
        CellOctree.cpp
        CellGrid.cpp
)
target_sources( oktal PRIVATE MortonIndex.cpp OctreeGeometry.cpp)
