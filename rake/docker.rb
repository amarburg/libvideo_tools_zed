

namespace :docker do
  task :image do
    sh "nvidia-docker build -t amarburg/libvideoio_zed:latest docker/"
  end

  task :build => "image" do
    sh "nvidia-docker run --env VERBOSE=1 --env BUILD_ROOT=build-docker \
            --env CONAN_USER_HOME=/home/lsdslam/libvideoio_zed/.conan-docker \
            -v `pwd`:/home/lsdslam/libvideoio_zed -w /home/lsdslam/libvideoio_zed \        
            amarburg/libvideoio_zed:latest rake debug:test"
  end

  task :console => "image" do
    sh "nvidia-docker run --tty --interactive --env VERBOSE=1 --env BUILD_ROOT=build-docker \
            --env CONAN_USER_HOME=/home/lsdslam/libvideoio_zed/.conan-docker \
            -v `pwd`:/home/lsdslam/libvideoio_zed -w /home/lsdslam/libvideoio_zed \
            -v /home/aaron/workspace/mascot/mascot_data:/home/lsdslam/data \
            amarburg/libvideoio_zed:latest"
  end
end
