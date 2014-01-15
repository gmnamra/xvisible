/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2004/04/28 01:44:31  arman
 *removed template instantiations
 *
 *Revision 1.2  2004/04/27 21:03:31  arman
 *added rcLineSegment
 *
 *Revision 1.1  2004/04/26 17:10:42  arman
 *InfLine Support
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_line.h>

inline rcInfLine rcInfLine::parallel(const rc2Dvector& p) const
{ return rcInfLine(dir(), p); }

inline rcInfLine rcInfLine::normal(const rc2Dvector& p) const
{ return rcInfLine(rc2Dvector(-dir().y(), dir().x()), p); }

inline bool rcInfLine::operator==(const rcInfLine& l) const
{ return mDir == l.mDir && mPos == l.mPos; }

inline bool rcInfLine::operator!=(const rcInfLine& l) const
{ return !(*this == l); }


rcInfLine::rcInfLine (const rc2Dvector& dir, const rc2Dvector& pos)
  : mDir(dir),  mPos(pos)
{
  if (mDir.x() == 0 && mDir.y() == 0)
    throw general_exception ("bad rcInfLine ctor");
}

rcInfLine::rcInfLine (const rcRadian& t, const rc2Dvector& pos)
  : mDir(cos(t),sin(t)),  mPos(pos)
{
}

rcInfLine::rcInfLine (const rc2Dvector& v)
  : mDir(v.unit()), mPos(v)
{
  if (mDir.x() == 0 && mDir.y() == 0)
    throw general_exception ("bad rcInfLine ctor");
}

rcInfLine rcInfLine::transform(const rc2Xform& c) const
{
  return rcInfLine(c.mapVector(dir()).unit(), c.mapPoint(pos()));
}

void rcInfLine::transform(const rc2Xform& c, rcInfLine& result) const
{
  result.dir() = c.mapVector(dir()).unit();
  result.pos() = c.mapPoint(pos());
}

double rcInfLine::toPoint(const rc2Dvector& p) const
{
  return rmABS ((p.x()-pos().x()) * -dir().y() +
	     (p.y()-pos().y()) *  dir().x());
}

rcRadian rcInfLine::angle (const rcInfLine& l) const
{
  return (l.angle() - angle()).normSigned();
}

rc2Dvector rcInfLine::intersect(const rcInfLine& l, bool isPar) const
{
  isPar = false;
  double d = dir().x() * l.dir().y() - dir().y() * l.dir().x();

  isPar = real_equal (d, 0.0);

  if (isPar) return rc2Dvector ();

  double s = ((l.pos().x() - pos().x()) * l.dir().y() -
	      (l.pos().y() - pos().y()) * l.dir().x()) / d;
  return rc2Dvector((dir() * s) + pos());
}

bool rcInfLine::isParallel(const rcInfLine& l) const
{
  double d = dir().x() * l.dir().y() - dir().y() * l.dir().x();
  return rmABS (d) < 1e-12;
}

rc2Dvector rcInfLine::project(const rc2Dvector& p) const
{
  return (dir() * ((p - pos()) * dir())) + pos();
}

double rcInfLine::offset(const rc2Dvector& p) const
{
  return (p - pos()) * dir();
}
