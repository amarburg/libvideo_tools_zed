///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////




/**************************************************************************************************
 ** This sample demonstrates how to record a SVO file with the ZED SDK function                   **
 ** optionally, images are displayed using OpenCV                                                 **
 ***************************************************************************************************/

/* this sample use BOOST for program options arguments */


//standard includes
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <chrono>
#include <thread>


//Opencv Include (for display
#include <opencv2/opencv.hpp>

//ZED Include
#include <zed/Mat.hpp>
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>


//boost for program options
#include <boost/program_options.hpp>

#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace sl::zed;
using namespace std;
namespace po = boost::program_options;

Camera* zed = NULL;
volatile bool _run;

#ifdef _WIN32

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {

            // Handle the CTRL-C signal.
	if (fdwCtrlType== CTRL_C_EVENT)
	{
		_run = false;
		printf("\nSaving file...\n");
		zed->stopRecording();
		printf("\Done...\n");

	}


	return FALSE;
}
#else

void nix_exit_handler(int s) {
	_run = false;

}
#endif

int main(int argc, char **argv) {
    std::string filename = "zed_record.svo";

    bool display = 0;
    int resolution = 2; //Default resolution is set to HD720

    // Option handler (boost program_options)
    int opt;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("display,d", "toggle image display (might slow down the recording)")
            ("resolution,r", po::value<int>(&opt)->default_value(2), "ZED Camera resolution \narg: 0: HD2K   1: HD1080   2: HD720   3: VGA")
            ("filename,f", po::value< std::string >(), "Record filename")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    if (vm.count("display")) {
        cout << "Display on" << endl;
        display = 1;
    }
    if (vm.count("filename")) {
        filename = vm["filename"].as<std::string>();
        cout << "Filename was set to " << filename << endl;
    }
    if (vm.count("resolution")) {
        resolution = vm["resolution"].as<int>();
        switch (resolution) {
            case 0: cout << "Resolution set to HD2K" << endl;
                break;
            case 1: cout << "Resolution set to HD1080" << endl;
                break;
            case 2: cout << "Resolution set to HD720" << endl;
                break;
            case 3: cout << "Resolution set to VGA" << endl;
                break;
        }
    }
    // Camera init
    ERRCODE err;
    // Create the camera at HD 720p resolution
    // The realtime recording will depend on the write speed of your disk.
    zed = new Camera(static_cast<ZEDResolution_mode> (resolution));
    // ! not the same Init function - a more lighter one , specific for recording !//
    sl::zed::InitParams param;
    param.mode = NONE;
    param.verbose = true;


    err = zed->init(param);

    //zed_ptr = zed; // To call Camera::stop_recording() from the exit handler function

    std::cout << "ERR code Init: " << errcode2str(err) << std::endl;

    // Quit if an error occurred
    if (err != SUCCESS) {
        delete zed;
        return 1;
    }

    err = zed->enableRecording(filename);
    std::cout << "ERR code Recording: " << errcode2str(err) << std::endl;

    if (err != SUCCESS) {
        delete zed;
        return 1;
    }

    // CTRL-C (= kill signal) handler
#ifdef _WIN32
	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
		printf("\nERROR: Could not set control handler");
		return 1;
	}

#else // unix
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = nix_exit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif

	_run = true;
    // Wait for the auto exposure and white balance
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Recording loop
    cout << "Recording..." << endl;
    cout << "Press 'Ctrl+C' to stop and exit " << endl;
	int count = 0;
	while (_run && count < 1200) {


        //simple recording function
        if (!zed->grab(SENSING_MODE::STANDARD,0,0,0))
        {
            // A new frame is available
            // You can ingest it in the video file by calling record().
            zed->record();

++count;

            if (display) zed->displayRecorded(); // convert the image to RGB and display it
            std::this_thread::sleep_for(std::chrono::microseconds(1));

        }
		else
			std::this_thread::sleep_for(std::chrono::microseconds(100));

    }


#ifdef _WIN32
	delete zed;
#else
printf("Saved %d frames\n", count );
    printf("Saving file...\n");
    zed->stopRecording();
    printf("Done...\n");

    if (zed) delete zed;

#endif

    return 0;
}
