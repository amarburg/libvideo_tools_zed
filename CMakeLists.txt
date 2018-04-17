cmake_minimum_required(VERSION 3.5)
if( NOT FIPS_IMPORT )
        get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
        include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

        fips_setup(PROJECT libvideoio_zed)
endif()

find_package(Eigen3 REQUIRED )
find_package(Boost COMPONENTS filesystem system program_options REQUIRED )  #n.b. program_options is only required for my version of Stereolabs "Recorder"

include( ${PROJECT_SOURCE_DIR}/cmake/Zed.cmake )
#set( CUDA_VERSION 8.0 )

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
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pg")

## Configure OpenCV ##

find_package(OpenCV ${OPENCV_VERSION} COMPONENTS ${OPENCV_COMPONENTS} REQUIRED)

## Set global include paths
set( LIBVIDEO_TOOLS_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
	${CUDA_INCLUDE_DIRS}
  	${OpenCV_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	${ZED_INCLUDE_DIRS} )

include_directories( ${LIBVIDEO_TOOLS_INCLUDE_DIRS} )

#fips_add_subdirectory("lib/")

if( NOT FIPS_IMPORT )
#        set( TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test/data" )
#        add_definitions( -DTEST_DATA_DIR="${TEST_DATA_DIR}" )

        fips_add_subdirectory("tools/")

        if( FIPS_UNITTESTS )
          fips_add_subdirectory( test/unit/ )
        endif()

endif()


if( NOT FIPS_IMPORT)
        fips_finish()
endif()

