

task :perf do

  binary = "build-jetson-Release/bin/ZedRecorder"

  stats_file = 'perf_statistics.csv'

  git_hash = `git log -n 1 --pretty=format:"%h"`

  %w( vga ).each { |resolution|

    5.times.each { |i|

      svo ="/mnt/disk/ubuntu/perf_testing/perf_#{resolution}_#{Time::now.strftime("%H%M%S")}.svo"
      sh "#{binary} --resolution #{resolution} --frames 1200 --statistics-output #{stats_file} --statistics-id #{git_hash} --svo-output #{svo}"

    }


  }

end
