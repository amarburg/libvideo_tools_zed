
add_definitions( -DUSE_ZED )

find_package(ZED REQUIRED)

# if( ZED_VERSION VERSION_EQUAL "1.0.0")
  add_definitions( -DZED_1_0 )
# endif()

set( ZED_INCLUDE_DIRS /usr/local/zed/include )

set( ZED_CUDA_LIBRARIES /usr/local/zed/lib/libsl_core.so
                        ${ZED_LIBRARIES}
                        ${CUDA_LIBRARIES}
                        ${CUDA_npps_LIBRARY}
                        ${CUDA_nppi_LIBRARY} )
