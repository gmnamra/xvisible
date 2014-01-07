/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcRATIOPROCESS_H
#define __rcRATIOPROCESS_H

/*! \class rcRatioProcess "analysis/include/rc_ratioprocess.h"
 *  \brief This is a test class.
 *
 *  Ratio Process encapsulates processing dualview and other ratio image pairs. 
 */

#include <rc_register.h>
#include <bitset>

class rcRatioProcess
{
public:

  /*! \fn rcRatioProcess (const rcWindow& dualView, rcIPair& range)
   *  \brief Constructors
   *  \param dualview is an image containing horizontally two spectrally different views of same object
   *  \param left, right are the horizontal views -- assumed at integer registration
   *  \exception std::out_of_range parameter is out of range.
   *  \return a character pointer.
   */ 
  rcRatioProcess (const rcWindow& dualView, const rcIPair& range);
  rcRatioProcess (const rcWindow& dualView);	

  //@enum Defaults
  //
  enum defaults
    {
      eRange = 5
    };
	
  //@enum Haves
  //
  enum Haves
    {
      eRegistration = 0,
      eRatio,
      eValidData,
      eLeftRight,
      eRightLeftRatio,
      eNumHaves 
    };
	
  bool have (enum Haves what) const;
  bool mark (enum Haves what);
  bool clear (enum Haves what);
  void reset ();
	
	
  // compiler generated dtor assignment and copy ok
  // Accessors
  const rc2Fvector& registeration () const;
  const rcWindow& ratioPlane () const;
  const rcWindow& leftRight () const;
  const rcWindow& rightLeftRatio () const;
  const rcWindow& left () const;
  const rcWindow& right () const;

  // Mutators
  bool align ();
  bool ratio ();
  bool graphics (rcVisualSegmentCollection& v);
  bool process ();

private:
  // Info markers
  bitset<eNumHaves> mHaves;
  rcWindow mThinDual; // median / copied copy of Dual. Not valid if dual goes out of scope
  rcWindow mRatio; // per pixel ratio calculation
  rcHistoStats mRatioStats; // Histogram info on ration
  rcWindow mLeftRight;
  rcWindow mRightLeftRatio;
  rcWindow mLeft,mRight;
  rc2Fvector mRegistration;
  rcIPair mSize; // registration area size
  rcIPair mRange; // mis alignment range
  float mR; // peak registration quality
};

#endif /* __rcRATIOPROCESS_H */
