
// TODO:   Reduce the DRY

#include <string>
using namespace std;

#include <opencv2/opencv.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <sl/Camera.hpp>
#include "libvideoio_zed/ZedUtils.h"
#include "libvideoio_zed/ZedSource.h"

#include <tclap/CmdLine.h>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include "libvideoio/G3LogSinks.h"

#include "libvideoio/DataSource.h"
#include "libvideoio/Undistorter.h"

#include "logger/LogWriter.h"

#include "libvideoio/Display.h"
#include "libvideoio/ImageOutput.h"
#include "libvideoio/VideoOutput.h"
#include "libvideoio/CompositeCanvas.h"
using namespace libvideoio;


using namespace lsd_slam;


bool keepGoing = true;

void signal_handler( int sig )
{
	switch( sig ) {
		case SIGINT:
		keepGoing = false;
		break;
	}
}

using cv::Mat;

int main( int argc, char** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
	auto stderrHandle = worker->addSink(std::unique_ptr<ColorStderrSink>( new ColorStderrSink ),
	&ColorStderrSink::ReceiveLogMessage);
	g3::initializeLogging(worker.get());

	signal( SIGINT, signal_handler );

	try {
		TCLAP::CmdLine cmd("ZedRecorder", ' ', "0.1");

		TCLAP::ValueArg<std::string> svoInputArg("i","svo-input","Input SVO file",true,"","SVO input file", cmd);

		TCLAP::ValueArg<std::string> loggerOutputArg("l","log-output","Output Logger filename",false,"","Output logger filename", cmd);
		TCLAP::ValueArg<std::string> compressionArg("","compression","",false,"snappy","{snappy}", cmd);

		TCLAP::ValueArg<std::string> imageOutputArg("","image-output","",false,"","Output image directory", cmd);
		TCLAP::ValueArg<std::string> videoOutputArg("","video-output","",false,"","Output video filename", cmd);
		TCLAP::ValueArg<std::string> videoFormatArg("","video-format","",false,"AVC1","Video format in fourcc format", cmd);

		TCLAP::ValueArg<int> skipArg("","skip","",false,1,"", cmd);

		TCLAP::ValueArg<int> startAtArg("","start-at","",false,0,"", cmd);


		TCLAP::ValueArg<std::string> statisticsOutputArg("","statistics-output","",false,"","", cmd);

		TCLAP::SwitchArg disableSelfCalibSwitch("","disable-self-calib","", cmd, false);

		TCLAP::SwitchArg depthSwitch("","depth","", cmd, false);
		TCLAP::SwitchArg rightSwitch("","right","", cmd, false);

		TCLAP::SwitchArg guiSwitch("","display","", cmd, false);

		cmd.parse(argc, argv );

		int compressLevel = logger::LogWriter::DefaultCompressLevel;
		if( compressionArg.isSet() ) {
			if( compressionArg.getValue() == "snappy" ){
				compressLevel = logger::LogWriter::SnappyCompressLevel;
			}else {
				try {
					compressLevel = std::stoi(compressionArg.getValue() );
				} catch ( std::invalid_argument &e ) {
					throw TCLAP::ArgException("Don't understand compression level.");
				}

			}
		}

		// Output validation
		if( !videoOutputArg.isSet() && !imageOutputArg.isSet() && !loggerOutputArg.isSet() && !guiSwitch.isSet() ) {
			LOG(WARNING) << "No output options set.";
			exit(-1);
		}

		libvideoio::Display display( guiSwitch.getValue() );
		libvideoio::ImageOutput imageOutput( imageOutputArg.getValue() );
		const bool needDepth = false; //( svoOutputArg.isSet() ? false : true );
		const int whichGpu = -1;

		sl::Camera camera;

		sl::InitParameters initParameters;

		LOG(INFO) << "Loading SVO file " << svoInputArg.getValue();
		initParameters.svo_input_filename = svoInputArg.getValue().c_str();

		initParameters.sdk_gpu_id = whichGpu;
		initParameters.sdk_verbose = true;
		initParameters.depth_mode = sl::DEPTH_MODE_MEDIUM;

		sl::RuntimeParameters runtimeParameters;
		runtimeParameters.enable_depth = true;
		runtimeParameters.enable_point_cloud = false;
		runtimeParameters.sensing_mode = sl::SENSING_MODE_STANDARD;

		sl::ERROR_CODE err = camera.open(initParameters);
		if (err != sl::SUCCESS) {
			LOG(FATAL) << "Unable to init the Zed camera (" << err << "): " << sl::errorCode2str(err);
			exit(-1);
		}

		// const bool recording = svoOutputArg.isSet();
		// if( recording ) {
		//
		// 	auto svoCompression = sl::SVO_COMPRESSION_MODE_LOSSLESS;
		// 	if( svoCompressionArg.getValue() == "none" ){
		// 		svoCompression = sl::SVO_COMPRESSION_MODE_RAW;
		// 		LOG(INFO) << "Disabling SVO compression";
		// 	} else if( svoCompressionArg.getValue() == "lossy" ){
		// 		svoCompression = sl::SVO_COMPRESSION_MODE_LOSSY;
		// 		LOG(INFO) << "Using lossy SVO compression";
		// 	} else {
		// 		LOG(INFO) << "Using lossless SVO compression";
		// 	}
		//
		// 	err = camera.enableRecording( svoOutputArg.getValue().c_str(), svoCompression  );
		//
		// 	if (err != sl::SUCCESS) {
		// 		LOG(FATAL) << "Error while setting up logging (" << err << "): " << sl::errorCode2str(err);
		// 		exit(-1);
		// 	}
		// }


		ZedSource dataSource( camera, depthSwitch.getValue() );

		// if( calibOutputArg.isSet() ) {
		// 	LOG(INFO) << "Saving calibration to \"" << calibOutputArg.getValue() << "\"";
		// 	calibrationFromZed( camera, calibOutputArg.getValue() );
		// }

		int numFrames = dataSource.numFrames();
		CHECK( numFrames > 0 );

		float fps = dataSource.fps();
		CHECK( fps >= 0 );

		LOG(INFO) << "Processing " << numFrames << " from video at " << fps << " FPS";

		logger::LogWriter logWriter( compressLevel );
		logger::FieldHandle_t leftHandle = 0, rightHandle = 1, depthHandle = 2;
		if( loggerOutputArg.isSet() ) {
			auto res = dataSource.imageSize();
			cv::Size sz( res.width, res.height);

			leftHandle = logWriter.registerField( "left", sz, logger::FIELD_BGRA_8C );
			if( depthSwitch.getValue() ) depthHandle = logWriter.registerField( "depth", sz, logger::FIELD_DEPTH_32F );
			if( rightSwitch.getValue() ) rightHandle = logWriter.registerField( "right", sz, logger::FIELD_BGRA_8C );

			if( !logWriter.open( loggerOutputArg.getValue() ) ) {
				LOG(FATAL) << "Unable to open file " << loggerOutputArg.getValue() << " for logging.";
			}
		}

		imageOutput.registerField( leftHandle, "left" );
		imageOutput.registerField( rightHandle, "right" );
		imageOutput.registerField( depthHandle, "depth" );

		libvideoio::VideoOutput videoOutput( videoOutputArg.getValue(),
		fps > 0 ? fps : 30, videoFormatArg.getValue() );

		// int dt_us = (fps > 0) ? (1e6/fps) : 0;
		// const float sleepFudge = 1.0;
		// dt_us *= sleepFudge;

		std::chrono::steady_clock::time_point startTime( std::chrono::steady_clock::now() );

		// Wait for the auto exposure and white balance
		std::this_thread::sleep_for(std::chrono::seconds(1));

		int count = 0, skip = skipArg.getValue();
		while( keepGoing ) {
			if( count > 0 && (count % 100)==0 ) LOG(INFO) << count << " frames";

			std::chrono::steady_clock::time_point loopStart( std::chrono::steady_clock::now() );

			if( dataSource.grab() ) {

				if( count < startAtArg.getValue() ) continue;

				cv::Mat left, right, depth;
				dataSource.getImage( 0, left );

				imageOutput.write( leftHandle, left );

				if( rightSwitch.getValue() ) {
					dataSource.getImage( 1, right );
					imageOutput.write( rightHandle, right );
				}

				if( depthSwitch.getValue() ) {
					dataSource.getDepth( depth );

					// Normalize depth
					double mn = 1.0, mx = 1.0;
					minMaxLoc( depth, &mn, &mx );
					Mat depthInt( depth.cols, depth.rows, CV_8UC1 ), depthNorm( depth.cols, depth.rows, depth.type() );
					depthNorm = depth * 255 / mx;
					depthNorm.convertTo( depthInt, CV_8UC1 );

					imageOutput.write( depthHandle, depthInt );
				}

				// Handler loger output
				if( loggerOutputArg.isSet() ) {
					logWriter.newFrame();
					logWriter.addField( leftHandle, left );
					if( rightSwitch.isSet() ) logWriter.addField( rightHandle, right.data );

					// Use non-normalized depth
					if( depthSwitch.getValue() ) logWriter.addField( depthHandle, depth.data );

					if( loggerOutputArg.isSet() ) {
						const bool doBlock = false; //( dt_us == 0 );
						if( !logWriter.writeFrame( doBlock ) ) {
							LOG(WARNING) << "Error while writing frame...";
						}
					}
				}

				// Handle video output
				if( videoOutput.isActive() ) {
					cv::Mat vidOut;
					if( rightSwitch.getValue() ) {
						CompositeCanvas comp( left, right );
						vidOut = comp;
					} else {
						vidOut = left;
					}
					videoOutput.write( vidOut );
				}

				// Handle display output
				if( count % skip == 0 ) {
					display.showLeft( left );

					if( rightSwitch.getValue() ) display.showRight( right );
					display.showDepth( depth );

					display.waitKey();
				}

			} else {
				LOG(WARNING) << "Problem grabbing from camera.";
			}

			count++;

			if( numFrames > 0 && count >= numFrames ) {
				keepGoing = false;
			}
		}


		LOG(INFO) << "Cleaning up...";
		//if( camera && svoOutputArg.isSet() ) camera->stopRecording();

		std::chrono::duration<float> dur( std::chrono::steady_clock::now()  - startTime );

		LOG(INFO) << "Recorded " << count << " frames in " <<   dur.count();
		LOG(INFO) << " Average of " << (float)count / dur.count() << " FPS";

		std::string fileName;
		// if( svoOutputArg.isSet() ) {
		// 	fileName = svoOutputArg.getValue();
		// } else

		if( loggerOutputArg.isSet() ) {
			logWriter.close();
			fileName = loggerOutputArg.getValue();
		}

		if( !fileName.empty() ) {
			unsigned int fileSize = fs::file_size( fs::path(fileName));
			float fileSizeMB = float(fileSize) / (1024*1024);
			LOG(INFO) << "Resulting file is " << fileSizeMB << " MB";
			LOG(INFO) << "     " << fileSizeMB/dur.count() << " MB/sec";
			LOG(INFO) << "     " << fileSizeMB/count << " MB/frame";

			if( statisticsOutputArg.isSet() ) {
				ofstream out( statisticsOutputArg.getValue(), ios_base::out | ios_base::ate | ios_base::app );
				if( out.is_open() ) {
					// TODO.  Include resolution in statistics output"
					out <<  "," << fps << "," << (guiSwitch.isSet() ? "display" : "") << "," << count << "," << dur.count() << ","
					<< fileSizeMB << endl;
				}
			}
		}

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{
		LOG(WARNING) << "error: " << e.error() << " for arg " << e.argId();
		exit(-1);
	}



	return 0;
}
