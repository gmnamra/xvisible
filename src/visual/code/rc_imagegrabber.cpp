/*
 *  rc_imagegrabber.cpp
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_qtime.h>
#include <rc_imagegrabber.h>
#include <rc_ipconvert.h>

// #define DEBUG
// #define DEBUG_LOG

//
// Local utilities
//

static OSErr rfImageFileToRcWindow (const FSSpec* pfsspecImage, rcWindow& rcwin, GraphicsImportComponent& gi)
{
   uint32 imageWidth, imageHeight, pf;
   ImageDescriptionHandle imageDescH = NULL;
   ImageDescription *desc;
   rcWindow tmp;
   GWorldPtr gWorld = NULL;
   ComponentResult result;
   QDErr err = noErr;
   rcPixel pd;
   CTabHandle ctb;
   bool isGray;
   bool isWhiteReversed;
   int32 overEightSize (0), fileSize (0);

   if (pfsspecImage == NULL) return (bdNamErr);

   // Get the file size. We use this to uncover quicktime's inability to tell us if it is 8 or 16 bit monochrom
   // @todo move to util or separate function. 

   FSRef filerf;
   OSErr osErr;
   osErr = FSpMakeFSRef(pfsspecImage, &filerf);   
   FSCatalogInfoBitmap whichInfo;
   FSCatalogInfo    catalogInfo;
   whichInfo = kFSCatInfoNodeFlags;
   whichInfo |= kFSCatInfoDataSizes;
   whichInfo |= kFSCatInfoRsrcSizes;
   osErr = FSGetCatalogInfo(&filerf, whichInfo, &catalogInfo, NULL, NULL,NULL);
   fileSize = catalogInfo.dataLogicalSize + catalogInfo.rsrcLogicalSize;

  
   // Get the graphics importer
   GetGraphicsImporterForFile(pfsspecImage, &gi);

   result = GraphicsImportGetImageDescription(gi, &imageDescH);
   
   if( noErr != result || imageDescH == NULL )
   {
     return (bdNamErr);
   }
   desc = *imageDescH;
   getPixelInfoFromImageDesc (imageDescH, pd, pf, isGray, isWhiteReversed, &ctb);
   
#ifdef DEBUG_LOG
   for (int i = 0; i < 256; i++)
      fprintf (stderr, "[%d]: %d\t%d\t%d\n", i, (*ctb)->ctTable[i].rgb.Qt::red, (*ctb)->ctTable[i].rgb.Qt::green, (*ctb)->ctTable[i].rgb.Qt::blue);
#endif
      
   // Get size and format information we need
   Rect boundsRect = { 0, 0, desc->height, desc->width };
   imageWidth = desc->width;
   imageHeight = desc->height;
   overEightSize = imageWidth * imageHeight;
   overEightSize += overEightSize / 10; // add 10 percent

   // Check to see if file size and pixel format / depth match
   
   if (pd == rcPixel8 && pf == k8IndexedGrayPixelFormat && overEightSize < fileSize)
     {
       pd = rcPixel16;
       pf = k16GrayCodecType;
     }

   PixMapHandle	pixMap = NULL;
   // We will allocate GWorld memory ourselves as a framebuf ===> Dispose will not free this memory
   try
      {
         rcSharedFrameBufPtr buf(new rcFrame (imageWidth, imageHeight, rcPixel (pd)));
         buf->setIsGray(isGray);

         if ( pd < rcPixel16 ) {
             rfFillColorMap( ctb, buf );
         }
         // set a rcWindow to be an entire window in to this
         rcWindow rw (buf);
         tmp = rw;
         
         assert (pf);

         err = QTNewGWorldFromPtr( &gWorld, pf, &boundsRect, ctb, NULL, kNativeEndianPixMap, 
			(char *) (tmp.rowPointer(0)), tmp.rowUpdate ());

         if (noErr != err)
         {
            goto bail; // Should throw an exception ????
         }

         pixMap = GetGWorldPixMap(gWorld);
         LockPixels(pixMap);
      }
   catch (...)
   {
      goto bail;
   }


   // Associate this GWorld with the importer
   if( (result = GraphicsImportSetGWorld(gi, gWorld, NULL)) != noErr )
   {
      goto bail;
   }  

   // Import the pixels in to the GWorld. Note that the image is drawn in to the memory that we own.

   if( (result = GraphicsImportDraw(gi)) != noErr )
   {
      goto bail;
   }

   if ( isWhiteReversed && tmp.depth() == rcPixel8 ) {
       // Reverse pixels and use default color map
       rfReversePixels8( tmp );
   }

   //   if ( isWhiteReversed && tmp.depth() == rcPixel16 ) {
   //       // Reverse pixels and use default color map
   //       rfReversePixels16( tmp );
   //   }

   // Copy window out
   rcwin = tmp;

   UnlockPixels(pixMap);
 
   bail:
   DisposeGWorld(gWorld);
   CloseComponent(gi);

   if( imageDescH != NULL)
      DisposeHandle((Handle)imageDescH);

   return err;
}

//
// rcImageGrabber implementation
//

rcImageGrabber::rcImageGrabber( const vector<std::string>& fileNames, rcCarbonLock* cLock, double frameInterval, bool nameSort ) :
        rcFileGrabber( cLock ),
        mFileNames( fileNames ),
        mCurrentIndex( 0 ),
        mFrameInterval( frameInterval ), mCurrentTimeStamp( frameInterval )
{
    lock();
    
    rmAssert( mFileHandles.empty() );
        
    // Sort names just in case
    if (nameSort)
      rfImageNameSort( mFileNames );

#ifdef DEBUG
    // Paranoia, verify sort order
    for (vector<std::string>::iterator name = mFileNames.begin(); name < mFileNames.end()-1; name++ ) {
        char* str1 = (char*)name->c_str();
        char* str2 = (char*)(name+1)->c_str();
        int num1 = rfImageFrameNum( str1 );
        int num2 = rfImageFrameNum( str2 );
        rmAssert( num1 <= num2 );
    }
#endif
    
    for( vector<std::string>::iterator file = mFileNames.begin(); file < mFileNames.end(); ++file )
		{
			bool isdir = false;
			FSSpec spec = rfMakeFSSpecFromPosixPath( file->c_str(), isdir);

        if ( ValidFSSpec( &spec ) == noErr )
        {
            FSSpec* newSpec = new FSSpec( spec );

            mFileHandles.push_back( newSpec );
        }
    }

    if ( mFileHandles.empty() )
        setLastError( eFrameErrorFileInit );

    unlock();
}


rcImageGrabber::~rcImageGrabber()
{
    // Delete specs
    for (vector<FSSpec*>::iterator f = mFileHandles.begin(); f != mFileHandles.end(); f++)
        delete *f;
}
    
// Start grabbing
bool rcImageGrabber::start()
{
    if ( getLastError() == eFrameErrorOK )
        return true;
    else
        return false;
}

// Stop grabbing
bool rcImageGrabber::stop()
{
    lock();
    
    CloseComponent( mImporter );
    
    unlock();
    
    return true;
}
    
// Returns the number of frames available
int32 rcImageGrabber::frameCount()
{
    if ( mFileHandles.size() > 0 )
        return mFileHandles.size();
    else
        return -1;
}

// Get next frame, assign the frame to ptr
rcFrameGrabberStatus rcImageGrabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
{
    lock();
    
    setLastError( eFrameErrorUnknown );
    ptr = NULL;
    rcFrameGrabberStatus ret = eFrameStatusError;
    
    if ( isBlocking ) {
        if ( mCurrentIndex < mFileHandles.size() ) {
            rcWindow image;
            FSSpec *spec = mFileHandles[mCurrentIndex];
            ++mCurrentIndex;

            // Get the frame and convert it from 32 to 8 bit if possible
            OSErr status = rfImageFileToRcWindow( spec, image, mImporter );

            if ( status == noErr ) {
                ptr = image.frameBuf();
                if ( mFrameInterval > 0.0 ) {
                    // Force a fixed frame interval
                    image.frameBuf()->setTimestamp( mCurrentTimeStamp );
                    mCurrentTimeStamp += mFrameInterval;
                }
                ret = eFrameStatusOK;
                setLastError( eFrameErrorOK );
            } else {
                ret = eFrameStatusError;
                // TODO: analyze OSErr and map it to proper eFrameError
                setLastError( eFrameErrorFileRead );
            }
        } else {
            ret = eFrameStatusEOF;
            setLastError( eFrameErrorOK );
        }
    } else {
         // Non-blocking operation not implemented yet
        setLastError( eFrameErrorNotImplemented );
        ret = eFrameStatusError;
    }

    unlock();
    
    return ret;
}

// Get name of input source, ie. file name, camera name etc.
const std::string rcImageGrabber::getInputSourceName()
{
    if ( !mFileNames.empty() ) {
        if ( mCurrentIndex < mFileNames.size() )
            return mFileNames[mCurrentIndex];
        else
            return mFileNames[0];
    }
    // We don't even have a file...
    return "empty file";
}
