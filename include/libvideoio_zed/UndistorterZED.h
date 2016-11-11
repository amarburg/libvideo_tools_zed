#pragma once


#include "libvideoio/Undistorter.h"
#include "ZedUtils.h"

#include <zed/Camera.hpp>

namespace lsd_slam
{

class UndistorterZED : public UndistorterLogger
{
  public:

  UndistorterZED( sl::zed::Camera *camera  );

~UndistorterZED();


};

}
