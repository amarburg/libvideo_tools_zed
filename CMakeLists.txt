cmake_minimum_required(VERSION 2.8.7)
project(videio)
include(ExternalProject)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

## Configure other external packages
find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(TCLAP REQUIRED )
find_package(ZLIB REQUIRED )
find_package(Eigen3 REQUIRED )

set( EXTERNAL_PROJECTS  )

include( ${PROJECT_SOURCE_DIR}/cmake/Zed.cmake )
set( CUDA_VERSION 8.0 )

## Default OpenCV version
set( OPENCV_VERSION 3.1 )
list(APPEND OPENCV_COMPONENTS core highgui imgproc video calib3d)

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )
if( ${ARCHITECTURE} STREQUAL "aarch64" )
	# Jetson version uses OpenCV4Tegra from NVidia
	#set( OPENCV_VERSION 2.4 )

	## For compilation on Jetson TX1
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a") # -DENABLE_NEON" )

elseif( ${ARCHITECTURE} STREQUAL "x86_64" )
	# x86 version uses Zed 1.0 SDK which is linked against Opencv 3.1
	# if( ZED_VERSION VERSION_EQUAL "1.0.0")
	# 	set( OPENCV_VERSION 3.1 )
	# 	set( CUDA_VERSION 7.5 )
	# 	list( APPEND OPENCV_COMPONENTS videoio imgcodecs )
	# endif()

  # -DENABLE_SSE enables Jakob's original SSE optimizations in his code,
	# it has nothing to do with compiler behavior
  set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4.1 -msse3 -msse2 -msse -march=native -DENABLE_SSE" )
endif()

# option(USE_GPU "Build GPU-accelerated tools" OFF)
# if( USE_GPU )
# 	add_definitions( -DUSE_GPU )
# 	list(APPEND OPENCV_COMPONENTS gpu )
# endif()

find_package( CUDA REQUIRED ${CUDA_VERSION} )

## C++ flags applied to all builds
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

## Configure OpenCV ##

find_package(OpenCV ${OPENCV_VERSION} COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)


if(APPLE)
	set( CMAKE_MACOSX_RPATH OFF )
endif()


include( ${PROJECT_SOURCE_DIR}/cmake/BuildLibVideoTools.cmake )

## Set global include paths
set( LIBVIDEO_TOOLS_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
	${CUDA_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	${ZED_INCLUDE_DIRS}
 	${LIBVIDEO_TOOLS_INCLUDE_DIRS} )

include_directories( ${LIBVIDEO_TOOLS_INCLUDE_DIRS} )

add_subdirectory("lib/")
add_subdirectory("tools/")

option( BUILD_UNIT_TESTS "Build unit tests" ON )
if( BUILD_UNIT_TESTS )
  ## Build a local copy of gtest (as per the GTest FAQ)
  include( ${CMAKE_SOURCE_DIR}/cmake/BuildGTestLibraries.cmake )

  add_subdirectory( test/unit/ )

endif()


add_custom_target( deps DEPENDS ${EXTERNAL_PROJECTS} )
