
add_library(nm_utils STATIC
	"Window.cpp"
 "RetentiveArray.hpp")

target_include_directories(nm_utils PUBLIC ${SDL2_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nm_utils PUBLIC ${SDL2_LIBRARIES})

if(MSVC)
	target_compile_definitions(nm_utils PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()