/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.24  2006/01/10 23:40:48  arman
 *removed unneeded functions
 *
 *Revision 1.23  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.22  2005/11/07 23:27:48  arman
 *cell lineage iv and bug fixes
 *
 *Revision 1.21  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.23  2005/06/22 18:55:02  arman
 *added comment
 *
 *Revision 1.22  2005/05/25 01:36:25  arman
 *added ref counting support
 *
 *Revision 1.21  2005/05/16 00:51:05  arman
 *added simple constrain processing
 *
 *Revision 1.20  2005/04/18 02:02:58  arman
 *added similarity based projection
 *
 *Revision 1.19  2005/04/11 00:22:08  arman
 *added affinedImage
 *
 *Revision 1.18  2005/02/17 04:20:51  arman
 *added class specific tiny
 *
 *Revision 1.17  2004/08/20 01:23:47  arman
 *added support for masking and basic image moment calculations
 *
 *Revision 1.16  2004/07/19 20:28:34  arman
 *shape is upto unit test
 *
 *Revision 1.15  2004/04/20 15:36:50  arman
 *corrected setMask api
 *
 *Revision 1.14  2004/04/01 23:45:57  arman
 *added new accessors
 *
 *Revision 1.13  2004/03/31 00:37:39  arman
 *added copy support
 *
 *Revision 1.12  2004/03/30 21:27:21  arman
 *added shape
 *
 *Revision 1.11  2004/03/26 12:49:56  arman
 *added test
 *
 *Revision 1.10  2004/03/25 22:11:20  arman
 *added peak detect
 *
 *Revision 1.9  2004/03/19 17:18:02  arman
 *More Contractile Motion
 *
 *Revision 1.8  2004/03/15 22:01:41  arman
 *incremental
 *
 *Revision 1.7  2004/03/15 06:16:27  arman
 *critical set of features implemented
 *
 *Revision 1.6  2004/03/12 22:17:15  arman
 *incremental
 *
 *Revision 1.5  2004/03/11 04:33:52  arman
 *added new functions.
 *
 *Revision 1.4  2004/03/08 22:12:43  arman
 *added printCache
 *
 *Revision 1.3  2004/03/05 18:00:06  arman
 *incremental
 *
 *Revision 1.2  2004/03/05 04:36:14  arman
 *incremental
 *
 *Revision 1.1  2004/03/04 20:59:13  arman
 *incremental
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_SHAPE_H
#define __RC_SHAPE_H

#include <rc_window.h>
#include <rc_polygon.h>
#include <rc_affinewindow.h>
#include <bitset>
#include <boost/shared_ptr.hpp>

/*!
    @class rcShape
    @abstract Geometric Shape Measurements
    @discussion 
    @throws exception when not initialized
    @throws bar_exception
    @namespace 
    @updated 2003-03-15
*/
class rcShape 
{
 public:

/*! 
  @function Constructor
  @discussion Creates a rcShape from A Polygon or A Polygon and an Image
  @discussion or an image. 
*/
  enum geometry
  {
    eAffine,
    eBoundingBox,
    eImage,
    eDefault = eImage
  };

  enum Valid
    {
      eIsValid = 0,
      eValidFrame,
      eValidPoly,

      eValidArea,
      eValidPerimeter,
      eValidCircularity,
      eValidEllipseRatio,
      eValidMinor,
      eValidMajor,
      eValidMajorProfile,
      eValidMinorProfile,
      eValidCom,
      eValidMoments,
      eValidAffineRect,
      eValidAffineWindow,
      eValidAffineCom,
      eValidAffinedImage,
      eValidDimensionConstraints,
      eNumValids
    };

 bool isValid (enum Valid what) const;
  bool validate (enum Valid what);
  bool inValidate (enum Valid what);

  // Constructors & Setters
  rcShape();
  rcShape (const rcPolygon& poly);
  rcShape (const rcWindow& frame, const rcPolygon& poly);
  rcShape (const rcWindow& frame);

 // Destructor
  virtual ~rcShape();


  //@note: a bounded mask indicates use of mask. An unbounded 
  //           mask (default) indicates use of affine window as a mask
  void setMask ();

  bool isBound (geometry = eAffine);

  rcIRect orthogonalBoundingBox() const;

  /*
    @function
    @discussion Returns the smallest image-coordinate-aligned rectangle that
    contains all pixels of this shape.  The origin of the
    rectangle is the location of this shape in the original
    scene.
  */

  rc2Fvector orthogonalBoundingBoxCenter() const;

  float orthogonalBoundingBoxAspect() const;

  const rcPolygon& shape () const {return mPoly;}


  double area() const;
  /* 
   * Requires: polygon
   * For shapes purely represented by a polygon, this number of pixels
   * inside the polygon (in other words, each pixel counts as 1).
   */

  double mass () const;
  /* Requires: frame
   * For shapes in an intensity image represented by the polygon,
   * this is the sum of all the pixel values inside the polygon.
   *
   */

  double perimeter() const;
  /* returns perimeter of convex hull of the polygon
   *
   */

  rc2Fvector centerOfmass(geometry g = eDefault) const;
  /* returns position of center of mass 
   */

  rc2Fvector median(geometry g = eDefault) const;
  /*
   * Returns position of the median point on the major/minor axis
   * Median point sits between 2 halves of mass
   */

  rcDPair inertia() const;
  /* returns second moments of inertia about vertical and
   * horizontal axes (x() and y() respectively) through center of mass.
   */

  rcDPair inertiaPrincipal() const;
  /* returns second moments of inertia about first and second principal axes
   * x() = moment about first principal axis (minimum inertia)
   * y() = moment about second principal axis (maximum inertia)
   */

  double elongation() const;
  /* returns inertiaPrincipal().y() / inertiaPrincipal().x()
   */

  rcRadian angle() const;
  /* 
   * Currently this is the angle of the affine rectangle.

     orientation of first principal axis; (-PI/2, PI/2] centered on x axis,
     positive defined as +x axis towards +y axis .
     If elongation() is exactly 1, then angle() is undetermined but is
     reported as 0.
     */

  double circularity() const;
  /* returns (perimeter() squared) / (4 * pi * area())
             Not valid for whole-image shape analysis.
     throws  BadMeasure: not valid in whole-image shape mode.
     */

  double ellipseRatio() const;
  /* returns normalized rms deviation of boundary point radius values from r0,
             where r0 is the square root of (area()/pi).
             Not valid for whole-image shape analysis.
  */

  const vector<float>& majorProfile () const;
  const vector<float>& minorProfile () const;
  /*
    @function 
    @discussion Horizontal and vertical projections (affine or bounding)
  */

  const rcAffineRectangle& affineRect () const;
  const rcAffineWindow& affineWindow () const;
  const rcWindow& affineImage () const;

  /*
    @function 
    @discussion AffineRect and AffineWindow corresponding to the Polygon
  */

  float majorDimension () const;
  float minorDimension () const;
  rcDPair majorEnds () const;
  rcDPair minorEnds () const;
  bool majorEndConstraints (rcDPair& oneEnd, rcDPair& otherEnd);
  bool clearMajorEndConstrainsts ();
  bool haveMajorEndConstrainsts () const;
  /*
    @function 
    @discussion Detect ends of the profiles (affinor bounding)
    @           constraints envelope the possible detections, 
    @           measurements outside are disgarded. 
    @           constraints pairs are start, length
    @           oneEnd corresponds to majorEnds.x()
    @           otherEnd corresponds to majorEnds.y()
    @           (only implemented for major dimension)
  */

  int32 extendProfile (int32);
  const int32 extendProfile () const;
  /*
    @function 
    @discussion Extend For Profile Generation
    No check is done to assure that the extension is
    within image
  */

  // required for STL
  bool operator<(const rcShape& rhs) const;
  bool operator==(const rcShape& rhs) const;

  friend ostream& operator<< (ostream&o, const rcShape&s)
  {
    o << "Com:    " << s.centerOfmass (eAffine) << endl;
    o << "Per:    " << s.perimeter () << endl;
    o << "area:   " << s.area () << endl;
    o << "Mass:   " << s.mass () << endl;
    o << "Affine:  " << s.affineRect() << endl;
    o << "Angle:  " << s.angle ().Double() << endl;
    o << "Major:  " << s.majorDimension () << endl;
    o << "Minor:  " << s.minorDimension () << endl;


    cout << "{" << endl;
    uint32 i(0);
    const vector<float>& mp = s.majorProfile();
    for (i = 0; i < mp.size()-1; i++) cout << "{" << i << "," << mp[i] << "},";
    cout << "{" << i << "," << mp[i] << "}};";  
    cout << endl;

    return o;
  }

  double tiny ()
  {
    return 1e-10;
  }

  void   printCache ();

  bool testPeak ();

  void copyFrom (const rcShape&);

private:

  rcShape(const rcShape& rhs);
  rcShape& operator=(const rcShape& rhs);

  bitset<eNumValids> mValidSet;

  double mArea, mMass,  mPerimeter, mElongation, mCircularity, mEllipseRatio;
  rcDPair mInertia;
  rcRadian mAngle;
  rc2Dvector mMedian, mCom, mAffineCom, mAffineMedian; 
  rc2Fvector mAffineSize;
  rcDPair mMajorEnds;
  rcDPair mMinorEnds;
  rcDPair mMajorOneEndConstraints;
  rcDPair mMajorOtherEndConstraints;
  float mMajord, mMinord;
  float mX, mY, mXX, mYY;
  int32 mExtendSize;

  rcWindow mFrame;
  rcWindow mMask;
  rcPolygon mPoly;
//  rcRleWindow mRegion;
  rcAffineRectangle mAffineRect;
  rcAffineWindow mAffineWindow;
  vector<float> mMajorD;
  vector<float> mMinorD;
  rcWindow mAffinedImage;

  // Private workers
  void project (const rcWindow& src, const rcWindow&mask, vector<float>& widthProj, vector<float>& heightProj);
  void projectMI (const rcWindow& src, const rcWindow&mask, vector<float>& widthProj, vector<float>& heightProj);

  float dimension ();
  void profiles ();
  void reset (geometry);
  double needArea() ;
  double needMass () ;
  double needPerimeter() ;
  rcRadian needAngle ();
  rc2Fvector needCenterOfmass(geometry g = eDefault);
  rc2Fvector needMedian(geometry g = eDefault);
  const rcAffineRectangle& needAffineRect ();
  const rcAffineWindow& needAffineWindow ();
  bool peakDetect1d (const vector<float>& signal, rcDPair& extendPeaks);

  //  rcDPair inertia() const;
  //  rcDPair inertiaPrincipal() const;
  //  double elongation() const;
  //  rcRadian angle() const;
  double needCircularity() ;
  double needEllipseRatio() ;
  float needMajorDimension () ;
  float needMinorDimension () ;
  uint8 mBg;
  uint8 mFg;  

};

typedef boost::shared_ptr<rcShape> rcShapeRef;
  
#endif /* __RC_SHAPE_H */
