set( _Milestone 1 )
set( _Task 2 )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for Vec

set( TestApp TestVec )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testStackAllocation
  testSizeAndData
  testDefaultCtor
  testConstantCtor
  testInitListCtor
  testElementAccess
  testEquality
  testRange
  testAdditiveOps
  testMultiplicativeOps
  testAugmentedAssignments
  testAugmentedAssignments2
  testMagnitude
  testConvertingCtor
  testTypeAliases
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()

############## Tests for MortonIndex

set( TestApp TestMortonIndex )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testBasicInterface
  testFromPath
  testGetPath
  testPositionQueries
  testTraversal
  testSafeTraversal
  testEquality
  testInequalities
  testGridCoordinates
  testFromGridCoordinates
)
foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()

############## Tests for Box

set( TestApp TestBox )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testConstructorsAndGetters
  testSetters
  testObservers
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()


############## Tests for OctreeGeometry

set( TestApp TestOctreeGeometry )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testBasicInterface
  testCellExtents
  testCellGeometry
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()
