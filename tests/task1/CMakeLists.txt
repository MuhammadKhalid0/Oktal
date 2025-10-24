
set( _Milestone 1 )
set( _Task 1 )

add_custom_target( OktalTests-Milestone${_Milestone} )
add_dependencies( OktalTests-All OktalTests-Milestone${_Milestone} )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for PeriodicBox

set( TestApp TestPeriodicBox )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set( TestIDs testConstructor testMapFullyPeriodic testMapPartlyPeriodic testPeriodicDistance )
foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()
