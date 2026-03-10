if( AppExecutable STREQUAL "NOTFOUND" )
  message(FATAL_ERROR "No executable for the TGV simulation found.")
endif()

if(NOT EXISTS ${AppExecutable})
  message(FATAL_ERROR "No executable for the TGV simulation found.")
endif()


message(STATUS "Executable for the TGV simulation found at ${AppExecutable}")

set(outputDirectory tgv-out)
set(errorsFile ${outputDirectory}/errors.txt)

# Invalid command line
execute_process(
  COMMAND ${AppExecutable} 5
  RESULT_VARIABLE execResult
)

if(${execResult} EQUAL 0)
  message(FATAL_ERROR "Expected application to return nonzero exit code when called with invalid arguments.")
endif()

# Invalid refinement level
execute_process(
  COMMAND ${AppExecutable} 4 ${outputDirectory}
  RESULT_VARIABLE execResult
)

if(${execResult} EQUAL 0)
  message(FATAL_ERROR "Expected application to return nonzero exit code when called with invalid arguments.")
endif()

execute_process(
  COMMAND ${AppExecutable} 5 ${outputDirectory}
  COMMAND_ERROR_IS_FATAL LAST
)

if(EXISTS ${errorsFile})
  message(STATUS "Found expected output file ${errorsFile}")
else()
  message(FATAL_ERROR "TGV application did not create expected output file ${errorsFile}")
endif()
