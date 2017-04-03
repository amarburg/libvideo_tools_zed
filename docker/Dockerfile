# This is a comment
FROM amarburg/lsdslam-dev-host:nvidia_cuda_opencv3.1_zed1.2
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

VOLUME /home/lsdslam/libvideoio_zed
VOLUME /home/lsdslam/data

COPY settings/* /usr/local/zed/settings

RUN conan config set settings_defaults.compiler.libcxx=libstdc++11
