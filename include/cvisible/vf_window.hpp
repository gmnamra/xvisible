#ifndef __VF_UTILS__
#define __VF_UTILS__

#include <cinder/Channel.h>
#include <cinder/Area.h>

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <vfi386_d/rc_window.h>
#include <vfi386_d/rc_fileutils.h>

using namespace ci;
using namespace std;



struct vf_utils
{
    
    static rcSharedFrameBufPtr newFromChannel8u ( ci::Channel8u& onec )
    {
        return rcSharedFrameBufPtr (new rcFrame (reinterpret_cast<char*>(onec.getData()), 
                                                 (int32) onec.getRowBytes (),
                                                 (int32) onec.getWidth (), 
                                                 (int32) onec.getHeight (), rcPixel8, true));
        
    }
    
    
    static ci::Channel8u*  newCiChannel (const rcSharedFrameBufPtr& sf)
    {
        ci::Channel8u* ch8 = new ci::Channel8u ( sf->width(), sf->height () );
        
        const cinder::Area clippedArea = ch8->getBounds();
        int32_t rowBytes = ch8->getRowBytes();
        
        for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
        {
            uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
            const uint8 *srcPtr = sf->rowPointer(y);
            for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
            {
                *dstPtr++ = *srcPtr++;
            }
        }	
        
        return ch8;
    }
    
    
    static const ci::Channel8u* newChannel8uFrom (const rcWindow& w)
    {
        if (!w.isBound()) return 0;
        ci::Channel8u* ch8 = new ci::Channel8u ( w.width(), w.height () );
        
        const cinder::Area clippedArea = ch8->getBounds();
        int32_t rowBytes = ch8->getRowBytes();
        
        for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
        {
            uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
            const uint8 *srcPtr = w.rowPointer(y);
            for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
            {
                *dstPtr++ = *srcPtr++;
            }
        }
        
        return ch8;
    }
    
    
    
    static bool ImageExport2JPG (const rcWindow& image, std::string filePathName)
    {
        std::string extn = rfGetExtension (filePathName);
        if (extn != std::string("jpg"))
            return false;
        
        const ci::Channel8u* chcopy = newChannel8uFrom (image);
        ci::writeImage (filePathName, ImageSourceRef( *chcopy ) );
        return rfFileExists (filePathName);
        
    }
    
    
    
};


#endif

