/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.4  2005/12/04 19:41:27  arman
 *register unit test plus lattice bug fix
 *
 *Revision 1.3  2005/12/03 00:35:36  arman
 *tiff plus motion compensation
 *
 *Revision 1.2  2005/10/24 02:08:04  arman
 *incremental
 *
 *Revision 1.1  2005/10/22 23:10:40  arman
 *image set registration
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_REGISTER_H
#define __RC_REGISTER_H

#include <vector>
#include <rc_vector2d.h>
#include <rc_pair.h>
#include <rc_macro.h>
#include <rc_window.h>
#include <rc_windowiterator.h>
#include <rc_analysis.h>
#include <rc_histstats.h>

#include <Iterator>

template <class Iterator>
class rcImageSetRegister
{
public:
  enum defaults
    {
      minSize = 3
    };

  enum registerTo
    {
      eFirst = 0,
      eString,
      eProgressive
    };
  // 
  rcImageSetRegister () : mIsValid (false) {}
  rcImageSetRegister (Iterator start, Iterator passedFinish);

 //! setRegister takes three arguments and returns void
    /*!
      \param model an iterator to a an image
      \param range is a pair of integers for search ranges +/- range
      \param a vector of float vectors for peak positions returned
      \return bool indicating success
    */
  void setRegister  (Iterator model, rcIPair range, registerTo option = eFirst );
  void setHistogramCompare (Iterator model, registerTo option);

  const rc2Fvector origin () const { return mOrigin; }
  const rcIPair size () const { return mSize; }
  const vector<rcWindow>& snaps () const
  { return mModels; }

  const vector<float>& sequentialCorrelations () const
  { return mSequentialCorrelations; }

  const vector<float>& sequential1Dmoves () const
  { return mSequential1Dmoves; }

  const vector<rc2Fvector>& sequential2Dmoves () const
  { return mSequential2Dmoves; }

private:
  bool mIsValid;
  Iterator mStart;
  Iterator mEnd;
  int32 mN;
  rcIPair mSize;
  rc2Fvector mOrigin;
  vector<rcWindow> mModels;
  vector<rcHistoStats> mHistos;
  vector<float> mSequentialCorrelations;
  vector<rc2Fvector> mSequential2Dmoves;
  vector<float> mSequential1Dmoves;
};

#include <rc_register.txx>

#endif /* __RC_REGISTER_H */
