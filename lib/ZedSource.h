#pragma once


#include <sl/Camera.hpp>
#include "libvideoio/DataSource.h"

namespace lsd_slam {

class ZedSource : public DataSource {
public:

	ZedSource( sl::Camera *camera, bool doComputeDepth = false, sl::SENSING_MODE mode = sl::RAW )
    : _cam( camera ),
      _mode( mode ),
      _computeDepth( doComputeDepth )

  {
    CHECK( _cam );
    _numImages = 2;
    _hasDepth = true;

    _fps = _cam->getCurrentFPS();

    LOG(INFO) << "Camera reports fps: " << _fps;
  }

  // Delete copy operators
  ZedSource( const ZedSource & ) = delete;
  ZedSource &operator=( const ZedSource & ) = delete;

  virtual int numFrames( void ) const
  {
    return _cam->getSVONumberOfFrames();
  };

  virtual bool grab( void )
  {
    if( _cam->grab( sl::STANDARD, false, false, false ) ) {
//    if( _cam->grab( _mode, _computeDepth, _computeDepth, false ) ) {
      LOG( WARNING ) << "Error from Zed::grab";
      return false;
    }

    return true;
  }

  virtual int getImage( int i, cv::Mat &mat )
  {
    if( i == 0 )
      mat = sl::slMat2cvMat( _cam->retrieveImage( sl::LEFT ) );
    else if( i == 1 )
      mat = sl::slMat2cvMat( _cam->retrieveImage( sl::RIGHT ) );

    return 0;
  }

  virtual void getDepth( cv::Mat &mat )
  {
      if( _computeDepth )
        mat = sl::slMat2cvMat( _cam->retrieveMeasure( sl::DEPTH ) );
      else
        LOG(WARNING) << "Asked for depth after begin configured not to compute depth";
  }

  virtual ImageSize imageSize( void ) const
  {
    return ZedImageSize( _cam->getImageSize() );
  }

protected:

  sl::Camera *_cam;
  sl::SENSING_MODE _mode;
  bool _computeDepth;


};

}
