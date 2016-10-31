
#include <g3log/g3log.hpp>

#include <zed/Camera.hpp>

#include "libvideo_tools/Camera.h"
#include "libvideo_tools/ImageSize.h"


// "Casting" structs
struct ZedCamera : public lsd_slam::Camera
{
	ZedCamera( sl::zed::StereoParameters* params )
      : lsd_slam::Camera( params->LeftCam.fx, params->LeftCam.fy,
 								params->LeftCam.cx, params->LeftCam.cy )
    {;}

};

struct ZedImageSize : public lsd_slam::ImageSize
{
	ZedImageSize( const sl::zed::resolution &res )
		: lsd_slam::ImageSize( res.width, res.height)
	{;}
};

bool calibrationFromZed( sl::zed::Camera *camera, const std::string &filename )
{
	std::ofstream out( filename );
	if( !out.is_open() ) {
		LOG(WARNING) << "Unable to write to calibration file \"" << filename << "\"";
		return false;
	}

	sl::zed::StereoParameters *params = camera->getParameters();

	const sl::zed::CamParameters &left( params->LeftCam );
	out << left.fx << " " << left.fy << " " << left.cx << " " << left.cy <<
 					" " << left.disto[0] << " " << left.disto[1] << " " << left.disto[2] <<
					" " << left.disto[3] << " " << left.disto[4] << std::endl;

	sl::zed::resolution res( camera->getImageSize() );
	out << res.width << " " << res.height << std::endl;

	return true;
}



inline sl::zed::ZEDResolution_mode parseResolution( const std::string &arg )
{
	if( arg == "hd2k" )
		return sl::zed::HD2K;
	else if( arg == "hd1080" )
	 return sl::zed::HD1080;
	else if( arg == "hd720" )
		return sl::zed::HD720;
	else if( arg == "vga" )
		return sl::zed::VGA;
	else
		LOG(FATAL) << "Couldn't parse resolution \"" << arg << "\"";
}

inline std::string resolutionToString( sl::zed::ZEDResolution_mode arg )
{
	switch( arg ) {
		case sl::zed::HD2K:
				return "HD2K";
		case sl::zed::HD1080:
				return "HD1080";
		case sl::zed::HD720:
				return "HD720";
		case sl::zed::VGA:
				return "VGA";
		default:
				return "(unknown)";
	}
}
