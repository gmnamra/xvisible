/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/09/09 16:38:43  arman
 *moved here
 *
 *Revision 1.1  2005/07/27 23:42:33  arman
 *moved to the include dir
 *
 *Revision 1.7  2005/07/21 22:09:39  arman
 *incremental
 *
 *Revision 1.6  2005/03/25 22:04:55  arman
 *added tangent field
 *
 *Revision 1.5  2003/12/05 20:38:11  arman
 *added cone
 *
 *Revision 1.4  2003/11/15 23:15:27  arman
 *added laplacian of Gaussian
 *
 *Revision 1.3  2003/11/15 22:22:04  arman
 *added difference of gaussian
 *
 *Revision 1.2  2003/11/14 22:02:46  arman
 *added include
 *
 *Revision 1.1  2003/11/14 19:38:26  arman
 *Mathematically generated images
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_MATHMODEL_H
#define __RC_MATHMODEL_H

#include <rc_window.h>

//// OptoKinetic Model of Biological Motion ///////////
//// Gabor = Gaussian * Sine  
///////////////////////////////////////////////////////
class rcMathematicalImage
{
public:

  enum distribution
    {
      eGauss,
      eGabor,
      eDoGauss,
      eLoGauss,
      eCone,
      eFrustum
    };

  enum polarity
    {
      eHiPeak,
      eLowPeak,
      eField
    };
      

  rcMathematicalImage (rcRadian o, rcRadian p, double f, double a, double std)
    :mOrientation (o), mPhase (p), mFrequency (f), mAmplitude (a), mStd (std), mRatio (0.0)
  {}

  rcMathematicalImage (double std = 1.0)
    :mOrientation (rcRadian (0.0)), mPhase (rcRadian (0.0)), mFrequency (1.0), mAmplitude (1.0), mStd (std), mRatio (0.0)
  {}

  rcMathematicalImage (double std, double ratio)
    :mOrientation (rcRadian (0.0)), mPhase (rcRadian (0.0)), mFrequency (1.0), mAmplitude (1.0), mStd (std), mRatio (ratio)
  {}

  void gauss (rcWindow& i, polarity p = eLowPeak)
  {
    mMakeImage (i, eGauss, p);
  }

  void gabor (rcWindow& i, polarity p = eLowPeak)
  {
    mMakeImage (i, eGabor, p);
  }

  void dOg (rcWindow& i, polarity p = eLowPeak)
  {
    mMakeImage (i, eDoGauss, p);
  }

  void lOg (rcWindow& i, polarity p = eLowPeak)
  {
    mMakeImage (i, eLoGauss, p);
  }

  void cone (rcWindow& i, polarity p = eLowPeak)
  {
    mMakeImage (i, eCone, p);
  }

  void frustum (rcWindow& i, float dr = 0.0f)
  {
    mMakeField (i, eFrustum, dr);
  }

private:
  rcRadian mOrientation;
  rcRadian mPhase;
  double mFrequency;
  double mAmplitude;
  double mStd;
  double mRatio;
  void mMakeImage (rcWindow&, distribution, polarity);
  void mMakeField (rcWindow&, distribution, float dr = 0.0f);
};



#endif /* __RC_MATHMODEL_H */
