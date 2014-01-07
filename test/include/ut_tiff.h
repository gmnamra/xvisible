/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/10/25 22:38:50  arman
 *in progress
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_TIFF_H
#define __UT_TIFF_H

#include <rc_unittest.h>
#include <rc_tiff.h>

class UT_tiff : public rcUnitTest {
public:
  UT_tiff(std::string& basename);
  ~UT_tiff();

  virtual uint32 run();
	uint32 testFile (std::string& file, uint32 num);
private:
	vector<std::string> mTiffFiles;
  vector<uint32> mNumFrames;
  std::string mBasename;
};


#endif /* __UT_TIFF_H */
