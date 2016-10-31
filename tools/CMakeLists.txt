
include_directories(
  ${LIBVIDEO_TOOLS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# set(CMAKE_CXX_FLAGS
#    "${CMAKE_CXX_FLAGS} -ggdb"
# )

add_executable( ZedRecorder ZedRecorder.cpp)
target_link_libraries( ZedRecorder libvideo_tools_zed )
set_target_properties( ZedRecorder PROPERTIES COMPILE_FLAGS "-ggdb")


# # build image node
# add_executable(LSD LSD.cpp
#   LSD/GuiThread.cpp
#   LSD/InputThread.cpp)
# target_link_libraries( LSD lsdslam )
