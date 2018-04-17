
include_directories(
  ${LIBVIDEO_TOOLS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

fips_begin_app( ZedConverter cmdline )
	fips_files( ZedConverter.cpp )
  fips_libs( ${ZED_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT} )
  fips_deps( logger videoio )
fips_end_app()

fips_begin_app( ZedRecorder cmdline )
	fips_files( ZedRecorder.cpp )
  fips_libs(${ZED_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CUDA_LIBRARIES} )
  fips_deps( logger videoio )
fips_end_app()
