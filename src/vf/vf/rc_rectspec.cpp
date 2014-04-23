/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2003/12/19 07:00:15  arman
 *added specializations for unsigned trims
 *
 *Revision 1.1  2003/01/11 16:32:48  arman
 *Instantiation of unsigned types and specialization for normalize ()
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "rc_rectangle.h"

// Always instantiated
template class rcRectangle<uint32>;
template class rcRectangle<uint16>;
template class rcRectangle<uint8>;


// Specializations of Unsigned rects
template<>
void rcRectangle<uint32>::normalize()
{
  // No normalization needed for unsigned types
}
template<>
void rcRectangle<uint16>::normalize()
{
  // No normalization needed for unsigned types
}
template<>
void rcRectangle<uint8>::normalize()
{
  // No normalization needed for unsigned types
}

// We check against 1 instead of 0 to cover unsigned types
template<>
rcRectangle<uint32> rcRectangle<uint32>::trim(uint32 left, uint32 right, uint32 top,
	      uint32 bottom, bool throwIfNegativeSize) const
{
  uint32 newWidth = width() - (left + right);
  uint32 newHeight = height() - (top + bottom);
  if (newWidth < 1 || newHeight < 1)
      if (throwIfNegativeSize)
	throw general_exception("rcRectErrors::NegativeSize");

  return rcRectangle<uint32>(mUpperLeft + rcPair<uint32>(left, top),
                   mLowerRight - rcPair<uint32>(right, bottom));
}

// We check against 1 instead of 0 to cover unsigned types
template<>
rcRectangle<uint16> rcRectangle<uint16>::trim(uint16 left, uint16 right, uint16 top,
	      uint16 bottom, bool throwIfNegativeSize) const
{
  uint16 newWidth = width() - (left + right);
  uint16 newHeight = height() - (top + bottom);
  if (newWidth < 1 || newHeight < 1)
      if (throwIfNegativeSize)
	throw general_exception("rcRectErrors::NegativeSize");

  return rcRectangle<uint16>(mUpperLeft + rcPair<uint16>(left, top),
                   mLowerRight - rcPair<uint16>(right, bottom));
}

// We check against 1 instead of 0 to cover unsigned types
template<>
rcRectangle<uint8> rcRectangle<uint8>::trim(uint8 left, uint8 right, uint8 top,
	      uint8 bottom, bool throwIfNegativeSize) const
{
  uint8 newWidth = width() - (left + right);
  uint8 newHeight = height() - (top + bottom);
  if (newWidth < 1 || newHeight < 1)
      if (throwIfNegativeSize)
	throw general_exception("rcRectErrors::NegativeSize");

  return rcRectangle<uint8>(mUpperLeft + rcPair<uint8>(left, top),
                   mLowerRight - rcPair<uint8>(right, bottom));
}

