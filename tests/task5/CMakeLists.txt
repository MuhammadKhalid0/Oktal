set( _Milestone 1 )
set( _Task 5 )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for CellGrid

set( TestApp TestCellGrid )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testEnumerationInterface
  testBuilderInterface
  testEnumerateNoPhantoms
  testEnumerateWithPhantoms
  testAdjacencyInterface
  testNeighborIndices
  testNoPeriodicity
  testTorus
  testCellsRange
  testCentralDifference
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()


############## Tests for b) Utilities

set( TestApp TestUtilities )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testUniformGrid
  testVtkExport
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()

############## Tests for Poisson Solver

set( TestName TestPoisson )

if(TARGET poisson)
  add_dependencies( OktalTests-Task${_Task} poisson )
  set( _PoissonExecutable $<TARGET_FILE:poisson> )
else()
  set( _PoissonExecutable NOTFOUND )
endif()

add_test(
    NAME ${TestName}
    COMMAND 
      ${CMAKE_COMMAND}
      -D PoissonExecutable=${_PoissonExecutable}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/TestPoisson.cmake 
  )
set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
