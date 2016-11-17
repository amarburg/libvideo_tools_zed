
// TODO:   Reduce the DRY

#include <string>
#include <thread>
using namespace std;

#include <signal.h>

#include <opencv2/opencv.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


#include <zed/Camera.hpp>
//#include "libvideoio_zed/ZedUtils.h"
//#include "libvideoio_zed/ZedSource.h"


#include <tclap/CmdLine.h>

//#include "libvideoio/DataSource.h"
//#include "libvideoio/Undistorter.h"
//
//#include "logger/LogWriter.h"
//
//#include "libvideoio/Display.h"


//using namespace lsd_slam;


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
	signal( SIGINT, signal_handler );

	//try {
		TCLAP::CmdLine cmd("LSDRecorder", ' ', "0.1");

		TCLAP::ValueArg<std::string> resolutionArg("r","resolution","Input resolution: hd2k,hd1080,hd720,vga",false,"hd1080","", cmd);
		TCLAP::ValueArg<float> fpsArg("f","fps","Input FPS, otherwise defaults to max FPS from input source",false,0.0,"", cmd);

		TCLAP::ValueArg<std::string> svoOutputArg("s","svo-output","Output SVO file",true,"","", cmd);

		TCLAP::ValueArg<std::string> calibOutputArg("","calib-output","Output calibration file (from stereolabs SDK)",false,"","Calib filename", cmd);

		TCLAP::ValueArg<std::string> statisticsOutputArg("","statistics-output","",false,"","", cmd);

		// TCLAP::SwitchArg noGuiSwitch("","no-gui","Don't show a GUI", cmd, false);

		TCLAP::SwitchArg disableSelfCalibSwitch("","disable-self-calib","", cmd, false);

		TCLAP::SwitchArg guiSwitch("","display","", cmd, false);


		TCLAP::ValueArg<int> durationArg("","duration","Duration",false,0,"seconds", cmd);
		TCLAP::ValueArg<int> numFramesArg("","frames","Number of frames to capture",false,0,"frames", cmd);


		cmd.parse(argc, argv );

		// Output validation
		if( !svoOutputArg.isSet() && !guiSwitch.isSet() ) {
			std::cout << "No output options set." << std::endl;
			exit(-1);
		}

		//zed_recorder::Display display( guiSwitch.getValue() );
//} catch (TCLAP::ArgException &e)  // catch any exceptions
//	{
//		std::cout<< "error: " << e.error() << " for arg " << e.argId();
//		exit(-1);
//	}

		const bool needDepth = false; //( svoOutputArg.isSet() ? false : true );
		const sl::zed::ZEDResolution_mode zedResolution = sl::zed::VGA; //parseResolution( resolutionArg.getValue() );
		const int whichGpu = -1;

		sl::zed::Camera *camera = NULL;

		//LOG_IF( FATAL, calibOutputArg.isSet() && svoOutputArg.isSet() ) << "Calibration data is only generated when using live video, not when recording to SVO.";

			//LOG(INFO) << "Using live Zed data";
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
			std::cout << "Unable to init the Zed camera (" << err << "): " << errcode2str(err) <<std::endl;
			delete camera;
			exit(-1);
		}

			err = camera->enableRecording( svoOutputArg.getValue() );

			if (err != sl::zed::SUCCESS) {
				std::cout << "Error while setting up logging (" << err << "): " << errcode2str(err) << std::endl;
			}

		//dataSource = new ZedSource( camera, needDepth );

		//if( calibOutputArg.isSet() ) {
		//		LOG(INFO) << "Saving calibration to \"" << calibOutputArg.getValue() << "\"";
		//		calibrationFromZed( camera, calibOutputArg.getValue() );
		//}

		int numFrames = 0; //dataSource->numFrames();

		std::chrono::steady_clock::time_point start( std::chrono::steady_clock::now() );
		int duration = durationArg.getValue();
		std::chrono::steady_clock::time_point end( start + std::chrono::seconds( duration ) );

		if( duration > 0 )
			std::cout << "Will log for " << duration << " seconds or press CTRL-C to stop." << std::endl;
		else
			std::cout<< "Logging now, press CTRL-C to stop." << std::endl;

		// Wait for the auto exposure and white balance
		std::this_thread::sleep_for(std::chrono::seconds(1));

		int count = 0, skip = 10;
		while( keepGoing ) {

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

		if( numFramesArg.isSet() && count >= numFramesArg.getValue() ) {
			keepGoing = false;
		}
}


		std::cout<< "Cleaning up..." << std::endl;
		camera->stopRecording();
		if( camera ) delete camera;


		std::chrono::duration<float> dur( std::chrono::steady_clock::now()  - start );

		std::cout<< "Recorded " << count << " frames in " <<   dur.count();
		std::cout<< " Average of " << (float)count / dur.count() << " FPS";

		std::string fileName(svoOutputArg.getValue());


		if( !fileName.empty() ) {
			unsigned int fileSize = fs::file_size( fs::path(fileName));
			float fileSizeMB = float(fileSize) / (1024*1024);
			std::cout<< "Resulting file is " << fileSizeMB << " MB" << std::endl;
			std::cout<< "     " << fileSizeMB/dur.count() << " MB/sec" << std::endl;
			std::cout << "     " << fileSizeMB/count << " MB/frame" << std::endl;
		}








	return 0;
}
