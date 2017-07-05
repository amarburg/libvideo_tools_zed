
include_directories(
  ${LIBVIDEO_TOOLS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable( ZedConverter ZedConverter.cpp)
target_link_libraries( ZedConverter
  ${ZED_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CONAN_LIBS} )

add_executable( ZedRecorder ZedRecorder.cpp)
target_link_libraries( ZedRecorder
  ${ZED_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CUDA_LIBRARIES}
  ${CONAN_LIBS} )
