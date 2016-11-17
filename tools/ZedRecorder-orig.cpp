
// TODO:   Reduce the DRY

#include <string>
using namespace std;

#include <opencv2/opencv.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <zed/Camera.hpp>
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
		TCLAP::CmdLine cmd("LSDRecorder", ' ', "0.1");

		TCLAP::ValueArg<std::string> resolutionArg("r","resolution","Input resolution: hd2k,hd1080,hd720,vga",false,"hd1080","", cmd);
		TCLAP::ValueArg<float> fpsArg("f","fps","Input FPS, otherwise defaults to max FPS from input source",false,0.0,"", cmd);

		TCLAP::ValueArg<std::string> svoOutputArg("s","svo-output","Output SVO file",false,"","", cmd);

		TCLAP::ValueArg<std::string> calibOutputArg("","calib-output","Output calibration file (from stereolabs SDK)",false,"","Calib filename", cmd);

		TCLAP::ValueArg<std::string> statisticsOutputArg("","statistics-output","",false,"","", cmd);

		// TCLAP::SwitchArg noGuiSwitch("","no-gui","Don't show a GUI", cmd, false);

		TCLAP::SwitchArg disableSelfCalibSwitch("","disable-self-calib","", cmd, false);

		TCLAP::SwitchArg guiSwitch("","display","", cmd, false);


		TCLAP::ValueArg<int> durationArg("","duration","Duration",false,0,"seconds", cmd);

		cmd.parse(argc, argv );

		// Output validation
		if( !svoOutputArg.isSet() && !guiSwitch.isSet() ) {
			LOG(WARNING) << "No output options set.";
			exit(-1);
		}

		zed_recorder::Display display( guiSwitch.getValue() );

		const bool needDepth = false; //( svoOutputArg.isSet() ? false : true );
		const sl::zed::ZEDResolution_mode zedResolution = parseResolution( resolutionArg.getValue() );
		const int whichGpu = -1;

		DataSource *dataSource = NULL;
		sl::zed::Camera *camera = NULL;

		LOG_IF( FATAL, calibOutputArg.isSet() && svoOutputArg.isSet() ) << "Calibration data is only generated when using live video, not when recording to SVO.";

			LOG(INFO) << "Using live Zed data";
			camera = new sl::zed::Camera(  zedResolution ); //, 60.0 ); //fpsArg.getValue() );

		sl::zed::ERRCODE err = sl::zed::LAST_ERRCODE;
		sl::zed::InitParams initParams;
		const sl::zed::MODE zedMode = sl::zed::MODE::NONE;  //(needDepth ? sl::zed::MODE::PERFORMANCE : sl::zed::MODE::NONE);
		initParams.mode = zedMode;
		const bool verboseInit = true;
		initParams.verbose = verboseInit;
		//initParams.disableSelfCalib = true; // disableSelfCalibSwitch.getValue();
		err = camera->init( initParams );

		if (err != sl::zed::SUCCESS) {
			LOG(WARNING) << "Unable to init the Zed camera (" << err << "): " << errcode2str(err);
			delete camera;
			exit(-1);
		}

		if( svoOutputArg.isSet() ) {
			err = camera->enableRecording( svoOutputArg.getValue() );

			if (err != sl::zed::SUCCESS) {
				LOG(WARNING) << "Error while setting up logging (" << err << "): " << errcode2str(err);
			}
		}

		//dataSource = new ZedSource( camera, needDepth );

		//if( calibOutputArg.isSet() ) {
		//		LOG(INFO) << "Saving calibration to \"" << calibOutputArg.getValue() << "\"";
		//		calibrationFromZed( camera, calibOutputArg.getValue() );
		//}

		int numFrames = 0; //dataSource->numFrames();
		float fps = 60.0; //= dataSource->fps();

		CHECK( fps >= 0 );

		int dt_us = (fps > 0) ? (1e6/fps) : 0;
		const float sleepFudge = 0.1;
		dt_us *= sleepFudge;
dt_us = 0;

		LOG(INFO) << "Input is at " << resolutionToString( zedResolution ) << " at nominal " << fps << "FPS";

		std::chrono::steady_clock::time_point start( std::chrono::steady_clock::now() );
		int duration = durationArg.getValue();
		std::chrono::steady_clock::time_point end( start + std::chrono::seconds( duration ) );

		if( duration > 0 )
			LOG(INFO) << "Will log for " << duration << " seconds or press CTRL-C to stop.";
		else
			LOG(INFO) << "Logging now, press CTRL-C to stop.";

		// Wait for the auto exposure and white balance
		std::this_thread::sleep_for(std::chrono::seconds(1));

		int count = 0, skip = 10;
		while( keepGoing  && count < 1200 ) {
			//if( count > 0 && (count % 100)==0 ) LOG(INFO) << count << " frames";

			//std::chrono::steady_clock::time_point loopStart( std::chrono::steady_clock::now() );

			//if( (duration > 0) && (loopStart > end) ) { keepGoing = false;  break; }

//				if( !dataSource->grab() ) {
//					LOG(WARNING) << "Error occured while recording from camera";
//				}

				if( !camera->grab( sl::zed::STANDARD, false, false, false ) ) {

				camera->record();

				// } else if( count % skip == 0 ) {
				// 	// According to the docs, this:
				// 	//		Get[s] the current side by side YUV 4:2:2 frame, CPU buffer.
				// 	sl::zed::Mat slRawImage( camera->getCurrentRawRecordedFrame() );
				// 	// Make a copy before enqueueing
				// 	Mat rawCopy;
				// 	sl::zed::slMat2cvMat( slRawImage ).reshape( 2, 0 ).copyTo( rawCopy );
				// 	display.showRawStereoYUV( rawCopy );
				// }
++count;
 std::this_thread::sleep_for(std::chrono::microseconds(1));

} else {
 std::this_thread::sleep_for(std::chrono::microseconds(100));
}

			//if( count % skip == 0 )
			//	display.waitKey();


//			if( dt_us > 0 ) {
//				std::chrono::steady_clock::time_point sleepTarget( loopStart + std::chrono::microseconds( dt_us ) );
//				//if( std::chrono::steady_clock::now() < sleepTarget )
//				std::this_thread::sleep_until( sleepTarget );
//			}



			//if( numFrames > 0 && count >= numFrames ) {
			//	keepGoing = false;
			//}
		}


		LOG(INFO) << "Cleaning up...";
		if( camera && svoOutputArg.isSet() ) camera->stopRecording();

		std::chrono::duration<float> dur( std::chrono::steady_clock::now()  - start );

		LOG(INFO) << "Recorded " << count << " frames in " <<   dur.count();
		LOG(INFO) << " Average of " << (float)count / dur.count() << " FPS";

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
					out << resolutionToString( zedResolution ) << "," << fps << "," << (guiSwitch.isSet() ? "display" : "") << "," << count << "," << dur.count() << ","
							<< fileSizeMB << endl;
				}
			}
		}



		if( dataSource ) delete dataSource;
		if( camera ) delete camera;

	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{
		LOG(WARNING) << "error: " << e.error() << " for arg " << e.argId();
		exit(-1);
	}



	return 0;
}
