/******************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *  rc_draw.cpp
 *
 *  Routines for drawing into rcWindow objects.
 *
 *****************************************************************************/

#include <rc_draw.h>
#include <rc_fill.h>

void rcDrawPolygon(const rcPolygon& p, rcWindow& dest, uint8 color, bool fill)
{
  rmAssert(dest.contains(p.orthogonalEnclosingRect()));

  const int32 count = p.vertexCnt();

  if (count == 0)
    return;

  if (count == 1) {
    rc2Dvector p0 = p.vertexAt(0);

    dest.setPixel((int32)(p0.x() + 0.5), (int32)(p0.y() + 0.5), color);
    return;
  }

  for (int32 i = 0; i < count; i++)
    rcDrawLine(p.vertexAt(i), p.vertexAt(i+1), dest, color, fill);
}

void rcFillPolygon(const rcPolygon& p, rcWindow& dest, uint8 color)
{
  /* Note: Convert existing points over to doubles so they can be used
   * as arguments to an existing fill routine. At the very least, the
   * fill routine should be converted over to directly use rcPolygons.
   */
  rmAssert(dest.contains(p.orthogonalEnclosingRect()));

  const int32 count = p.vertexCnt();

  if (count == 0)
    return;

  if (count == 1) {
    rc2Dvector p0 = p.vertexAt(0);

    dest.setPixel((int32)(p0.x() + 0.5), (int32)(p0.y() + 0.5), color);
    return;
  }

  if (count == 2) {
    rcDrawLine(p.vertexAt(0), p.vertexAt(1), dest, color, true);
    return;
  }

  double* coords = (double*)malloc(count*2*sizeof(double));
  rmAssert(coords);
  double* cp = coords;

  for (int32 vi = 0; vi < count; vi++) {
    const rc2Dvector& vert = p.vertexAt(vi);
    *cp++ = vert.x(); *cp++ = vert.y();
  }

  rc_fill_poly(dest, count, coords, color);
  free(coords);
}

void rcDrawAffineRectangle(const rcAffineRectangle& ar, rcWindow& dest,
			   uint8 color, bool fill)
{
  const rc2Dvector ul = ar.affineToImage(rc2Dvector(0, 0));
  const rc2Dvector ur = ar.affineToImage(rc2Dvector(1, 0));
  const rc2Dvector ll = ar.affineToImage(rc2Dvector(0, 1));
  const rc2Dvector lr = ar.affineToImage(rc2Dvector(1, 1));
  rcDrawLine(ul, ur, dest, color, fill);
  rcDrawLine(ur, lr, dest, color, fill);
  rcDrawLine(lr, ll, dest, color, fill);
  rcDrawLine(ll, ul, dest, color, fill);
}

void rcDrawIRect(const rcIRect& ir, rcWindow& dest, uint8 color, bool fill)
{
  const rc2Dvector ul = rc2Dvector(ir.ul().x(), ir.ul().y());
  const rc2Dvector ur = rc2Dvector(ir.ur().x(), ir.ur().y());
  const rc2Dvector ll = rc2Dvector(ir.ll().x(), ir.ll().y());
  const rc2Dvector lr = rc2Dvector(ir.lr().x(), ir.lr().y());
  rcDrawLine(ul, ur, dest, color, fill);
  rcDrawLine(ur, lr, dest, color, fill);
  rcDrawLine(lr, ll, dest, color, fill);
  rcDrawLine(ll, ul, dest, color, fill);
}

void binarizeFloatWindow(const rcWindow& src, rcWindow& dest, const float t,
			 const uint8 fg, const uint8 bg)
{
  rmAssert(src.depth() == rcPixel32);
  rmAssert(dest.depth() == rcPixel8);
  const int32 width =
    (src.width() < dest.width()) ? src.width() : dest.width();
  const int32 height =
    (src.height() < dest.height()) ? src.height() : dest.height();
  const int32 loopCnt = width/16;
  const int32 loopRem = width & 0xF;

  for (int32 y = 0; y < height; y++) {
    const float* srcP = (float*)src.rowPointer(y);
    uint8* destP = (uint8*)dest.rowPointer(y);

    for (int32 x = 0; x < loopCnt; x++) {
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
      *destP++ = (*srcP++ <= t) ? bg : fg;
    }

    for (int32 x = 0; x < loopRem; x++) {
      *destP++ = (*srcP++ <= t) ? bg : fg;
    }
  }
}
