
/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_gen_movie_file.cpp
 *   Creation Date  09/10/2003
 *   Author         Peter Roberts
 *
 * Encapsulation of code needed to generate a movie from a series of
 * rcWindows.
 *
 ***************************************************************************/

#include <stdio.h>

#include <rc_gen_movie_file.h>
#include <rc_moviefileformat.h>
#include <rc_ipconvert.h>

rcGenMovieFile::rcGenMovieFile( const std::string& fName ) :
        _fileName( fName ),
        _origin(movieOriginSynthetic), _conversion(rcSelectAverage), _creator("Unknown"),
        _rev(movieFormatRevLatest), _saveStream(0), _lastError( eGenMovieFileErrorOK ),
        _overWrite( false ), _flushed( false ), _frameInterval( -1.0 ),
        _verbose( false ), _inputDepth( rcPixelUnknown )
{
    _lastError = open(_fileName, _overWrite);
}

rcGenMovieFile::rcGenMovieFile( const std::string& fName,
                                const movieOriginType& o,
                                const std::string& c,
                                const movieFormatRev& rev,
                                bool over,
                                const float& fInt ) :
        _fileName( fName ), _origin(o), _conversion(rcSelectAverage), _creator(c),
        _rev(rev), _saveStream(0), _lastError( eGenMovieFileErrorOK ),
        _overWrite( over ), _flushed( false ), _frameInterval( fInt ),
        _verbose( false ), _inputDepth( rcPixelUnknown )
{
    _lastError = open(_fileName, _overWrite);
}

rcGenMovieFile::rcGenMovieFile( const std::string& fName,
                                const rcChannelConversion& conv,
                                const movieFormatRev& rev,
                                bool over,
                                const float& fInt ) :
        _fileName( fName ), _origin(movieOriginUnknown), _conversion(conv), _creator("Unknown"),
        _rev(rev), _saveStream(0), _lastError( eGenMovieFileErrorOK ),
        _overWrite( over ), _flushed( false ), _frameInterval( fInt ),
        _verbose( false ), _inputDepth( rcPixelUnknown )
{
    _lastError = open(_fileName, _overWrite);
}

rcGenMovieFile::rcGenMovieFile( const std::string& fName,
                                const movieOriginType& o,
                                const rcChannelConversion& conv,
                                const std::string& c,
                                const movieFormatRev& rev,
                                bool over,
                                const float& fInt ) :
        _fileName( fName ), _origin(o), _conversion(conv), _creator(c),
        _rev(rev), _saveStream(0), _lastError( eGenMovieFileErrorOK ),
        _overWrite( over ), _flushed( false ), _frameInterval( fInt ),
        _verbose( false ), _inputDepth( rcPixelUnknown )
{
    _lastError = open(_fileName, _overWrite);
}

rcGenMovieFile::~rcGenMovieFile()
{
    flush();
}

rcIPair rcGenMovieFile::frameSize() const
{
  if (_toc.empty())
    return rcIPair(0, 0);
      
  if (_rev <= movieFormatRev1)
    return rcIPair((int32)_movieHdr.width(), (int32)_movieHdr.height());

  if (_rev >= movieFormatRev2)
  return rcIPair((int32)_movieHdr2.width(), (int32)_movieHdr2.height());

  rmAssert( 0 );        // We should never get here
  return rcIPair(0, 0); // Silences compiler warning
}

// Is this instance valid
bool rcGenMovieFile::valid() const
{
    return (_lastError == eGenMovieFileErrorOK);
}

 // Return last error
rcGenMovieFileError rcGenMovieFile::lastError() const
{
    return _lastError;
}


rcGenMovieFileError rcGenMovieFile::addFrame(const rcWindow& frame)
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
    if ( !valid() )
        return _lastError;
    
    if ( _toc.empty() ) {
        _lastError = initHeader( frame );
        if ( !valid() )
            return _lastError;
    } else {
        if ( _rev <= movieFormatRev1 ) {
            // All added frames must be the same size
            rmAssert(int(_movieHdr.width()) == frame.width());
            rmAssert(int(_movieHdr.height()) == frame.height());
        } else if ( _rev >= movieFormatRev2 ) {
            // All added frames must be the same size
            rmAssert(int(_movieHdr2.width()) == frame.width());
            rmAssert(int(_movieHdr2.height()) == frame.height());
        }
       
        rmAssert( _inputDepth == frame.depth() );
    }
    
    int64 timestamp = 0;
    
    if ( _frameInterval > 0.0f ) {
        // Generate timestamps from forced frame rate
        if ( !_toc.empty() )
            timestamp = convertSecondsToTimestamp((rcTimestamp(_toc.back()) + rcTimestamp(_frameInterval)).secs());
    } else
        timestamp = frame.frameBuf()->timestamp()._timestamp;
    
    if ( frame.depth() == rcPixel8 ) {
        // 8-bit, no conversion
        write( frame, timestamp );
    } else if ( frame.depth() == rcPixel32S ) {
        if (  _conversion == rcSelectAll ) {
            // 32-bit, no conversion
            write( frame, timestamp );
        } else {
            // Convert 32-bit to 8-bit
            rcWindow frame8(frame.width(), frame.height());
	    rfImageConvert32to8 (frame, frame8, _conversion);
            rmAssert( rcPixel8 == frame8.depth() );
            frame8.frameBuf()->setIsGray(true);
            frame8.frameBuf()->setTimestamp(rcTimestamp(timestamp));
            write( frame8, timestamp );
        }
    } else if ( frame.depth() == rcPixel16) {
      if (  _conversion == rcSelectAll ) {
	// 16-bit, no conversion
	write( frame, timestamp );
      } else {      
      // @todo merge this and rc_ipconvert{h,cpp} move them to visual
      // Convert 16-bit to 8-bit
            rcWindow frame8(frame.width(), frame.height());
	    rfImageConvert168 (frame, frame8, _conversion);
            rmAssert( rcPixel8 == frame8.depth() );
            frame8.frameBuf()->setIsGray(true);
            frame8.frameBuf()->setTimestamp(rcTimestamp(timestamp));
            write( frame8, timestamp );
        }
    } else {
      // Unsupported depth
      rmAssert( 0 );
    }
    
    _toc.push_back(timestamp);
    
    return eGenMovieFileErrorOK;
}


rcGenMovieFileError rcGenMovieFile::addFrame(const rcWindow& frame, const rcWindow& alpha)
{

  rmAssert(alpha.size() == frame.size());
  rmAssert(int(alpha.depth()) == frame.depth());
  rmAssert (_origin == movieOriginGrayIsVisibleAlphaIsFlu);

  // Combine two frames in to a 32 bit image
  rcWindow newFrame = rfImageConvert8ToARGB (frame, alpha);

  return addFrame (newFrame);
}

// Add conversion header
rcGenMovieFileError rcGenMovieFile::addHeader( const rcMovieFileConvExt& hdr )
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
    
    _cnvHdrs.push_back( hdr );

    return eGenMovieFileErrorOK;
}

// Add origin header
rcGenMovieFileError rcGenMovieFile::addHeader( const rcMovieFileOrgExt& hdr )
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
    
    _orgHdrs.push_back( hdr );

    return eGenMovieFileErrorOK;
}

// Add camera header
rcGenMovieFileError rcGenMovieFile::addHeader( const rcMovieFileCamExt& hdr )
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
    
    _camHdrs.push_back( hdr );

    return eGenMovieFileErrorOK;
}

// Add experiment header
rcGenMovieFileError rcGenMovieFile::addHeader( const rcMovieFileExpExt& hdr )
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
    
    _expHdrs.push_back( hdr );

    return eGenMovieFileErrorOK;
}

// Writes extension headers and closes file
// No more write operations are allowed after this
rcGenMovieFileError rcGenMovieFile::flush()
{
    if ( _flushed )
        return eGenMovieFileErrorOK;
    if ( !_saveStream )
        return eGenMovieFileErrorNull;
    
    _flushed = true;

    if (_toc.size() < 2) {
        if ( fclose(_saveStream)) {
            perror("fclose in flush(0) failed");
            return eGenMovieFileErrorClose;
        }
        return eGenMovieFileErrorNull;
    }
        
    // Update basic header
    int64 firstTime = _toc[0];
    double movieTime = rcTimestamp(_toc[_toc.size()-1] - firstTime).secs();
 
    
    switch ( _rev ) {
        case movieFormatRev0:
            _movieHdr.averageFrameRate((_toc.size()-1)/movieTime);
            _movieHdr.frameCount(_toc.size());
            break;

        case movieFormatRev1:
        {
            _movieHdr.averageFrameRate((_toc.size()-1)/movieTime);
            _movieHdr.frameCount(_toc.size());
            // Store TOC header
            rcMovieFileTocExt tocHdr( _toc.size() );
            if ( tocHdr.write( _saveStream, _toc ) )
                return eGenMovieFileErrorWrite;
            if ( _verbose )
                cerr << tocHdr << endl;
            /* Store EOF header.
             */
            rcMovieFileExt eofHdr(movieExtensionEOF);
            
            if (eofHdr.write(_saveStream))
                return eGenMovieFileErrorWrite;
            if ( _verbose )
                cerr << eofHdr << endl;
        }
        break;

        case movieFormatRev2:
        {
            _movieHdr2.averageFrameRate((_toc.size()-1)/movieTime);
            _movieHdr2.frameCount(_toc.size());
            // Update rev2 header additions
            const off_t movieRecordSz = sizeof(int64) + _movieHdr2.bytesInFrame();
            // Offset to start of headers
            _movieHdr2.extensionOffset( sizeof(_movieHdr2) +
                                        movieRecordSz*_movieHdr2.frameCount());
    
            // Store TOC header
            rcMovieFileTocExt tocHdr( _toc.size() );
            if ( tocHdr.write( _saveStream, _toc ) )
                return eGenMovieFileErrorWrite;
            if ( _verbose )
                cerr << tocHdr << endl;
            std::string creator = _creator;

            // Each rev2 header needs a unique id
            int32 id = 1;
            // Store origin headers
            if ( !_orgHdrs.empty() ) {
                for ( uint32 i = 0; i < _orgHdrs.size(); ++i ) {
                    rcMovieFileOrgExt orgHdr = _orgHdrs[i];
                    orgHdr.id( id++ );
                    if ( orgHdr.write( _saveStream ) ) 
                        return eGenMovieFileErrorWrite;
                    if ( _verbose )
                        cerr << orgHdr << endl;
                }
            } else {
                // User added no headers, produce a default header
                rcMovieFileOrgExt orgHdr( _origin,
                                          _movieHdr2.baseTime(),
                                          _movieHdr2.frameCount(),
                                          _movieHdr2.width(),
                                          _movieHdr2.height(),
                                          _inputDepth,
                                          _rev,
                                          _creator.c_str() );
                orgHdr.id( id++ );
                if ( orgHdr.write( _saveStream ) ) 
                    return eGenMovieFileErrorWrite;
                if ( _verbose )
                    cerr << orgHdr << endl;
            }

            // Store camera headers
            for ( uint32 i = 0; i < _camHdrs.size(); ++i ) {
                rcMovieFileCamExt cam = _camHdrs[i];
                
                cam.id( id++ );
                if ( cam.write( _saveStream ) )
                    return eGenMovieFileErrorWrite;
            }
            
            // Store conversion headers
            for ( uint32 i = 0; i < _cnvHdrs.size(); ++i ) {
                rcMovieFileConvExt cnv = _cnvHdrs[i];
                
                if ( cnv.channel() != movieChannelUnknown ) {
                    cnv.id( id++ );
                    if ( cnv.write( _saveStream ) )
                        return eGenMovieFileErrorWrite;
                }
            }

            // Store experiment headers
            for ( uint32 i = 0; i < _expHdrs.size(); ++i ) {
                rcMovieFileExpExt exp = _expHdrs[i];
                exp.id( id++ );
                if ( exp.write( _saveStream ) )
                    return eGenMovieFileErrorWrite;
            }

            /* Store EOF header.
             */
            rcMovieFileExt eofHdr(movieExtensionEOF);
            
            if (eofHdr.write(_saveStream))
                return eGenMovieFileErrorWrite;
            if ( _verbose )
                cerr << eofHdr << endl;
        }
        break;

        case movieFormatInvalid:
            rmAssert( 0 );
            break;
    }

    if (fseek(_saveStream, 0, SEEK_SET)) {
        perror("fseek in flush failed");
        if (fclose(_saveStream))
            perror("fclose in flush(5) failed");
        return eGenMovieFileErrorSeek;
    }
    if ( _rev <= movieFormatRev1 ) {
        // Write basic header 
        if ( _movieHdr.write( _saveStream ) )
            return eGenMovieFileErrorWrite;
    } else if ( _rev >= movieFormatRev2 ) {
        // Write rev2 header
        if ( _movieHdr2.write( _saveStream ) )
            return eGenMovieFileErrorWrite;
    } else {
        rmAssert( 0 );
    }
      
    if (fclose(_saveStream)) {
        perror("fclose in flush(7) failed");
        return eGenMovieFileErrorClose;
    }
    
    return eGenMovieFileErrorOK;
}

// Open output stream
rcGenMovieFileError
rcGenMovieFile::open(const std::string& fName, bool overwrite)
{
    if (fName.empty())
        return eGenMovieFileErrorOpen;

    if (!overwrite) {
        FILE* temp = fopen(fName.c_str(), "r");
        if (temp) {
            fclose(temp);
            perror("fclose in open(1) failed");
            return eGenMovieFileErrorExists;
        }
    }

    _saveStream = fopen(fName.c_str(), "w+");

    if (_saveStream == NULL) {
        char buf[1024];
        snprintf( buf, rmDim(buf), "fopen in open failed for %s",
                  fName.c_str() );
        perror(buf);
        return eGenMovieFileErrorOpen;
    }

    /* Write out a dummy header indicating the movie is invalid.  Will
     * only be changed to a valid header at the very end when we know
     * the movie is good.
     */
    _movieHdr = rcMovieFileFormat(movieFormatInvalid);
    _movieHdr2 = rcMovieFileFormat2(movieFormatInvalid);

    if ( _rev <= movieFormatRev1 ) {
        // Write basic header 
        if ( _movieHdr.write( _saveStream ) )
            return eGenMovieFileErrorWrite;
    } else if ( _rev >= movieFormatRev2 ) {
        // Write rev2 header
        if ( _movieHdr2.write( _saveStream ) )
            return eGenMovieFileErrorWrite;
    } else {
        rmAssert( 0 );
    }
      
    return eGenMovieFileErrorOK;
}

// Write one frame to stream
rcGenMovieFileError rcGenMovieFile::write( const rcWindow& frame,
                                           const int64& timestamp )
{
    if ( _flushed )
        return eGenMovieFileErrorFlush;
        
    if ( !valid() )
        return _lastError;
  
    const uint32 rowDataLth = frame.width() * get_bytes().count (frame.depth());
    const uint32 rowUpdateLth = (rowDataLth + 15) & 0xFFFFFFF0;
    const uint32 byteCnt = sizeof(int64) + rowUpdateLth*frame.height();

    rcMovieFrameFormat* fPtr = (rcMovieFrameFormat*)malloc(byteCnt);
    rmAssert(fPtr);

    fPtr->timestamp = timestamp;

    for (int32 yo = 0; yo < frame.height(); yo++) {
        uint8* dPtr = (uint8*)&fPtr->rawPixels[yo*rowUpdateLth];
        const uint8* sPtr = (uint8*)frame.rowPointer(yo);
        memcpy(dPtr, sPtr, rowDataLth);
    }
    
    if (fwrite(fPtr, byteCnt, 1, _saveStream) != 1) {
        perror("fwrite of frame in saveToDisk failed");
        if (fclose(_saveStream))
            perror("fclose in saveToDisk(1) failed");
        free (fPtr);
        return eGenMovieFileErrorWrite;
    }

    free(fPtr);
    
    return eGenMovieFileErrorOK;
}

// Initialize movie header with frame information
rcGenMovieFileError rcGenMovieFile::initHeader( const rcWindow& frame )
{
    rmAssert(rcPixel8 == frame.depth() || rcPixel16 == frame.depth() ||
             rcPixel32S == frame.depth());

    uint32 rowDataLth = frame.width();
    uint32 rowUpdateLth = (rowDataLth + 15) & 0xFFFFFFF0;
    
    if ( _conversion == rcSelectAll && frame.depth() == rcPixel32S) {
        rowDataLth = frame.width() * get_bytes().count (frame.depth());
        rowUpdateLth = (rowDataLth + 15) & 0xFFFFFFF0;
    }

    if ( _conversion == rcSelectAll && frame.depth() == rcPixel16) {
      rowDataLth = frame.width() * get_bytes().count (frame.depth());
      rowUpdateLth = (rowDataLth + 15) & 0xFFFFFFF0;
    }
    
    if ( _rev <= movieFormatRev1 ) {
        _movieHdr = rcMovieFileFormat( frame.frameBuf(), _rev );
        _movieHdr.rowUpdate( rowUpdateLth );
        if ( _frameInterval > 0.0f && _origin == movieOriginSynthetic ) {
            // Synthetic frame rate, set base time to current time
            _movieHdr.baseTime( getCurrentTimestamp() );
        }
    } else if ( _rev >= movieFormatRev2 ) {
        _movieHdr2 = rcMovieFileFormat2( frame.frameBuf(), _rev );
        _movieHdr2.rowUpdate( rowUpdateLth );

        if ( _conversion != rcSelectAll ) {
            // Output will be 8-bit
            _movieHdr2.depth( rcPixel8 );
        }
        if ( _frameInterval > 0.0f && _origin == movieOriginSynthetic ) {
            // Synthetic frame rate, set base time to current time
            _movieHdr2.baseTime( getCurrentTimestamp() );
        }
    } 

    _inputDepth = frame.depth();
    
    return eGenMovieFileErrorOK;
}
    
