#pragma once


#include <sl/Camera.hpp>
#include "libvideoio/DataSource.h"

namespace lsd_slam {

class ZedSource : public DataSource {
public:

	ZedSource( sl::Camera &camera, bool doComputeDepth = false, sl::SENSING_MODE mode = sl::SENSING_MODE_STANDARD )
    : _cam( camera ),
      _computeDepth( doComputeDepth ),
			_runtimeParameters( mode, true, false )

  {
    _numImages = 2;
    _hasDepth = true;

    //_fps = _cam->getCurrentFPS();

    //LOG(INFO) << "Camera reports fps: " << _fps;
  }

  // Delete copy operators
  ZedSource( const ZedSource & ) = delete;
  ZedSource &operator=( const ZedSource & ) = delete;

  virtual int numFrames( void ) const
  {
    return _cam.getSVONumberOfFrames();
  };



  virtual bool grab( void )
  {
		auto err = _cam.grab( _runtimeParameters );
    if(  err != sl::SUCCESS ) {
      LOG( WARNING ) << "Error from Zed::grab: " << sl::errorCode2str(err);
      return false;
    }

    return true;
  }

  virtual int getImage( int i, cv::Mat &cvMat )
  {
		sl::Mat slMat;

    if( i == 0 )
      _cam.retrieveImage( slMat, sl::VIEW_LEFT );
    else if( i == 1 )
			_cam.retrieveImage( slMat, sl::VIEW_RIGHT );

		cvMat = slMat2cvMat( slMat );
    return 0;
  }

  virtual void getDepth( cv::Mat &cvMat )
  {
			sl::Mat slMat;

      if( _computeDepth )
					_cam.retrieveMeasure(slMat, sl::MEASURE_DEPTH );
      else
        LOG(WARNING) << "Asked for depth after begin configured not to compute depth";

			cvMat = slMat2cvMat( slMat );
  }

  virtual ImageSize imageSize( void ) const
  {
		sl::CameraInformation camInfo( _cam.getCameraInformation() );
    return ZedImageSize( camInfo.calibration_parameters.left_cam.image_size );
  }

protected:

  sl::Camera &_cam;
	sl::RuntimeParameters _runtimeParameters;
  bool _computeDepth;


};

}
