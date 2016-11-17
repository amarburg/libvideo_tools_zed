
include_directories(
  ${LIBVIDEO_TOOLS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# set(CMAKE_CXX_FLAGS
#    "${CMAKE_CXX_FLAGS} -ggdb"
# )

add_executable( ZedRecorder ZedRecorder.cpp)
target_link_libraries( ZedRecorder
  ${ZED_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}   )

add_executable( Recorder main.cpp )
target_link_libraries( Recorder
  ${ZED_CUDA_LIBRARIES} 
  ${Boost_LIBRARIES}   )

#set_target_properties( ZedRecorder PROPERTIES COMPILE_FLAGS "-ggdb")

# add_executable( ZedConverter ZedConverter.cpp)
# target_link_libraries( ZedConverter videoio_zed )
#set_target_properties( ZedConverter PROPERTIES COMPILE_FLAGS "-ggdb")
