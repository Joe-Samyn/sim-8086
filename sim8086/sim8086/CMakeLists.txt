# CMakeList.txt : CMake project for sim8086, include source and define
# project specific logic here.
set(SRC "src/main.cpp"
		"src/sim8086.h"
		"src/decoder.h")

# Add source to this project's executable.
add_executable (sim8086 ${SRC})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sim8086 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
