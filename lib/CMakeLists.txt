# Set source files
file(GLOB VIDEOIO_ZED_SOURCE_FILES *.cpp )


# build shared library.
fips_begin_module( videoio_zed )
fips_files( ${VIDEOIO_ZED_SOURCE_FILES})

fips_libs( ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${ZED_CUDA_LIBRARIES}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)

fips_end_module()
