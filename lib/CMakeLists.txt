# Set source files
file(GLOB VIDEOIO_ZED_SOURCE_FILES *.cpp )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LIBVIDEO_TOOLS_INCLUDE_DIRS}
)

# build shared library.
add_library(libvideo_tools_zed SHARED ${VIDEOIO_ZED_SOURCE_FILES})

target_link_libraries(libvideo_tools_zed
  ${LIBLOGGER_LIBS}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${G3LOG_LIB}
  ${ZED_CUDA_LIBRARIES}
)
