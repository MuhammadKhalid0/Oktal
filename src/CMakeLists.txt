add_subdirectory( geometry )

add_subdirectory( octree )

target_sources(
    oktal
    PRIVATE
        geometry/PeriodicBox.cpp
        octree/CellOctree.cpp
)