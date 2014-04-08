/*
 *
 *$Id $
 *$Log: $
 *
 */

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <rc_window.h>
#include <rc_fileutils.h>

using namespace ci;
using namespace std;



bool rfImageExport2JPG (const rcWindow& image, std::string filePathName)
{
    std::string extn = rfGetExtension (filePathName);
    if (extn != std::string("jpg"))
        return false;
    
    Channel8u* chcopy = image.new_channel ();
    ci::writeImage (filePathName, ImageSourceRef( *chcopy ) );
    return rfFileExists (filePathName);

}


