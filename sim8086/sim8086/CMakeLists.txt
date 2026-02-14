# CMakeList.txt : CMake project for sim8086, include source and define

# project specific logic here.
set(SRC "src/main.cpp"
		"src/sim8086.h"
		"src/Decoder.h")

# Add source to this project's executable.
add_executable (sim8086 ${SRC} "src/InstructionTable.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sim8086 PROPERTY CXX_STANDARD 20)
endif()

# TESTS

# Define test target
add_custom_target(run_tests
		 COMMAND python3 ${CMAKE_SOURCE_DIR}/sim8086/tests/test_decoder.py $<TARGET_FILE:sim8086>
		 WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		 COMMENT "Running decoder tests..")

add_dependencies(run_tests sim8086)
