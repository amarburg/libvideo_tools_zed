
#include <g3log/g3log.hpp>

#include <sl/Camera.hpp>

#include "libvideoio/Camera.h"
#include "libvideoio/ImageSize.h"
using namespace libvideoio;

// As of API 1.0.0, the resolutions have changed slightly:
// 2.2K	4416x1242	15fps	Wide
// 1080p	3840x1080	15/30fps	Wide
// 720p	2560x720	15/30/60fps	Extra Wide
// VGA (Wide)	1344x376	15/30/60/100fps	Extra Wide

// "Casting" structs
struct ZedCamera : public libvideoio::Camera
{
	ZedCamera( const sl::CalibrationParameters &params )
      : libvideoio::Camera( params.left_cam.fx, params.left_cam.fy,
 														params.left_cam.cx, params.left_cam.cy )
    {;}

};

struct ZedImageSize : public ImageSize
{
	ZedImageSize( const sl::Resolution &res )
		: ImageSize( res.width, res.height)
	{;}
};

bool calibrationFromZed( sl::Camera &camera, const std::string &filename )
{
	std::ofstream out( filename );
	if( !out.is_open() ) {
//		LOG(WARNING) << "Unable to write to calibration file \"" << filename << "\"";
		return false;
	}

	sl::CameraInformation camInfo( camera.getCameraInformation() );

	const sl::CameraParameters &left( camInfo.calibration_parameters.left_cam );
	out << left.fx << " " << left.fy << " " << left.cx << " " << left.cy <<
 					" " << left.disto[0] << " " << left.disto[1] << " " << left.disto[2] <<
					" " << left.disto[3] << " " << left.disto[4] << std::endl;

	sl::Resolution res( left.image_size );
	out << res.width << " " << res.height << std::endl;

	return true;
}



inline sl::RESOLUTION parseResolution( const std::string &arg )
{
	if( arg == "hd2k" )
		return sl::RESOLUTION_HD2K;
	else if( arg == "hd1080" )
	 return sl::RESOLUTION_HD1080;
	else if( arg == "hd720" )
		return sl::RESOLUTION_HD720;
	else if( arg == "vga" )
		return sl::RESOLUTION_VGA;
	else
		;
		LOG(FATAL) << "Couldn't parse resolution \"" << arg << "\"";
}

inline std::string resolutionToString( sl::RESOLUTION arg )
{
	switch( arg ) {
		case sl::RESOLUTION_HD2K:
				return "HD2K";
		case sl::RESOLUTION_HD1080:
				return "HD1080";
		case sl::RESOLUTION_HD720:
				return "HD720";
		case sl::RESOLUTION_VGA:
				return "VGA";
		default:
				return "(unknown)";
	}
}


// This function is no longer in the core Zed distributino to reduce
// their dependency on OpenCV
cv::Mat slMat2cvMat(sl::Mat& input) {
	//convert MAT_TYPE to CV_TYPE
	int cv_type = -1;
	switch (input.getDataType()) {
		case sl::MAT_TYPE_32F_C1: cv_type = CV_32FC1; break;
		case sl::MAT_TYPE_32F_C2: cv_type = CV_32FC2; break;
		case sl::MAT_TYPE_32F_C3: cv_type = CV_32FC3; break;
		case sl::MAT_TYPE_32F_C4: cv_type = CV_32FC4; break;
		case sl::MAT_TYPE_8U_C1: cv_type = CV_8UC1; break;
		case sl::MAT_TYPE_8U_C2: cv_type = CV_8UC2; break;
		case sl::MAT_TYPE_8U_C3: cv_type = CV_8UC3; break;
		case sl::MAT_TYPE_8U_C4: cv_type = CV_8UC4; break;
		default: break;
	}

	// cv::Mat data requires a uchar* pointer. Therefore, we get the uchar1 pointer from sl::Mat (getPtr<T>())
	//cv::Mat and sl::Mat will share the same memory pointer
	return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(sl::MEM_CPU));
}
