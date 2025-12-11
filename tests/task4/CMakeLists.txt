set( _Milestone 1 )
set( _Task 4 )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for OctreeCursor

set( TestApp TestCursor )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testBasicInterface
  testObservers
  testEqualityOperators
  testAscendDescend
  testMoveToSiblings
  testToEnd
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()

############## Tests for octree iterators

set( TestApp TestOctreeIterators )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testPolicyConcept
  testIteratorTemplate
  testRangeTemplate
  testPreOrderDepthFirst
  testPreOrderDepthFirstWithPhantoms
  testHorizontal
  testHorizontalWithPhantoms
)
foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()
