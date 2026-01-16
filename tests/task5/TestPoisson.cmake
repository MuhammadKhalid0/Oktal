if( PoissonExecutable STREQUAL "NOTFOUND" )
  message(FATAL_ERROR "No executable for the Poisson solver found.")
endif()

if(NOT EXISTS ${PoissonExecutable})
  message(FATAL_ERROR "No executable for the Poisson solver found.")
endif()


message(STATUS "Executable for the Poisson solver found at ${PoissonExecutable}")

set(VtkOutputFile poisson-output.vtkhdf)

# Invalid command line
execute_process(
  COMMAND ${PoissonExecutable} 3 100 oops
  RESULT_VARIABLE execResult
)

if(${execResult} EQUAL 0)
  message(FATAL_ERROR "Expected application to return nonzero exit code when called with invalid arguments.")
endif()

execute_process(
  COMMAND ${PoissonExecutable} 3 100 1e-3 ${VtkOutputFile}
  COMMAND_ERROR_IS_FATAL LAST
)

if(EXISTS ${VtkOutputFile})
  message(STATUS "Found expected output file ${VtkOutputFile}")
else()
  message(FATAL_ERROR "Poisson application did not create expected output file ${VtkOutputFile}")
endif()
