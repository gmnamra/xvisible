/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *  $Id: ut_movieconverter.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 ***************************************************************************/

#ifndef _UT_MOVIEFILECONVERTER_H_
#define _UT_MOVIEFILECONVERTER_H_

#include <rc_unittest.h>
#include <rc_movieconverter.h>

class UT_movieconverter : public rcUnitTest {
  public:

    UT_movieconverter( const char* rfyInputMovie,
                       const char* QTInputMovie );
    ~UT_movieconverter();
    
    virtual uint32 run();
    
  private:
    void testOptions();
    void testOptions( rcMovieConverterOptions& opt );
    void testToRfy();
    void testToQT();
    
    std::string mRfyInputMovie;
    std::string mQTInputMovie;
};

#endif /* _UT_MOVIEFILECONVERTER_H_ */
