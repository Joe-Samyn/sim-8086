# CMakeList.txt : CMake project for sim8086, include source and define

# project specific logic here.
set(SRC "src/Main.cpp")

# Add source to this project's executable.
add_executable (sim8086 ${SRC})

# Add compile definitions 
add_compile_definitions(DEBUG)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sim8086 PROPERTY CXX_STANDARD 20)
endif()

enable_testing()

set(TST_SRC "tests/test_main.cpp")
add_executable(sim_tests ${TST_SRC})

target_include_directories(sim_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_test(NAME SimulatorTests COMMAND sim_tests)
