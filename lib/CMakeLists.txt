# Set source files
file(GLOB VIDEOIO_ZED_SOURCE_FILES *.cpp )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONAN_INCLUDE_DIRS}
)


# build shared library.
add_library( videoio_zed SHARED ${VIDEOIO_ZED_SOURCE_FILES})

target_link_libraries( videoio_zed
  ${CONAN_LIBS}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${ZED_CUDA_LIBRARIES}
)
