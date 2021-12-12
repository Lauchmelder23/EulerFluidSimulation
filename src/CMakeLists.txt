set(BUILD_EULER_FLUID ON CACHE BOOL "Build Euler Fluid Project")

if(${BUILD_EULER_FLUID})
	add_subdirectory(EulerFluid)
endif()