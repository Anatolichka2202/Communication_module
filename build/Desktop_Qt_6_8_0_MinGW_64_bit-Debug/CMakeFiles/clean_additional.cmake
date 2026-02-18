# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CommunicationLib_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CommunicationLib_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\test_runner_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\test_runner_autogen.dir\\ParseCache.txt"
  "CommunicationLib_autogen"
  "test_runner_autogen"
  )
endif()
