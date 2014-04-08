/*
 *  rc_qtime.cpp
 *  framebuf
 *
 *  Created by Arman Garakani on Sat May 18 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_qtime.h>
#include <rc_window.h>
#include <rc_ipconvert.h>

#include <algorithm>

    //#define DEBUG_LOG

#define BailErr(x) {err = x; if(err != noErr) goto bail;}


    class rfQuicktimeImporter {
    public:
        GraphicsImportComponent gi;
    };


    std::string _convertPascalStringToString (const StringPtr theString)
    {
        std::string Str (theString[1], theString[0]);
        return Str;
    }


        // Creates a rcWindow from the file whose FSSpec is passed in (OS X)
    static OSErr rfImageFileToRcWindow (const FSSpec * pfsspecImage, rcWindow& rcwin, GraphicsImportComponent& gi);
        // Returns a vector of rcWindows corresponding to the image files
    static OSErr rfImageFileToRcWindowGi (vector <rcWindow>& images, vector <FSSpec*>& files, GraphicsImportComponent& gi );

    static void getVideoMediaImageDesc (Media theMedia, SampleDescriptionHandle anImageDesc);

    typedef const OSTypePtr TypeListPtr;
    typedef NavObjectFilterUPP  QTFrameFileFilterUPP;
    static uint32 countFrames(Movie * mp);

        //
        // Local utilities
        //

        // Comparator for file name sorting
    static bool fileCompare( const std::string& lhs, const std::string& rhs )
    {
        return ( rfImageFrameNum( (char*) lhs.c_str() ) < rfImageFrameNum( (char*) rhs.c_str() ) );
    }

    static OSErr rfImageFileToRcWindowGi(vector <rcWindow>& images, vector <FSSpec*>& files, GraphicsImportComponent& gi)
    {
        OSErr err = noErr;

        int32 n = files.size ();
        if ( ! n )
            return paramErr;

        vector<rcWindow> tmp (n);
        vector<int> indexes;

        for (int32 i = 0; i < n; i++)
        {
            std::string name = _convertPascalStringToString (files[i]->name);
            int num = rfImageFrameNum ((char *) name.c_str());
            assert (num >= 0);
            indexes.push_back (num);
        }

        sort (indexes.begin(), indexes.end());

        for (int32 i = 0; i < n; i++)
        {
            int j;
            std::string name = _convertPascalStringToString (files[i]->name);
            int num = rfImageFrameNum ((char *) name.c_str());
            assert (num >= 0);
            for (j = 0; indexes[j] != num && j < n; j++);

            rfImageFileToRcWindow (files[i], tmp[j], gi);
        }

        for (vector<rcWindow>::iterator tmpi = tmp.begin(); tmpi != tmp.end(); tmpi++)
            images.push_back (*tmpi);

        return err;
    }

        ///// File Support

    static FSSpec MakeFSSpec(CFStringRef path, CFStringRef subdirectory, bool relative = false)
    {
        FSSpec fs_spec;
        memset(&fs_spec, 0 , sizeof(fs_spec));

        CFURLRef url = NULL;

        if (!relative)
            url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
        else
        {
            CFBundleRef bundle_ref = CFBundleGetMainBundle();

            if (!bundle_ref)
            {
                fprintf (stderr, "CFBundleGetMainBundle returned NULL\n");
                return fs_spec;
            }

            url = CFBundleCopyResourceURL(bundle_ref, path, NULL, subdirectory);
        }

        if (!url) {
            return fs_spec;
        }

        FSRef fsRef;

        bool ok = CFURLGetFSRef(url, &fsRef);

        if (!ok) {
            CFRelease(url);
            return fs_spec;
        }

        HFSUniStr255 outName;
        OSStatus err = FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, &outName, &fs_spec, NULL);
        CFRelease(url);

        if (err) {
            fprintf (stderr, "FSGetCatalogInfo error %i\n", int(err));
            fs_spec.name[0] = 0;
        }

        return fs_spec;
    }

        // Get frame number from a file name (ie. "monoframe001.tif")

    int rfImageFrameNum (char * const fileName)
    {
        int i;
        int num = 0;
        char * s = fileName;

            // get to the .
        for(;*s != 0;++s)
            ; // get to the end of the string

        while (s != fileName &&  (*s < '0' || *s > '9'))
            s--;  // find last number

        for (i = 0; s != fileName; s--, i++) {
            const char c = *s;
            if (c >= '0' && c <= '9')
            {
                int j = i;
                int p = 1;
                while (j) p *= 10, j--;
                num += (i == 0) ? (c - '0') : p * (c - '0');
            }
            else if ( c != 0 )
                break; // Only consider the first continuous sequence of numbers
        }


        return num;
    }

        //
        // Public methods
        //

        // Sorts a vector of filenames by their sequence number (ie. "image001.tif, image002.tif")
    void rfImageNameSort( vector <std::string>& names )
    {
            // TODO: filecompare can be slow, we could precompute
            // frame index numbers
        stable_sort( names.begin(), names.end(), fileCompare );
    }

        // Returns a vector of rcWindows corresponding to the image files
    OSErr rfImageFileToRcWindow (vector <rcWindow>& images, vector <std::string>& fileNames )
    {
        rfQuicktimeImporter gi;
        vector<FSSpec*> fileHandles;
        vector<std::string>::iterator file;

        for( file = fileNames.begin(); file < fileNames.end(); ++file )
        {
            bool isvalid = false;
            FSSpec spec = rfMakeFSSpecFromPosixPath( (*file).c_str(), isvalid);
            if ( ValidFSSpec( &spec ) == noErr )
            {
                FSSpec* newSpec = new FSSpec(spec);

                fileHandles.push_back( newSpec );
            }
        }

        OSErr status = rfImageFileToRcWindowGi( images, fileHandles, gi.gi );

            // Delete specs
        for (vector<FSSpec*>::iterator f = fileHandles.begin(); f != fileHandles.end(); f++)
            delete *f;

        return status;
    }

        ////////// Open a Image File and return a rcWindow with its data ////////////////////

    OSErr rfImageFileToRcWindow (const FSSpec* pfsspecImage, rcWindow& rcwin, GraphicsImportComponent& gi)
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

        if (pfsspecImage == NULL) return (bdNamErr);

            // Get the graphics importer
        GetGraphicsImporterForFile(pfsspecImage, &gi);

        result = GraphicsImportGetImageDescription(gi, &imageDescH);

        if( noErr != result || imageDescH == NULL )
        {
            return (bdNamErr);
        }
        desc = *imageDescH;
        getPixelInfoFromImageDesc (imageDescH, pd, pf, isGray, isWhiteReversed, &ctb);

            // Get size and format information we need
        ::Rect boundsRect = { 0, 0, desc->height, desc->width };
        imageWidth = desc->width;
        imageHeight = desc->height;

        PixMapHandle	pixMap = NULL;
            // We will allocate GWorld memory ourselves as a framebuf ===> Dispose will not free this memory
        try
        {
            rcSharedFrameBufPtr buf(new rcFrame (imageWidth, imageHeight, rcPixel (pd)));
            buf->setIsGray(isGray);

                // set a rcWindow to be an entire window in to this
            rcWindow rw (buf);
            tmp = rw;

            assert (pf);

            err = NewGWorldFromPtr( &gWorld, pf, &boundsRect, ctb, NULL, 0, (char *) (tmp.rowPointer(0)), tmp.rowUpdate ());

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


        /////// Movie File Support //////////

        //------------------------------------------------------------------------------
        //	Open up the Movie file and get a Movie from it.
        //------------------------------------------------------------------------------
    OSErr rfMovieFileToRcWindow (vector <rcWindow>& frames, const std::string& fileName )
    {
        FSSpec	theFSSpec;
        short	refnum = 0;
        TimeValue	duration;
        TimeValue	theTime = 0, nextTime = 0;
        int32 frameCount = -1;
        Movie	thisMovie;
        uint32 fc, width, height;
        Media media;
        ::Rect     aMovieRect;
        OSErr err = noErr;
        OSType		whichMediaType = VIDEO_TYPE;
        short		flags = nextTimeMediaSample + nextTimeEdgeOK;

        SampleDescriptionHandle anImageDesc = NULL;

        assert (frames.size() == 0);
        bool isDir = false;
        theFSSpec = rfMakeFSSpecFromPosixPath( fileName.c_str(), isDir );
        err = ValidFSSpec( &theFSSpec );
        if ( err != noErr )
            return err;

            // Initialize QuickTime
        err = EnterMovies();
        assert (err == noErr);
        if ( err != noErr )
            return err;

            // open a Movie file using the FSSpec and create a Movie from that file.
        err = OpenMovieFile(&theFSSpec, &refnum, fsRdPerm);
        assert (err == noErr);
        if ( err != noErr )
            return err;

        NewMovieFromFile(&thisMovie, refnum, NULL, NULL, newMovieActive, NULL);
        err = GetMoviesStickyError ();
        err = GetMoviesError();

        fc = countFrames (&thisMovie); // VideoMediaType
        assert (fc > 0);
        vector<rcWindow> images (fc);

        GetMovieBox(thisMovie, &aMovieRect);
        width = aMovieRect.right - aMovieRect.left;
        height = aMovieRect.bottom - aMovieRect.top;

        media = GetTrackMedia (GetMovieIndTrack (thisMovie, 1));

            // Allocate a handle for sample description
        anImageDesc = (SampleDescriptionHandle)NewHandle(sizeof(Handle));

            // Get the image description for the movie
        getVideoMediaImageDesc (media, anImageDesc);

            // From Image description to rcWindow Depth
        rcPixel dp;
        uint32 pf;
        CTabHandle ctb;
        bool isGray;
        bool isWhiteReversed;

        getPixelInfoFromImageDesc ((ImageDescriptionHandle) anImageDesc, dp, pf, isGray, isWhiteReversed, &ctb);

            // If it is a depth we like, create rcWindows and fill them up.
        assert (dp != 0);

        vector<rcWindow>::iterator img;
        for( img = images.begin(); img < images.end(); img++ )
        {
            frameCount++;
            GetMovieNextInterestingTime(thisMovie,
                                        flags,
                                        1,
                                        &whichMediaType,
                                        theTime,
                                        0,
                                        &nextTime,
                                        &duration);
            if ( theTime < 0 )
                break;

                // Store the frame
            err = getNextVideoSample(thisMovie, *img, media, theTime, true );
            flags = nextTimeMediaSample;
            theTime = nextTime;
        }

        frames = images;

            //bail:
        DisposeMovie (thisMovie);

            //	we're done with the file.
        if (refnum)
            CloseMovieFile(refnum);

        return err;
    }

        // Fill rcFrame color map using QuickTime color table values
    void rfFillColorMap( const CTabHandle ctb, rcSharedFrameBufPtr& frame,
                        bool reverse )
    {
        rmAssert( frame->colorMap() != 0 );
        rmAssert( frame->colorMapSize() > 0 );

        uint16 cMapSize = frame->colorMapSize();
        int16 csize = 0;

        if ( ctb )
            csize = (*ctb)->ctSize + 1; // Element 0 is always there

        if ( csize > 1 ) {
            rmAssert( csize <= cMapSize );

            if ( csize < cMapSize )
                cMapSize = csize;

                // Set color map values
            for (int i = 0; i < cMapSize; i++) {
                uint32 r = (*ctb)->ctTable[i].rgb.red;
                uint32 g = (*ctb)->ctTable[i].rgb.green;
                uint32 b = (*ctb)->ctTable[i].rgb.blue;
                    // Reverse black and white
                if ( reverse ) {
                    r = 65535 - r;
                    g = 65535 - g;
                    b = 65535 - b;
                }
                    // QuickTime uses 16-bit color component values, we must
                    // scale them down to 8-bit component values
                uint32 color = rfRgb( r/256, g/256, b/256 );
                frame->setColor( i, color );
                rmAssert( frame->getColor( i ) == color );
#ifdef DEBUG_LOG_TABLE
                fprintf (stderr, "[%d]: %d\t%d\t%d \t\t%d\t%d\t%d \t\t%u\n",
                         i, r, g, b, r/256, g/256, b/256, color );
#endif
            }
        } else {
                // Set default linear gray scale values
            for (int i = 0; i < cMapSize; i++) {
                int c = cMapSize - i - 1;
                    // Force 8-bit color component value range
                if ( c > 256 )
                    c /= 256;
                uint32 color = rfRgb( c, c, c );
                frame->setColor( i, color );
                rmAssert( frame->getColor( i ) == color );
#ifdef DEBUG_LOG
                fprintf (stderr, "[%d]: %d\t%d\t%d\t\t%u\n",
                         i, c, c, c, color );
#endif
            }
        }
    }

        //
        // Public file utilities
        //
    static bool FSSpecFromPosixPath (CFStringRef posixPath, FSSpec& fileSpec, bool isDirectory);

    FSSpec rfMakeFSSpecFromPosixPath (const char* posixPath, bool isValid)
    {
        FSSpec filespec;
        bool isDirectory;
        CFStringRef posixPathRef = CFStringCreateWithCString ( NULL, posixPath, kCFStringEncodingUTF8);
        bool err = FSSpecFromPosixPath (posixPathRef, filespec, isDirectory);
        isValid = err == true;
        return filespec;
    }

    static bool FSSpecFromPosixPath (CFStringRef posixPath, FSSpec& fileSpec, bool isDirectory)
    {
        FSRef fsRef;

            // create a URL from the posix path:
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                     posixPath,
                                                     kCFURLPOSIXPathStyle,
                                                     isDirectory);

            // check to be sure the URL was created properly:
        if (url == NULL) {
                // printf("Can't get URL");
            return false;
        }

            // use the CF function to extract an FSRef from the URL:
        if (CFURLGetFSRef(url, &fsRef) == NULL){
                // printf("Can't get FSRef.\n");
            CFRelease(url);
            return false;
        }

            // use Carbon call to get the FSSpec from the FSRef
        if (FSGetCatalogInfo (&fsRef,
                              kFSCatInfoNone,
                              NULL /*catalogInfo*/,
                              NULL /*outName*/,
                              &fileSpec,
                              NULL /*parentRef*/) != noErr) {

                //	printf("Can't get FSSpec.\n");
            CFRelease(url);
            return false;
        }

            // We have a valid FSSpec! Clean up and return it:
        CFRelease(url);
        return true;
    }


    OSErr ValidFSSpec(const FSSpec *spec)
    {
        if (! spec->name[0] || (spec->name[0] + (sizeof(FSSpec) - sizeof(spec->name)) > sizeof(FSSpec)))
            return paramErr;

        FInfo fndrInfo;
        return FSpGetFInfo(spec, &fndrInfo);
    }

        //------------------------------------------------------------------------------
        //	Count the number of video "frames" in the Movie by stepping through
        //  all of the video "interesting times", or in other words, the places where the
        //	movie displays a new video sample. The time between these interesting times
        //  is not necessarily constant.
        //------------------------------------------------------------------------------
    static uint32 countFrames(Movie * mp)
    {
        OSType		whichMediaType = VIDEO_TYPE;
        short		flags = nextTimeMediaSample + nextTimeEdgeOK;
        TimeValue	duration;
        TimeValue	theTime = 0;
        int32 frameCount = -1;

        while (theTime >= 0)
        {
            frameCount++;
            GetMovieNextInterestingTime(*mp,
                                        flags,
                                        1,
                                        &whichMediaType,
                                        theTime,
                                        0,
                                        &theTime,
                                        &duration);
                //  after the first interesting time, don't include the time we
                //  are currently at.
            flags = nextTimeMediaSample;
        } // while

        return frameCount;
    }


        // Get the image description for the video media track

    static void getVideoMediaImageDesc (Media theMedia, SampleDescriptionHandle anImageDesc)
    {
        OSErr anErr = noErr;

        OSType mediaType;

        assert(anImageDesc != NULL);
        assert(theMedia != NULL);
        assert(index > 0);

            // Test if we are indeed dealing with video media.
        GetMediaHandlerDescription(theMedia, &mediaType, NULL, NULL);

        if(mediaType != VideoMediaType) return;

        GetMediaSampleDescription(theMedia, 1, anImageDesc);

            // Check to see if there were any errors in the movie
        anErr = GetMoviesError();
        assert(anErr == noErr);

    }

        // From an a quickTime imagedescription we can figure out a few things about the frame in the image:
        // depth: Contains the pixel depth specified for the compressed image:
        // Values of 1, 2, 4, 8, 16, 24, and 32 indicate the depth of color images.
        // Values of 34, 36, and 40 indicate 2-bit, 4-bit, and 8-bit grayscale, respectively, for grayscale images.
        // For us the only acceptable images are 8, 16, 24, and 32 in color and 40 which is 8 bit grey.
        // We detect a gray image through size and equality of color mappings
        // TBD: what is the best way of handling 32 bit images? The issue here is how to treat alpha and also byte ordering.

    void getPixelInfoFromImageDesc (ImageDescriptionHandle anImageDesc, rcPixel& pd, uint32& pixelFormat,
                                    bool& isGray, bool& isWhiteReversed, CTabHandle *ctb)
    {
        rmAssert (ctb);

        int16 aPixelDepth = (*anImageDesc)->depth;
            // Get the color table
        GetImageDescriptionCTable (anImageDesc, ctb);
        int16 csize = (**ctb)->ctSize;

        isGray = false;
        isWhiteReversed = false;

            // Check grayness
        if ( csize > 0 ) {
            isGray = true;
                // Test each color table entry
            for ( int16 i = 0; i < csize + 1; ++i ) {
                const int r = (**ctb)->ctTable[i].rgb.red;
                const int g = (**ctb)->ctTable[i].rgb.green;
                const int b = (**ctb)->ctTable[i].rgb.blue;
                if ( r != g || g != b ) {
                    isGray = false;
                    break;
                }
            }
            if ( isGray ) {
                    // Test whether index 0 contains white
                isWhiteReversed = true;

                for ( int16 i = 1; i < csize + 1; ++i ) {
                    const int prev = (**ctb)->ctTable[i-1].rgb.red;
                    const int cur = (**ctb)->ctTable[i].rgb.red;
                    if ( prev < cur ) {
                        isWhiteReversed = false;
                        break;
                    }
                }
            }
        }

        switch( aPixelDepth ) {
            case 8:
                pd = rcPixel8;
                pixelFormat = k8IndexedPixelFormat;
                break;
            case 16:
                pd = rcPixel32;
                pixelFormat = k32ARGBPixelFormat;
                break;
            case 24:
                pd = rcPixel32;
                pixelFormat = k32ARGBPixelFormat;
                break;
            case 32:
                pd = rcPixel32;
                pixelFormat = k32ARGBPixelFormat;
                break;
            case 40:
                pd = rcPixel8;
                pixelFormat = k8IndexedGrayPixelFormat;
                break;
            default:
                pd = rcPixelUnknown;
                pixelFormat = k32ARGBPixelFormat;
                break;
        }

#ifdef DEBUG_LOG
        fprintf (stderr, "Pixel depth in movie file is %d ==> %d bytes in depth - %s - %s - %s - color table size %i\n",
                 aPixelDepth, pd, isGray ? "is Gray" : "is not Gray",
                 isWhiteReversed ? "0 is white" : "0 is black",
                 pixelFormat == k8IndexedPixelFormat ? "k8IndexedPixelFormat" :
                 (pixelFormat == k8IndexedGrayPixelFormat) ? "k8IndexedGrayPixelFormat" :
                 (pixelFormat == k32ARGBPixelFormat) ? "k32ARGBPixelFormat" : "Other",
                 csize );
#endif            
    }
    
    
    OSErr getNextVideoSample(Movie theMovie, rcWindow& image, Media media, TimeValue fromTimePoint,
                             bool reduceGrayTo8bit)
    {
        OSErr 			anErr = noErr;
        GWorldPtr		gWorld = NULL;
        rcWindow 		tmp, tmp2;
        PixMapHandle	pixMap = NULL;
        CGrafPtr		aSavedPort, moviePort;
        GDHandle		aSavedGDevice, movieGDevice;
        ::Rect			movieRect;
        rcPixel 	pd;
        uint32 		pf;
        SampleDescriptionHandle imageDescH = NULL;
        CTabHandle 	ctb;
        bool      		isGray;
        bool             isWhiteReversed = false;
        
        assert(theMovie != NULL); 
        
            //• Store away current portrect and Gdevice, get pixel sizes and color table for GWorld creation purposes.
        GetGWorld(&aSavedPort, &aSavedGDevice);
        GetMovieGWorld(theMovie, &moviePort, &movieGDevice);
        
            //• Adjust the movie box.
        GetMovieBox(theMovie, &movieRect);
        uint32 width = movieRect.right - movieRect.left;
        uint32 height = movieRect.bottom - movieRect.top;
        ::Rect boundsRect; boundsRect.left = boundsRect.top = 0; boundsRect.bottom =  height; boundsRect.right = width;
            // Get depth, format, and color table infomation for this movie
            // Get the image description for the movie
            // Allocate a handle for sample description
        imageDescH = (SampleDescriptionHandle)NewHandle(sizeof(Handle));   
        getVideoMediaImageDesc (media, imageDescH);
        
        getPixelInfoFromImageDesc ((ImageDescriptionHandle) imageDescH, pd, pf, isGray,
                                   isWhiteReversed, &ctb);
        
            // Create aGWorld for writing the movie in to 
            // We will allocate GWorld memory ourselves as a framebuf ===> Dispose will not free this memory
        try
        {
            rcPixel depth = pd;
            
            rcSharedFrameBufPtr buf(new rcFrame (width, height, depth));
            buf->setIsGray(isGray);
            rcWindow rw (buf);
            
            tmp = rw;
            tmp2 = rw;
            
            assert (pf);
            
            anErr = NewGWorldFromPtr( &gWorld, pf, &boundsRect, ctb, NULL, 0, (char *) (tmp.rowPointer(0)), tmp.rowUpdate ());
            
            if (noErr != anErr)
            {
                goto Closure; // Should throw an exception ????
            }
            
            if ( reduceGrayTo8bit && isGray && depth > rcPixel8 ) {
                depth = rcPixel8;
                rcSharedFrameBufPtr buf8(new rcFrame (width, height, depth));
                buf8->setIsGray(isGray);
                    // Produce color map
                rfFillColorMap( ctb, buf8 );
                rcWindow image8(buf8);
                    // Reduce image depth
                rfRcWindow32to8( tmp, image8 );
                tmp2 = image8;
            } else {
                    // Produce color map if necessary
                if ( depth < rcPixel32 && !isWhiteReversed) {
                    rfFillColorMap( ctb, buf );
                }
            }
        }
        catch (...)
        {
            goto Closure;
        }
        
        pixMap = GetGWorldPixMap(gWorld);
        LockPixels(pixMap);
        
            //• Draw the next first video sample (frame) to GWorld number.
        SetMovieGWorld(theMovie, gWorld, GetGWorldDevice(gWorld));
        SetMovieTimeValue(theMovie, fromTimePoint);
        UpdateMovie(theMovie); MoviesTask(theMovie, 0);
        UnlockPixels(pixMap);
        
        if ( isWhiteReversed && tmp2.depth() == rcPixel8 ) {
                // Color map indicates that pixel value 0 is white
                // Reverse pixels and use default color map
            rfReversePixels8( tmp2 );
        }
        
            // Copy it to the vector
        image = tmp2;
        
        SetGWorld(aSavedPort, aSavedGDevice);
        
            //• Closure. Clean up if we have handles.
        
    Closure:
        if(gWorld != NULL)
            DisposeGWorld(gWorld);
        SetMovieGWorld(theMovie, moviePort, movieGDevice);
        SetGWorld(aSavedPort, aSavedGDevice);
        
        return anErr;
    }
    


