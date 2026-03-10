set( _Milestone 2 )
set( _Task 7 )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for Basic LBM Kernels

set( TestApp TestBasicLbmKernels )
set( TestDataFile ${TestApp}Data.h5 )
set( TestDataFileSource ${CMAKE_CURRENT_SOURCE_DIR}/${TestDataFile} )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )
target_compile_definitions( ${TestApp} PRIVATE DATAFILE_LOCATION=\"${TestDataFileSource}\" )

set(
  TestIDs
  testInitialization1
  testInitialization2
  testMacroscopicQuantities1
  testMacroscopicQuantities2
  testCollision
  testStreaming
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()


############## Tests for TGV Simulation

set( TestName TestTgv )
set( AppTarget tgv )

if(TARGET ${AppTarget})
  add_dependencies( OktalTests-Task${_Task} ${AppTarget} )
  set( _TgvExecutable $<TARGET_FILE:${AppTarget}> )
else()
  set( _TgvExecutable NOTFOUND )
endif()

add_test(
    NAME ${TestName}
    COMMAND 
      ${CMAKE_COMMAND}
      -D AppExecutable=${_TgvExecutable}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/${TestName}.cmake 
  )
set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
