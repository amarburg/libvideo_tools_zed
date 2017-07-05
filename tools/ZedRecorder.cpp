
#include <string>
#include <thread>
using namespace std;

#include <signal.h>

#include <opencv2/opencv.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


#include <sl/Camera.hpp>
#include "ZedUtils.h"


#include <tclap/CmdLine.h>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include "libvideoio/G3LogSinks.h"

#include "libvideoio/Display.h"
using namespace libvideoio;




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

	//try {
	TCLAP::CmdLine cmd("ZedRecorder", ' ', "0.1");

	TCLAP::ValueArg<std::string> resolutionArg("r","resolution","Input resolution: hd2k,hd1080,hd720,vga",false,"hd1080","", cmd);
	TCLAP::ValueArg<float> fpsArg("f","fps","Input FPS, otherwise defaults to max FPS from input source",false,0.0,"", cmd);

	TCLAP::ValueArg<std::string> svoOutputArg("s","svo-output","Output SVO file",false,"","", cmd);

	TCLAP::ValueArg<std::string> calibOutputArg("","calib-output","Output calibration file (from stereolabs SDK)",false,"","Calib filename", cmd);

	TCLAP::ValueArg<std::string> statisticsOutputArg("","statistics-output","",false,"","", cmd);
	TCLAP::ValueArg<std::string> statisticsIdArg("","statistics-id","",false,"","", cmd);


	// TCLAP::SwitchArg noGuiSwitch("","no-gui","Don't show a GUI", cmd, false);

	TCLAP::SwitchArg disableSelfCalibSwitch("","disable-self-calib","", cmd, false);

	TCLAP::SwitchArg guiSwitch("","display","", cmd, false);

	TCLAP::ValueArg<std::string> svoCompressionArg("","svo-compression","", false, "lossless", "lossless, lossy, none", cmd );


	TCLAP::ValueArg<int> durationArg("","duration","Duration",false,0,"seconds", cmd);
	TCLAP::ValueArg<int> numFramesArg("","frames","Number of frames to capture",false,0,"frames", cmd);
	TCLAP::ValueArg<int> skipArg("","skip","Only display every <skip> frames",false,10,"frames", cmd);


	cmd.parse(argc, argv );

	// Output validation
	if( !svoOutputArg.isSet() && !guiSwitch.isSet() ) {
		LOG(WARNING) << "No output options set.";
	}

	const bool doDisplay = guiSwitch.getValue();
	libvideoio::Display display( guiSwitch.getValue() );

	int skip = skipArg.getValue();

	//} catch (TCLAP::ArgException &e)  // catch any exceptions
	//	{
	//		std::cout<< "error: " << e.error() << " for arg " << e.argId();
	//		exit(-1);
	//	}


	const bool needDepth = false; //( svoOutputArg.isSet() ? false : true );
	const int whichGpu = -1;

	sl::Camera camera;

	sl::InitParameters initParameters;
	sl::RESOLUTION zedResolution = parseResolution( resolutionArg.getValue() );
	initParameters.camera_resolution = zedResolution;
	initParameters.sdk_gpu_id = whichGpu;
	initParameters.sdk_verbose = true;
	initParameters.depth_mode = sl::DEPTH_MODE_NONE;
	if( fpsArg.isSet() ) initParameters.camera_fps = fpsArg.getValue();

	sl::RuntimeParameters runtimeParameters;
	runtimeParameters.enable_depth = false;
	runtimeParameters.enable_point_cloud = false;
	runtimeParameters.sensing_mode = sl::SENSING_MODE_STANDARD;

	sl::ERROR_CODE err = camera.open(initParameters);
	if (err != sl::SUCCESS) {
		LOG(FATAL) << "Unable to init the Zed camera (" << err << "): " << sl::errorCode2str(err);
		exit(-1);
	}

	const bool recording = svoOutputArg.isSet();
	if( recording ) {

		auto svoCompression = sl::SVO_COMPRESSION_MODE_LOSSLESS;
		if( svoCompressionArg.getValue() == "none" ){
			svoCompression = sl::SVO_COMPRESSION_MODE_RAW;
			LOG(INFO) << "Disabling SVO compression";
		} else if( svoCompressionArg.getValue() == "lossy" ){
			svoCompression = sl::SVO_COMPRESSION_MODE_LOSSY;
			LOG(INFO) << "Using lossy SVO compression";
		} else {
			LOG(INFO) << "Using lossless SVO compression";
		}

		err = camera.enableRecording( svoOutputArg.getValue().c_str(), svoCompression  );

		if (err != sl::SUCCESS) {
			LOG(FATAL) << "Error while setting up logging (" << err << "): " << sl::errorCode2str(err);
			exit(-1);
		}
	}

	int numFrames = 0; //dataSource->numFrames();

	// Wait for the auto exposure and white balance
	std::this_thread::sleep_for(std::chrono::seconds(1));

	const int duration = durationArg.getValue();

	if( duration > 0 ) {
		LOG(INFO) << "Will log for " << duration << " seconds or press CTRL-C to stop.";
	} else {
		LOG(INFO) << "Logging now, press CTRL-C to stop.";
	}

	std::chrono::steady_clock::time_point start( std::chrono::steady_clock::now() );
	std::chrono::steady_clock::time_point end( start + std::chrono::seconds( duration ) );

	int count = 0, miss = 0, displayed = 0;
	sl::RecordingState recordStatus;

	bool logOnce = true;
	while( keepGoing ) {

		std::chrono::steady_clock::time_point loopStart( std::chrono::steady_clock::now() );
		if( (duration > 0) && (loopStart > end) ) { keepGoing = false;  break; }

		auto err = camera.grab( runtimeParameters );

		if( err == sl::SUCCESS ) {

			if( recording ) {
				 recordStatus = camera.record();

				if( recordStatus.status == false ){
					LOG(WARNING) << "Bad status when recording frame to SVO";

				}
			}


			const bool doDisplayThisFrame = (doDisplay && (count % skip == 0));

			// Haven't tested this yet...
			// if( svoOutputArg.isSet() ) {
			//
			//
			// 	if( doDisplayThisFrame ) {
			// 		// According to the docs, this:
			// 		//		Get[s] the current side by side YUV 4:2:2 frame, CPU buffer.
			// 		sl::Mat slRawImage( camera.getCurrentRawRecordedFrame() );
			// 		// Make a copy before enqueueing
			//
			// 		Mat rawCopy;
			// 		sl::slMat2cvMat( slRawImage ).reshape( 2, 0 ).copyTo( rawCopy );
			//
			// 		display.showRawStereoYUV( rawCopy );
			// 		display.waitKey();
			//
			// 		++displayed;
			//
			// 	}
			//
			// } else if ( doDisplayThisFrame ) {
			// 		// If you aren't recording, just grab data conventionally


					sl::Mat leftImage;
					auto err = camera.retrieveImage( leftImage, sl::VIEW_LEFT );

					cv::Mat cvCopy;
					slMat2cvMat( leftImage ).copyTo( cvCopy );

					display.showLeft( cvCopy );
					display.waitKey();

			//}

			++count;
			std::this_thread::sleep_for(std::chrono::microseconds(1));

		} else if( err == sl::ERROR_CODE_NOT_A_NEW_FRAME ) {
			// No new frame ready yet...
		} else {
			LOG(INFO) << "Error grabbing frame: " << sl::errorCode2str( err );

			// if grab() fails
			++miss;
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

		if( count > 0 && (count % 100)==0 ) {
			if( logOnce ) {
				LOG(INFO) << count << " frames";
				LOG_IF(INFO, recording ) << "Recording avg. " << recordStatus.average_compression_ratio << "% compression, avg. " << recordStatus.average_compression_time << " ms";
			}
			logOnce = false;
		} else {
			logOnce = true;
		}


			//			if( dt_us > 0 ) {
			//				std::chrono::steady_clock::time_point sleepTarget( loopStart + std::chrono::microseconds( dt_us ) );
			//				//if( std::chrono::steady_clock::now() < sleepTarget )
			//				std::this_thread::sleep_until( sleepTarget );
			//			}

			if( numFramesArg.isSet() && count >= numFramesArg.getValue() ) keepGoing = false;

		}

		std::chrono::duration<float> dur( std::chrono::steady_clock::now()  - start );

		auto fps = camera.getCameraFPS();

		LOG(INFO) << "Cleaning up...";
		if( recording ) camera.disableRecording();

		LOG(INFO) << "Recorded " << count << " frames in " <<   dur.count();
		LOG(INFO) << " Average of " << (float)count / dur.count() << " FPS";
		LOG(INFO) << "   " << miss << " misses / " << (miss+count) << " frames";
		LOG_IF( INFO, displayed > 0 ) << "   Displayed " << displayed << " frames";

		std::string fileName(svoOutputArg.getValue());

		if( !fileName.empty() ) {
			unsigned int fileSize = fs::file_size( fs::path(fileName));
			float fileSizeMB = float(fileSize) / (1024*1024);
			LOG(INFO) << "Resulting file is " << fileSizeMB << " MB";
			LOG(INFO) << "     " << fileSizeMB/dur.count() << " MB/sec";
			LOG(INFO) << "     " << fileSizeMB/count << " MB/frame";



			if( statisticsOutputArg.isSet() ) {
				ofstream out( statisticsOutputArg.getValue(), ios_base::out | ios_base::ate | ios_base::app );
				if( out.is_open() ) {
					if( statisticsIdArg.isSet() )
					out << statisticsIdArg.getValue() << ',' << resolutionToString( zedResolution ) << "," << fps << "," << (guiSwitch.isSet() ? "display" : "") << "," << count << "," << dur.count() << "," << fileSizeMB << endl;
					else
					out << resolutionToString( zedResolution ) << "," << fps << "," << (guiSwitch.isSet() ? "display" : "") << "," << count << "," << dur.count() << "," << fileSizeMB << endl;
				}
			}
		}


		return 0;
	}
