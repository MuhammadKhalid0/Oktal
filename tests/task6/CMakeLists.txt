set( _Milestone 2 )
set( _Task 6 )

add_custom_target( OktalTests-Milestone${_Milestone} )
add_dependencies( OktalTests-All OktalTests-Milestone${_Milestone} )

add_custom_target( OktalTests-Task${_Task} )
add_dependencies( OktalTests-Milestone${_Milestone} OktalTests-Task${_Task} )

############## Tests for GridVector

set( TestApp TestGridVector )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testTemplateInterface
  testContainerSemantics
  testGridVectorViewType
  testGridVectorViews
  testScalarFields
  testViewConversion
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()


############## Tests for VTK Exporters

set( TestApp TestVtkExportExtensions )

add_executable( ${TestApp} ${TestApp}.cpp )
target_link_libraries( ${TestApp} PRIVATE oktal advpt::testing )
add_dependencies( OktalTests-Task${_Task} ${TestApp} )

set(
  TestIDs
  testWriteGridVector
)

foreach( TestID ${TestIDs} )
  set( TestName "${TestApp} - ${TestID}" )
  add_test( NAME ${TestName} COMMAND $<TARGET_FILE:${TestApp}> ${TestID} )
  set_tests_properties( ${TestName} PROPERTIES LABELS "Task${_Task};Milestone${_Milestone}" )
endforeach()
