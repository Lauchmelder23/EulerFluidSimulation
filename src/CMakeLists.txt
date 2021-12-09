# CMakeList.txt : CMake project for EulerFluid, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (EulerFluid "main.cpp" "Application.hpp" "Application.cpp" "VectorField.hpp" "VectorField.cpp")

target_include_directories(EulerFluid PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(EulerFluid PUBLIC ${SDL2_LIBRARIES})

if(MSVC)
target_compile_definitions(EulerFluid PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

# TODO: Add tests and install targets if needed.
