/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#ifndef __RC_LABELRLE_H
#define __RC_LABELRLE_H

#include <rc_window.h>
#include <rc_polygon.h>

/*! \class rcLabelRLE register.h "analysis/include/register.h"
 *  \brief RLE with an associated runs and a lot less code !!
 *   Runs can have up to 31 bit length and an associated label
 */

class rcLabelRLE
{
public:
  struct Run
  {
    int32 value;
    int32 length;
    int32 label;
  };
  rcLabelRLE(const rcWindow&);
  ~rcLabelRLE();
  Run *pointToRow(int32 y) const {return rat_[y];}
 //! pointToRow takes one arguments and returns Run*
    /*!
      \param y is row coordinate
      \returns Run*
    */

//   rcIRect rectangle() const;
//   /* effect    Returns bounding rectangle of this rle
//    */
  
//   void rectangle(const rcIRect& r)
//   {
//     rmAssert (isBound());
//     rmAssert (r.width() == width() && r.height() == height());
//     mRect = r;
//   }

  int32 numRuns() const {return numRuns_;}

private:

  template<class T> Run* image2runs (const rcWindow&, T*);
  rcWindow image() const;
  void image(rcWindow&) const;

  Run** rat_;
  Run* alloc_;
  int32 numRuns_;
};


#endif /* __RC_LABELRLE_H */
