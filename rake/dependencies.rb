

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
