/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.9  2004/03/11 05:23:15  arman
 *added mapArea
 *
 *Revision 1.8  2004/03/01 14:33:27  arman
 **** empty log message ***
 *
 *Revision 1.7  2004/02/02 13:50:11  arman
 *added rc2fvector support
 *
 *Revision 1.6  2003/08/18 21:30:35  arman
 *removed unsupported mapArea
 *
 *Revision 1.5  2003/08/18 17:46:32  arman
 *incorporated rcRadian
 *
 *Revision 1.4  2003/03/06 23:10:23  sami
 *Inlining fix
 *
 *Revision 1.3  2003/03/04 18:40:25  arman
 *added more 2X support
 *
 *Revision 1.2  2003/03/04 16:00:35  arman
 *Added 2d xform support
 *
 *Revision 1.1  2002/12/08 23:04:24  arman
 *1D xform for calibration support initially. The class supports moving between
 *image and physical coordinates when the transformation is of form offset + a * scale.
 *Being 1D this could also serve for moving back and forth between frames and time.
 *2D xforms may be needed if calibration is a spatial function.
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcXFORMS_H
#define __rcXFORMS_H


#include <rc_math.h>
#include <rc_matrix.h>

/* To start we use a 1D xform for calibration. It retains
 * many of the same member functions as the higher-dimension xforms,
 * It supports 'scale' and  'offset'.
 */

class rc1Xform
{
public:

  rc1Xform() : mScale(1), mOffset(0) {}
  /*
   * effect Default ctor constructs the identity transform
   */

  rc1Xform(double scale, double offset) : mScale(scale), mOffset(offset) {}

  // default dtor, copy, assignment OK

  double scale () const { return mScale; }
  double offset() const { return mOffset; }
  void scale (double scale)  { mScale  = scale; }
  void offset(double offset) { mOffset = offset; }

  inline rc1Xform inverse() const
  {
      if (isSingular()) throw general_exception ("xform is Singular");
      double inverseScale = 1/mScale;
      return rc1Xform(inverseScale, inverseScale * -mOffset);
  }
  /*
   * effect   return the inverse transformation
   * throws   rcMathError::Singular if the transform is singular.
   */

  inline rc1Xform operator * (const rc1Xform& xform) const
  { return rc1Xform(mScale * xform.mScale, mScale * xform.mOffset + mOffset); }
  rc1Xform compose    (const rc1Xform& xform) const { return (*this * xform); }
  /*
   * effect  Compose methods:  Compose are left to right
   *         so A * B = AB
   */

  double mapPoint   (double pt) const { return (*this * pt); }
  double operator * (double pt) const { return (mScale * pt) + mOffset; }
  /*
   * effect   Maps the point by the xform: result = scale * pt + offset.
   * note     Both mapPoint and '*' map the input like a full 1D-point.
   *          (i.e. a location on the 1-D number line.)
   */

  double invMapPoint (double pt) const { return (inverse() * pt); }
  /*
   * effect   Maps the point by the inverse xform:
   *                   result = 1/scale * (pt - offset)
   * throws   rcMathError::Singular if the transform is singular.
   */

  // The following two map the input like a vector.
  // (i.e. the input is an 'unlocated' length along the 1-D number line.)
  double mapVector (double vect) const { return (mScale * vect); }
  /*
   * effect   Scales the vector: result = scale * vect
   */
  inline double invMapVector (double vect) const
  {
      if (isSingular()) throw general_exception ("xform is Singular");
      return (vect / mScale);
  }
  /*
   * effect   Scales the vector: result = 1/scale * vect
   * throws   rcMathError::Singular if the transform is singular.
   */

  inline bool operator == (const rc1Xform& xform) const
      { return ( (mScale == xform.mScale) && (mOffset == xform.mOffset) ); }

  inline bool operator != (const rc1Xform& xform) const
      { return ( (mScale != xform.mScale) || (mOffset != xform.mOffset) ); }

  // Identity transform
  static const rc1Xform I;

  bool isIdentity() const { return ((mScale == 1) && (mOffset == 0)); }
  /*
   * effect   Returns true if the transform is identity;
   */

  bool isSingular() const {return mScale == 0;}
  /*
   * effect   Returns true if the transform is singular.
   */


private:
  double mScale;
  double mOffset;
};

inline bool real_equal(const rc1Xform& x1, const rc1Xform& x2,
				     double epsilon = 1e-15)
{ return real_equal(x1.scale(),  x2.scale(),  epsilon) &&
         real_equal(x1.offset(), x2.offset(), epsilon);
}

// Minimal 2Dxform for now:

class rc2Xform
{
public:
  rc2Xform() : mT(rc2Dvector()), mC(rcMatrix_2d()) {}
  /*
   * effect Default ctor constructs the identity transform
   */

  rc2Xform(const rcMatrix_2d& c, const rc2Dvector& t) : mT(t), mC(c) {}
  rc2Xform(const rcMatrix_2d& c, const rc2Fvector& t) : mC(c) { trans (t); }

  // default dtor, copy, assignment OK

  const rcMatrix_2d& matrix() const { return mC; }

  const rc2Dvector& trans() const  { return mT; }
  const rc2Fvector transf () const { return rc2Fvector ((float) mT.x(), (float) mT.y()); }

  void matrix(const rcMatrix_2d& c) { mC = c; }
  void trans(const rc2Dvector& t)  { mT = t; }
  void trans(const rc2Fvector& t)  { mT.x((double) t.x());mT.y((double) t.y()); }

  rc2Xform inverse() const
  {
    rcMatrix_2d ic = mC.inverse();
    return rc2Xform(ic, ic * -mT);
  }

  /*
   * effect   return the inverse transformation
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */
  rc2Xform operator * (const rc2Xform& xform) const
  { return rc2Xform(mC * xform.mC, mC * xform.mT + mT); }
  rc2Xform compose (const rc2Xform& xform) const {return *this * xform;}
  /*
   * effect  Compose methods:  Compose are left to right
   *         so A * B = AB
   */
  double xRot() const   {return mC.xRot();}
  double yRot() const   {return mC.yRot();}
  double   xScale() const {return mC.xScale();}
  double   yScale() const {return mC.yScale();}
  /*
   * effect   xRot, yRot, xScale, yScale are one way to dissect a transform.
   * note     Be careful if you mix this decomposition with the one below.
   *          For example, when shear is present, aspect != yScale/xScale.
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */
  double   scale() const    {return mC.scale();}
  double   aspect() const   {return mC.aspect();}
  double shear() const    {return mC.shear();}
  rcRadian rotation() const {return mC.rotation();}
  /*
   * effect  scale, aspect, shear, rotation are another decomposition
   * note     Be careful if you mix this decomposition with the one above.
   *          For example, when shear is present, aspect != yScale/xScale.
   * throws  general_exception ("xform is Singular") if the transform is singular.
   */
  double mapAngle (const double& ang) const
  {
      rc2Dvector v( cos(ang),sin(ang) );
      v = mC * v;
      return ( atan2 (v.y(), v.x()) );
  }
  /*
   * effect   Returns the new angle after mapping by the xform.
   */
  double  invMapAngle (const double& ang) const;
  /*
   * effect   Returns the new angle after mapping by inverse of xform;
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */
  rc2Dvector mapPoint (const rc2Dvector &pt) const
  { return *this * pt; }
  rc2Fvector mapPoint (const rc2Fvector &pt) const
  { return *this * pt; }

  rc2Dvector operator * (const rc2Dvector &pt) const
  { return (mC * pt) + mT; }
  rc2Fvector operator * (const rc2Fvector &pt) const
  {
    rc2Dvector tmp (pt.x(), pt.y());
    tmp = (mC * tmp) + mT;
    return rc2Fvector ((float) tmp.x(), (float) tmp.y());
  }

  /*
   * effect   Maps the point by the xform: result = c * pt + t.
   * note     Both mapPoint and * are mapping the vector like a full 2D-point,
   *          with location as well as length and direction
   */
  rc2Dvector invMapPoint (const rc2Dvector &pt) const
  { return (inverse() * pt); }
  rc2Fvector invMapPoint (const rc2Fvector &pt) const
  {
    rc2Dvector tmp (pt.x(), pt.y());
    tmp = inverse() * tmp;
    return rc2Fvector ((float) tmp.x(), (float) tmp.y());
  }


  /*
   * effect   Maps the point by the inverse xform: result = c^-1 * (pt - t)
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */

  //The following 2 are mapping the vector like a vector, with only length
  //  and orientation
  rc2Dvector mapVector (const rc2Dvector &vect) const
  { return (mC * vect); }
  /*
   * effect   Rotates and scales the vector: result = c * vect
   */
  rc2Dvector invMapVector (const rc2Dvector &vect) const
  { return (mC.inverse() * vect); }


// The area decomposes in to two vectors who cross product it equals.
// Mapping vectors (1,0), and (0,1) in to  the new vector,
// and then take the cross product.
//
// for matrix |a b|, U=(1,0) maps to (a,c) and V=(0,1) maps to (b,d)
//            |c d|
//
// |ad - bc| ==>  |determinant|.
//
  double mapArea (double area) const
  { return (area * rmABS (mC.determinant())); }

// determinant (inverse) == inverse (determinant)
  double invMapArea (double area) const
  {
    if (isSingular())
      throw (general_exception ("Singular"));
    return (area / (rmABS (mC.determinant())));
  }

  /*
   * effect   Rotates and scales the vector: result = c^-1 * vect
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */
  //  double    mapArea (double area) const;
  /*
   * effect   Returns the new area after mapping by the xform.
   * notes    mapArea() computes the new area by multiplying the old area
   *          by the area of a mapped unit square.  The user may wish
   *          to pass in 1.0 as the old area to get the area
   *          conversion constant which then can be applied to map all
   *          the needed areas.
   */
  //  double    invMapArea (double area) const;
  /*
   * effect   Returns the new area after mapping by the inverse.
   * throws   general_exception ("xform is Singular") if the transform is singular.
   */

  bool operator==(const rc2Xform& xform) const
      { return ( (mC == xform.mC) && (mT == xform.mT) ); }

  bool operator != (const rc2Xform& xform) const
      { return !(*this == xform); }
  bool operator <  (const rc2Xform&) const;

  // Identity transform
  static const rc2Xform I;

  bool isIdentity() const { return mT.isNull() && mC.isIdentity(); }
  /*
   * effect   Returns true if the transform is identity;
   */
  bool isSingular() const {return mC.isSingular();}
  /*
   * effect   Returns true if the transform is singular.
   */

  friend ostream& operator<< (ostream& ous,const rc2Xform& xform)
  {
    ous << xform.matrix() << "v" << xform.trans();
    return ous;
  }

private:
  rc2Dvector mT;
  rcMatrix_2d mC;
};

#endif /* __rcXFORMS_H */

