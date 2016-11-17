from conans import ConanFile, CMake

class LibVideoIOZedConan(ConanFile):
  name = "libvideoio_zed"
  version = "master"
  settings = "os", "compiler", "build_type", "arch"
  generators = "cmake"
  options = {"opencv_dir": "ANY", "build_parallel": [True, False]}
  default_options = "opencv_dir=''", "build_parallel=True"
  exports = '*'
  #requires =  "libvideoio/master@amarburg/testing"
  requires = "g3log/master@amarburg/testing"

  def config(self):
    self.options["libvideoio"].opencv_dir = self.options.opencv_dir
    if self.scope.dev and self.scope.build_tests:
      self.requires( "gtest/1.8.0@lasote/stable" )
      self.options["gtest"].shared = False

  def imports(self):
    self.copy("*.dll", dst="bin", src="bin") # From bin to bin
    self.copy("*.dylib*", dst="bin", src="lib") # From lib to bin

  def build(self):
    cmake = CMake(self.settings)
    cmake_opts = ""

    cmake_opts += "-DOpenCV_DIR:PATH=%s " % (self.options.opencv_dir) if self.options.opencv_dir else ""
    cmake_opts += "-DBUILD_UNIT_TESTS=1 " if self.scope.dev and self.scope.build_tests else ""

    build_opts = "-j" if self.options.build_parallel else ""

    self.run('cmake "%s" %s %s ' % (self.conanfile_directory, cmake.command_line, cmake_opts))
    self.run('cmake --build . %s -- %s' % (cmake.build_config, build_opts))
    # if self.scope.dev and self.scope.build_tests:
    #   self.run('cp lib/libvideoio.* bin/')
      # self.run('make unit_test')

  def package(self):
    self.copy("*.h", dst="")
    #if self.options.shared:
    if self.settings.os == "Macos":
        self.copy(pattern="*.dylib", dst="lib", keep_path=False)
    else:
        self.copy(pattern="*.so*", dst="lib", src="lib", keep_path=False)
    #else:
    #    self.copy(pattern="*.a", dst="lib", src="lib", keep_path=False)

  def package_info(self):
      self.cpp_info.libs = ["videoio"]
