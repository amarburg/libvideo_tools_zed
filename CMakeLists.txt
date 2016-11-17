cmake_minimum_required(VERSION 2.8.7)
project(libvideoio_zed)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Eigen3 REQUIRED )
find_package(Boost COMPONENTS filesystem system REQUIRED )

include( ${PROJECT_SOURCE_DIR}/cmake/Zed.cmake )
set( CUDA_VERSION 8.0 )

## Need this workaround for CUDA 8.0
find_package( CUDA )
if( CUDA_VERSION )
	message("Found CUDA version ${CUDA_VERSION}")
	if(${CUDA_VERSION} VERSION_EQUAL "8.0")
		message("Disabling CUDA_USE_STATIC_CUDA_RUNTIME")
		set( CUDA_USE_STATIC_CUDA_RUNTIME OFF CACHE BOOL "" FORCE  )
	endif()
	find_package( CUDA )
endif()

## Default OpenCV version
set( OPENCV_VERSION 3.1 )
list(APPEND OPENCV_COMPONENTS core highgui imgproc video calib3d)

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )
if( ${ARCHITECTURE} STREQUAL "aarch64" )
	# Jetson version uses OpenCV4Tegra from NVidia
	set( OPENCV_VERSION 2.4 )

	## For compilation on Jetson TX1
#	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a") # -DENABLE_NEON" )

elseif( ${ARCHITECTURE} STREQUAL "x86_64" )
	  # -DENABLE_SSE enables Jakob's original SSE optimizations in his code,
	# it has nothing to do with compiler behavior
  set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4.1 -msse3 -msse2 -msse -march=native -DENABLE_SSE" )
endif()

find_package( CUDA REQUIRED ${CUDA_VERSION} )

## C++ flags applied to all builds
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" ) #-std=c++11")

## Configure OpenCV ##

find_package(OpenCV ${OPENCV_VERSION} COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)

## Set global include paths
set( LIBVIDEO_TOOLS_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
	${CUDA_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	${ZED_INCLUDE_DIRS}
	${CONAN_INCLUDE_DIRS} )

include_directories( ${LIBVIDEO_TOOLS_INCLUDE_DIRS} )

#add_subdirectory("lib/")
add_subdirectory("tools/")

option( BUILD_UNIT_TESTS "Build unit tests" ON )
if( BUILD_UNIT_TESTS )
  add_subdirectory( test/unit/ )
endif()
