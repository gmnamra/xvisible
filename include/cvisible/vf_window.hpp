#include <cinder/Channel.h>
#include <cinder/Area.h>

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <rc_window.h>
#include <rc_fileutils.h>

using namespace ci;
using namespace std;



class vf_utils
{
  
      static rcSharedFramePtr newFromChannel8u ( const ci::Channel8u& onec )
      {
        return rcSharedFramePtr (new rcFrame (reinterpret_cast<const char*>(onec.getData()), onec.getRowBytes (),
                                              onec.getWidth (), onec.getHeight (), rcPixel8, true));
        
      }

  
  const ci::Channel8u*  newCiChannel (const rcSharedFramePtr& sf)
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


  const ci::Channel8u* rcWindow::new_channel (const rcWindow& w)
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

  

bool ImageExport2JPG (const rcWindow& image, std::string filePathName)
{
    std::string extn = rfGetExtension (filePathName);
    if (extn != std::string("jpg"))
        return false;
    
    Channel8u* chcopy = image.new_channel ();
    ci::writeImage (filePathName, ImageSourceRef( *chcopy ) );
    return rfFileExists (filePathName);

}



};
