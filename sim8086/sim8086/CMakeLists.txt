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

