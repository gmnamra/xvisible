/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_cubic_generators.cpp
 *   Creation Date  09/10/2003
 *   Author         Peter Roberts
 *
 * rcCubicGeneratePixel is a concrete derived version of rcGeneratePixel.
 * It implements a cubic spline pixel interpolation algorithm.
 *
 ***************************************************************************/

#include <rc_cubic_generators.h>

double rcCubicGeneratePixel::genPixel(const rcAffineWindow* src,
					     double xoff, double yoff)
{
  rmAssert(src);
  const rcRect bounds(src->ar().boundingBox());
  rcWindow boundingBox(*src, bounds);

  const uint32 height = boundingBox.height();
  rmAssert(height);
  const uint32 width = boundingBox.width();
  rmAssert(width);

  if (_y2d.size() != height || _y2d[0].size() != width)
    gen2D2ndD(boundingBox);
  
  if (_ytmp.size() != height)
    _ytmp.resize(height);

  if (_y2tmp.size() != height)
    _y2tmp.resize(height);

  float xoffset = (float)xoff - bounds.x();

  for (uint32 y = 0; y < height; y++)
    _ytmp[y] = interpolate(boundingBox, y, _y2d[y], xoffset);

  gen2ndD(_ytmp, _y2tmp, 0.0, 0.0, false, false);

  float yoffset = (float)yoff - bounds.y();

  return interpolate(_ytmp, _y2tmp, yoffset);
}

void rcCubicGeneratePixel::clear()
{
  _y2d.resize(0);
  _ytmp.resize(0);
  _y2tmp.resize(0);
}

void rcCubicGeneratePixel::gen2D2ndD(const rcWindow& boundingBox)
{
  switch (boundingBox.depth())
  {
  case rcPixel8:
    _maxVal = rcUINT8_MAX;
    break;

  case rcPixel16:
    _maxVal = rcUINT16_MAX;
    break;

  case rcPixel32S:
    _maxVal = rcUINT32_MAX;
    break;

  case rcPixelDouble:
    _maxVal = 0.0;
    break;

  default:
    rmAssert(0);
    break;
  }

  if (boundingBox.height() != (int32)_y2d.size())
    _y2d.resize(boundingBox.height());

  for (uint32 y = 0; y < _y2d.size(); y++) {
    if (boundingBox.width() != (int32)_y2d[y].size())
      _y2d[y].resize(boundingBox.width());
    gen2ndD(boundingBox, y, _y2d[y], 0.0, 0.0, false, false);
  }
}

void rcCubicGeneratePixel::gen2ndD(const vector<float>& y, vector<float>& y2,
				   float yp1, float ypn, bool v1, bool vn)
{
  uint32 n = y.size(), last = n - 1;

  if (n != y2.size())
    y2.resize(n);

  vector<float> u(last);

  if (v1) {
    y2[0] = -0.5;
    u[0] = 3.0*((y[1]-y[0]) - yp1);
  }
  else
    y2[0] = u[0] = 0.0;

  for (uint32 i = 1; i < last; i++) {
    float p = 0.5 * y2[i-1] + 2.0;
    y2[i] = -0.5/p;
    u[i] = (y[i+1] - y[i]) - (y[i] - y[i-1]);
    u[i] = (3*u[i] - 0.5*u[i-1])/p;
  }

  float qn = 0.0, un = 0.0;
  if (vn) {
    qn = 0.5;
    un = 3*(ypn - (y[last] - y[last-1]));
  }

  y2[last] = (un - qn*u[last-1])/(qn*y2[last - 1] + 1.0);

  for (uint32 k = last-1; k != (uint32)-1; k--)
    y2[k] = y2[k]*y2[k+1] + u[k];
}

void rcCubicGeneratePixel::gen2ndD(const rcWindow& src, int32 row,
				   vector<float>& y2,
				   float yp1, float ypn, bool v1, bool vn)
{
  uint32 n = src.width(), last = n - 1;

  if (n != y2.size())
    y2.resize(n);

  vector<float> u(last);
  float qn = 0.0, un = 0.0;

  switch (src.depth())
  {
  case rcPixel8:
    {
      uint8* y = (uint8*)(src.rowPointer(row));

      if (v1) {
	y2[0] = -0.5;
	u[0] = 3.0*((y[1]-y[0]) - yp1);
      }
      else
	y2[0] = u[0] = 0.0;

      for (uint32 i = 1; i < last; i++) {
	float p = 0.5 * y2[i-1] + 2.0;
	y2[i] = -0.5/p;
	float yr = y[i+1], yc = y[i], yl = y[i-1];
	u[i] = (yr - yc) - (yc - yl);
	u[i] = (3*u[i] - 0.5*u[i-1])/p;
      }
      
      if (vn) {
	qn = 0.5;
	float yr = y[last], yl = y[last-1];
	un = 3*(ypn - (yr - yl));
      }
    }
    break;

  case rcPixel16:
    {
      uint16* y = (uint16*)(src.rowPointer(row));

      if (v1) {
	y2[0] = -0.5;
	u[0] = 3.0*((y[1]-y[0]) - yp1);
      }
      else
	y2[0] = u[0] = 0.0;

      for (uint32 i = 1; i < last; i++) {
	float p = 0.5 * y2[i-1] + 2.0;
	y2[i] = -0.5/p;
	float yr = y[i+1], yc = y[i], yl = y[i-1];
	u[i] = (yr - yc) - (yc - yl);
	u[i] = (3*u[i] - 0.5*u[i-1])/p;
      }
      
      if (vn) {
	qn = 0.5;
	float yr = y[last], yl = y[last-1];
	un = 3*(ypn - (yr - yl));
      }
    }
    break;

  case rcPixel32S:
    {
      uint32* y = (uint32*)(src.rowPointer(row));

      if (v1) {
	y2[0] = -0.5;
	u[0] = 3.0*((y[1]-y[0]) - yp1);
      }
      else
	y2[0] = u[0] = 0.0;

      for (uint32 i = 1; i < last; i++) {
	float p = 0.5 * y2[i-1] + 2.0;
	y2[i] = -0.5/p;
	float yr = y[i+1], yc = y[i], yl = y[i-1];
	u[i] = (yr - yc) - (yc - yl);
	u[i] = (3*u[i] - 0.5*u[i-1])/p;
      }
      
      if (vn) {
	qn = 0.5;
	float yr = y[last], yl = y[last-1];
	un = 3*(ypn - (yr - yl));
      }
    }
    break;

  case rcPixelDouble:
    {
      double* y = (double*)(src.rowPointer(row));

      if (v1) {
	y2[0] = -0.5;
	u[0] = 3.0*((y[1]-y[0]) - yp1);
      }
      else
	y2[0] = u[0] = 0.0;

      for (uint32 i = 1; i < last; i++) {
	float p = 0.5 * y2[i-1] + 2.0;
	y2[i] = -0.5/p;
	u[i] = (float)((y[i+1] - y[i]) - (y[i] - y[i-1]));
	u[i] = (3*u[i] - 0.5*u[i-1])/p;
      }
      
      if (vn) {
	qn = 0.5;
	un = 3*(ypn - (float)(y[last] - y[last-1]));
      }
    }
    break;

  default:
    rmAssert(0);
    break;
  }

  y2[last] = (un - qn*u[last-1])/(qn*y2[last - 1] + 1.0);

  for (uint32 k = last-1; k != (uint32)-1; k--)
    y2[k] = y2[k]*y2[k+1] + u[k];
}

double rcCubicGeneratePixel::interpolate(const vector<float>& y,
					 const vector<float>& y2d, float x)
{
  uint32 left = (uint32)x;
  uint32 right = left + 1;

  rmAssert(y.size() == y2d.size());

  float a = (float)right - x;
  float b = 1.0 - a;
  
  double retVal;

  if (b == 0.0) {
    rmAssert(left < y.size());
    retVal = y[left];
  }
  else {
    rmAssert(right < y.size());
    retVal = a*y[left] + b*y[right] +
      ((a*a*a - a)*y2d[left] + (b*b*b - b)*y2d[right])/6.0;
  }

  if (retVal < 0.0)
    return 0.0;

  return (_maxVal != 0.0 && (retVal > _maxVal)) ? _maxVal : retVal;
  
}

float rcCubicGeneratePixel::interpolate(const rcWindow& src, int32 row,
					const vector<float>& y2d, float x)
{
  uint32 left = (uint32)x;
  uint32 right = left + 1;

  rmAssert(src.width() == (int32)y2d.size());

  float retval;

  switch (src.depth())
  {
  case rcPixel8:
  case rcPixel16:
  case rcPixel32S:
    {
      float a = (float)right - x;
      float b = 1.0 - a;

      if (b == 0.0) {
	rmAssert((int32)left < src.width());
	retval = src.getPixel(left, row);
      }
      else {
	if ((int32)right >= src.width()) {
	  printf("row %d x %f left %d right %d w() %d b %f\n",
		 row, x, left, right, src.width(), b); 
	}
	rmAssert((int32)right < src.width());
	float yleft = src.getPixel(left, row);
	float yright = src.getPixel(right, row);

	retval = a*yleft + b*yright + 
	  ((a*a*a - a)*y2d[left] + (b*b*b - b)*y2d[right])/6.0;
      }
    }
    break;

  case rcPixelDouble:
    {
      double a = (double)right - x;
      double b = 1.0 - a;

      if (b == 0.0) {
	rmAssert((int32)left < src.width());
	retval = src.getDoublePixel(left, row);
      }
      else {
	rmAssert((int32)right < src.width());
	double yleft = src.getDoublePixel(left, row);
	double yright = src.getDoublePixel(right, row);
      
	retval = (float)(a*yleft + b*yright + 
			 ((a*a*a - a)*y2d[left] + (b*b*b - b)*y2d[right])/6.0);
      }
    }
    break;

  default:
    rmAssert(0);
    break;
  }

  return retval;
}
