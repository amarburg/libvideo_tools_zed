include_directories( ${GTEST_INCLUDE_DIRS}
                     ${CMAKE_SOURCE_DIR}/lib )

file( GLOB UNIT_TEST_SRCS *_test.cpp )

add_executable( videoio_zed_test
              ${UNIT_TEST_SRCS} )

link_directories( ${CONAN_LIBRARY_DIR} )

target_link_libraries( videoio_zed_test
          ${CONAN_LIBS}
        videoio_zed )

# Don't use ctest, instead just execture the Google test binaries
add_custom_target( unit_test
  COMMAND ./videoio_zed_test
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  DEPENDS videoio_zed_test )
