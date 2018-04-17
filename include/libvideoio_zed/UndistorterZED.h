#pragma once


#include "libvideoio/Undistorter.h"
#include "ZedUtils.h"

#include <sl/Camera.hpp>

namespace lsd_slam
{

class UndistorterZED : public UndistorterLogger
{
  public:

  UndistorterZED( sl::Camera *camera  );

  ~UndistorterZED();

};

}
