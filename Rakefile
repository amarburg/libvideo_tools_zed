
$:.unshift File.dirname(__FILE__) + "/.rb"
require 'perf_testing'

task :default => "debug:test"

@conan_opts = {  build_parallel: 'False' }
@conan_settings = {  }
@conan_scopes = { build_tests: 'True' }
@conan_build = "outdated"
load 'config.rb' if FileTest.readable? 'config.rb'

build_root = ENV['BUILD_ROOT'] || "build"

['Debug','Release'].each { |build_type|
  namespace build_type.downcase.to_sym do
    build_dir = ENV['BUILD_DIR'] || "#{build_root}-#{build_type}"

    @conan_settings[:build_type] = build_type
    conan_opts = @conan_opts.each_pair.map { |key,val| "-o %s=%s" % [key,val] } +
                @conan_settings.each_pair.map { |key,val| "-s %s=%s" % [key,val] } +
                @conan_scopes.each_pair.map { |key,val| "--scope %s=%s" % [key,val] }

    task :build do
      FileUtils::mkdir build_dir unless FileTest::directory? build_dir
      chdir build_dir do
        sh "conan install %s .. --build=%s" % [conan_opts.join(' '), @conan_build]
        sh "conan build .."
      end
    end

    task :test => :build do
      #
    end
  end
}


task :foo do
  sh "rm -rf build-jetson-Release"
  sh "rake release:build"
  #sh "time build-jetson-Release/bin/ZedRecorder --resolution vga --svo-output /mnt/disk/ubuntu/foobar.svo --frames 1200"
  #sh "time build-jetson-Release/bin/Recorder --resolution 3 --filename /mnt/disk/ubuntu/recorder.svo"
end


namespace :conan  do
  desc "Export as Conan package"
  task :export => :distclean do
    sh "conan export amarburg/testing"
  end

  task :upload => :export do
    sh "conan upload libvideoio/master@amarburg/testing"
  end
end

task :distclean do
  sh "rm -rf build-*"
end

namespace :dependencies do

  task :trusty do
    sh "sudo apt-get install -y cmake libopencv-dev libtclap-dev libboost-all-dev"
    sh "pip install conan"
  end

  task :osx do
    sh "brew update"
    sh "brew tap homebrew/science"
    sh "brew install homebrew/science/opencv tclap conan"
  end

  namespace :travis do

    task :linux => "dependencies:trusty"

    task :osx => [:pip_uninstall_numpy, "dependencies:osx"]

    task :pip_uninstall_numpy do
      sh "pip uninstall -y numpy"
    end

  end
end
