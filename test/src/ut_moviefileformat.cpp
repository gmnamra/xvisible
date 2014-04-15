/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   $Id: ut_moviefileformat.cpp 4625 2006-09-07 21:36:39Z armanmg $
 *
 ***************************************************************************/

#include <ut_moviefileformat.h>
#include <rc_window.h>
#include <rc_systeminfo.h>

// public

UT_moviefileformat::UT_moviefileformat()
{
}

UT_moviefileformat::~UT_moviefileformat()
{
  printSuccessMessage( "Movie file format tests", mErrors );
}

uint32 UT_moviefileformat::run()
{
  testId();
  testHeaderRev1();
  testHeaderRev2();
  testExt();
  testTocExt();
  testConvExt();
  testOrgExt();
  testCamExt();
  testExpExt();
    
  return mErrors;
}

// private

// Test base ID header
void UT_moviefileformat::testId()
{
  {
    rcMovieFileIdentifier id;
        
    rcUNITTEST_ASSERT( id.isValid() );
  }
  {
    rcMovieFileIdentifier id( movieFormatInvalid );
        
    rcUNITTEST_ASSERT( !id.isValid() );
  }
  {
    rcMovieFileIdentifier id( movieFormatRev0 );

    rcUNITTEST_ASSERT( id.rev() == movieFormatRev0 );
    rcUNITTEST_ASSERT( id.isValid() );
  }
  {
    rcMovieFileIdentifier id( movieFormatRev1 );

    rcUNITTEST_ASSERT( id.rev() == movieFormatRev1 );
    rcUNITTEST_ASSERT( id.isValid() );
  }
  {
    rcMovieFileIdentifier id( movieFormatRev2 );

    rcUNITTEST_ASSERT( id.rev() == movieFormatRev2 );
    rcUNITTEST_ASSERT( id.isValid() );
  }
  {
    rcMovieFileIdentifier id( movieFormatRevLatest );

    rcUNITTEST_ASSERT( id.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( id.isValid() );
  }
}

// Test rev1 header
void UT_moviefileformat::testHeaderRev1()
{
  {
    rcMovieFileFormat h;
    rcUNITTEST_ASSERT( h.rev() == movieFormatInvalid );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
  }
     
  {
    rcMovieFileFormat h( movieFormatRev0 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev0 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
  }
  {
    rcMovieFileFormat h( movieFormatRev1 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev1 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
  }
  {
    rcMovieFileFormat h( movieFormatRev2 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev2 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
  }
     
  {
    rcMovieFileFormat h( movieFormatRevLatest );
    // Accessors
    rcUNITTEST_ASSERT( h.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    // Mutators
    h.rowUpdate( 64 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 64 );
    h.averageFrameRate( 4.71 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 4.71 );
    h.frameCount( 97 );
    rcUNITTEST_ASSERT( h.frameCount() == 97 );
    h.baseTime( 17 );
    rcUNITTEST_ASSERT( h.baseTime() == 17 );
  }
  {
    uint32 width = 640;
    uint32 height = 480;
        
    rcWindow w( width, height );
    rcMovieFileFormat h( w.frameBuf(), movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.width() == width );
    rcUNITTEST_ASSERT( h.height() == height );
    rcUNITTEST_ASSERT( h.rowUpdate() == width );
    rcUNITTEST_ASSERT( h.bytesInFrame() == width*height );
  }
}

// Test rev2 header
void UT_moviefileformat::testHeaderRev2()
{
  {
    rcMovieFileFormat2 h;
    rcUNITTEST_ASSERT( h.rev() == movieFormatInvalid );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
  }
     
  {
    rcMovieFileFormat2 h( movieFormatRev0 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev0 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
  }
  {
    rcMovieFileFormat2 h( movieFormatRev1 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev1 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
  }
  {
    rcMovieFileFormat2 h( movieFormatRev2 );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRev2 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
  }
  {
    rcMovieFileFormat2 h( movieFormatRevLatest );
    // Accessors
    rcUNITTEST_ASSERT( h.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
  }
  {
    rcMovieFileFormat2 h( movieFormatRevLatest );
    // Accessors
    rcUNITTEST_ASSERT( h.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 0 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 0.0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.bytesInFrame() == 0 );
    rcUNITTEST_ASSERT( h.magic() != 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixel8 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 0 );
    rcUNITTEST_ASSERT( h.bom() == rfPlatformByteOrderMark() );
    // Mutators
    h.rowUpdate( 64 );
    rcUNITTEST_ASSERT( h.rowUpdate() == 64 );
    h.averageFrameRate( 4.71 );
    rcUNITTEST_ASSERT( h.averageFrameRate() == 4.71 );
    h.frameCount( 97 );
    rcUNITTEST_ASSERT( h.frameCount() == 97 );
    h.baseTime( 17 );
    rcUNITTEST_ASSERT( h.baseTime() == 17 );
    h.extensionOffset( 12345 );
    rcUNITTEST_ASSERT( h.extensionOffset() == 12345 );
  }
  {
    uint32 width = 640;
    uint32 height = 480;
        
    rcWindow w( width, height );
    rcMovieFileFormat2 h( w.frameBuf(), movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( h.width() == width );
    rcUNITTEST_ASSERT( h.height() == height );
    rcUNITTEST_ASSERT( h.depth() == w.depth() );
    rcUNITTEST_ASSERT( h.rowUpdate() == width );
    rcUNITTEST_ASSERT( h.bytesInFrame() == width*height );
  }
}

// Test base extension header
void UT_moviefileformat::testExt()
{
  {
    rcMovieFileExt h;
    rcUNITTEST_ASSERT( h.type() == movieExtensionEOF );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionEOF );
    rcUNITTEST_ASSERT( h.type() == movieExtensionEOF );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionTOC );
    rcUNITTEST_ASSERT( h.type() == movieExtensionTOC );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionORG );
    rcUNITTEST_ASSERT( h.type() == movieExtensionORG );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionCNV );
    rcUNITTEST_ASSERT( h.type() == movieExtensionCNV );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionCAM );
    rcUNITTEST_ASSERT( h.type() == movieExtensionCAM );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionEXP );
    rcUNITTEST_ASSERT( h.type() == movieExtensionEXP );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
  }
  {
    rcMovieFileExt h( movieExtensionTOC, 600 );
    rcUNITTEST_ASSERT( h.type() == movieExtensionTOC );
    rcUNITTEST_ASSERT( h.offset() == 600 );
  }
}

// Test TOC extension header
void UT_moviefileformat::testTocExt()
{
  {
    rcMovieFileTocExt h;
    rcUNITTEST_ASSERT( h.type() == movieExtensionTOC );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
    rcUNITTEST_ASSERT( h.count() == 0 );
  }
  {
    rcMovieFileTocExt h( 777 );
    rcUNITTEST_ASSERT( h.type() == movieExtensionTOC );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) + h.count()*sizeof(int64) );
    rcUNITTEST_ASSERT( h.count() == 777 );
  }
    
}

// Test ORG extension header
void UT_moviefileformat::testOrgExt()
{
  {
    rcMovieFileOrgExt h;
    rcUNITTEST_ASSERT( h.type() == movieExtensionORG );
    rcUNITTEST_ASSERT( h.offset() == sizeof( h ) );
    rcUNITTEST_ASSERT( h.origin() == movieOriginUnknown );
    rcUNITTEST_ASSERT( h.baseTime() == 0 );
    rcUNITTEST_ASSERT( h.frameCount() == 0 );
    rcUNITTEST_ASSERT( h.width() == 0 );
    rcUNITTEST_ASSERT( h.height() == 0 );
    rcUNITTEST_ASSERT( h.depth() == rcPixelUnknown );
    rcUNITTEST_ASSERT( h.rev() == movieFormatInvalid );
    rcUNITTEST_ASSERT( h.creatorName() != 0 );
    rcUNITTEST_ASSERT( h.id() == 0 );
  }
  {
    // Test some combinations
    for ( int32 w = 2; w < 8192; w *= 2 ) {
      for ( int32 h = 3; h < 8192; h *= 2 ) {
	for ( int32 org = movieOriginUnknown; org <= movieOriginSynthetic; ++org ) {
	  for ( int32 rev = movieFormatInvalid; rev <= movieFormatRevLatest; ++rev ) {
	    testOrgExt( movieOriginType(org), 11, w*h, w, h,
			rcPixel8, movieFormatRev(rev), "Cthulhu" );
	    testOrgExt( movieOriginType(org), getCurrentTimestamp(), w*h, w, h,
			rcPixel16, movieFormatRev(rev), "Thor" );
	    testOrgExt( movieOriginType(org), getCurrentTimestamp(), w*h, w, h,
			rcPixel32S, movieFormatRev(rev), "Zeus" );
	    testOrgExt( movieOriginType(org), getCurrentTimestamp(), w*h, w, h,
			rcPixel8, movieFormatRev(rev),
			"A somewhat longer name for a creator should not be a problem" );
	  }
	}
      }
    }
  }
}

// Test ORG extension header
void UT_moviefileformat::testOrgExt( movieOriginType o, int64 bt, uint32 fc,
                                     int32 w, int32 h, rcPixel depth,
                                     movieFormatRev r, const char* c )
{
  rcMovieFileOrgExt hd( o, bt, fc, w, h, depth, r, c );
  // Accessors
  rcUNITTEST_ASSERT( hd.offset() == sizeof( hd ) );
  rcUNITTEST_ASSERT( hd.type() == movieExtensionORG );
  rcUNITTEST_ASSERT( hd.origin() == o );
  rcUNITTEST_ASSERT( hd.baseTime() == bt );
  rcUNITTEST_ASSERT( hd.frameCount() == fc );
  rcUNITTEST_ASSERT( hd.width() == w );
  rcUNITTEST_ASSERT( hd.height() == h );
  rcUNITTEST_ASSERT( hd.depth() == depth );
  rcUNITTEST_ASSERT( hd.rev() == r );
  rcUNITTEST_ASSERT( !strcmp(hd.creatorName(), c) );
  rcUNITTEST_ASSERT( hd.id() == 0 );
  // Mutators
  hd.id( w+h );
  rcUNITTEST_ASSERT( hd.id() == w+h );
}

// Test CONV extension header
void UT_moviefileformat::testConvExt()
{
  {
    rcMovieFileConvExt hd;
    rcUNITTEST_ASSERT( hd.type() == movieExtensionCNV );
    rcUNITTEST_ASSERT( hd.offset() == sizeof( hd ) );
    rcUNITTEST_ASSERT( hd.frameCount() == 0 );
    rcUNITTEST_ASSERT( hd.sample() == 1 );
    rcUNITTEST_ASSERT( hd.cropRect() == rcRect() );
    rcUNITTEST_ASSERT( hd.date() > 0 );
    rcUNITTEST_ASSERT( hd.channel() == movieChannelUnknown );
    rcUNITTEST_ASSERT( hd.frameInterval() == 0 );
    rcUNITTEST_ASSERT( hd.rev() == movieFormatRevLatest );
    rcUNITTEST_ASSERT( hd.creatorName() != 0 );
    rcUNITTEST_ASSERT( hd.pixelsReversed() == false );
    rcUNITTEST_ASSERT( hd.id() == 0 );
  }
  {
       
    for ( uint32 so = 0; so < 32; so += 2 ) {
      for ( uint32 fc = so+1; fc < so+10; ++fc ) {
	rcRect r( 0, 0, so, fc );
	for ( uint32 sm = 1; sm < 8; ++sm ) {
	  for ( uint32 ch = movieChannelUnknown; ch <= movieChannelBlue; ++ch ) {
	    for ( float fInt = 0.0033; fInt < 10.0; fInt += 0.666 ) {
	      testConvExt( so, fc, sm, r, movieChannelType(ch), fInt, false, "Tapio" );
	      testConvExt( so, fc, sm, r, movieChannelType(ch), fInt, true, "Ahti" );
	    }
	  }
	}
      }
    }
  }
}

// Test CONV extension header
void UT_moviefileformat::testConvExt( uint32 so, uint32 fc, uint32 sm,
                                      const rcRect& r, movieChannelType ch,
                                      float frameInterval, bool pixelsRev, const char* c )
{
  rcMovieFileConvExt hd( so, fc, sm, r, ch, frameInterval, pixelsRev, c );
    
  rcUNITTEST_ASSERT( hd.type() == movieExtensionCNV );
  rcUNITTEST_ASSERT( hd.offset() == sizeof( hd ) );
  rcUNITTEST_ASSERT( hd.frameCount() == fc );
  rcUNITTEST_ASSERT( hd.sample() == sm );
  rcUNITTEST_ASSERT( hd.cropRect() == r );
  rcUNITTEST_ASSERT( hd.date() > 0 );
  rcUNITTEST_ASSERT( hd.channel() == ch );
  rcUNITTEST_ASSERT( hd.frameInterval() == frameInterval );
  rcUNITTEST_ASSERT( hd.rev() == movieFormatRevLatest );
  rcUNITTEST_ASSERT( !strcmp( hd.creatorName(), c ) );
  rcUNITTEST_ASSERT( hd.pixelsReversed() == pixelsRev );
  rcUNITTEST_ASSERT( hd.id() == 0 );
  // Mutators
  hd.id( fc+sm );
  rcUNITTEST_ASSERT( hd.id() == int32(fc+sm) );
  // cerr << hd << endl;
}

// Test CAM extension header
void UT_moviefileformat::testCamExt()
{
  {
    rcMovieFileCamExt hd;
    rcUNITTEST_ASSERT( hd.type() == movieExtensionCAM );
    rcUNITTEST_ASSERT( hd.offset() == sizeof( hd ) );
    rcUNITTEST_ASSERT( hd.mid() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.uid() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.format() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.mode() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.frameRate() == -1.0 );
    rcUNITTEST_ASSERT( hd.brightness() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.exposure() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.sharpness() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.whiteBalanceUB() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.whiteBalanceVR() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.hue() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.saturation() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.gamma() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.shutter() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.gain() == rcINT32_MAX );
    rcUNITTEST_ASSERT( hd.iris() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.focus() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.temperature() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.zoom() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.pan() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.tilt() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.filter() == rcUINT32_MAX );
    rcUNITTEST_ASSERT( hd.cropRect() == rcRect() );
    rcUNITTEST_ASSERT( hd.id() == 0 );

    uint32 v = 17;
    // Mutators
    hd.mid(++v);
    rcUNITTEST_ASSERT( hd.mid() == v );
    hd.uid(++v);
    rcUNITTEST_ASSERT( hd.uid() == v );
    hd.format(++v);
    rcUNITTEST_ASSERT( hd.format() == v );
    hd.mode(++v);
    rcUNITTEST_ASSERT( hd.mode() == v );
    hd.frameRate( 7.62 );
    rcUNITTEST_ASSERT( hd.frameRate() == 7.62 );
    hd.brightness(++v);
    rcUNITTEST_ASSERT( hd.brightness() == v );
    hd.exposure(++v);
    rcUNITTEST_ASSERT( hd.exposure() == v );
    hd.sharpness(++v);
    rcUNITTEST_ASSERT( hd.sharpness() == v );
    hd.whiteBalanceUB(++v);
    rcUNITTEST_ASSERT( hd.whiteBalanceUB() == v );
    hd.whiteBalanceVR(++v);
    rcUNITTEST_ASSERT( hd.whiteBalanceVR() == v );
    hd.hue(++v);
    rcUNITTEST_ASSERT( hd.hue() == v );
    hd.saturation(++v);
    rcUNITTEST_ASSERT( hd.saturation() == v );
    hd.gamma(++v);
    rcUNITTEST_ASSERT( hd.gamma() == v );
    hd.shutter(++v);
    rcUNITTEST_ASSERT( hd.shutter() == v );
    hd.gain(++v);
    rcUNITTEST_ASSERT( hd.gain() == v );
    hd.iris(++v);
    rcUNITTEST_ASSERT( hd.iris() == v );
    hd.focus(++v);
    rcUNITTEST_ASSERT( hd.focus() == v );
    hd.temperature(++v);
    rcUNITTEST_ASSERT( hd.temperature() == v );
    hd.zoom(++v);
    rcUNITTEST_ASSERT( hd.zoom() == v );
    hd.pan(++v);
    rcUNITTEST_ASSERT( hd.pan() == v );
    hd.tilt(++v);
    rcUNITTEST_ASSERT( hd.tilt() == v );
    hd.filter(++v);
    rcUNITTEST_ASSERT( hd.filter() == v );
    hd.cropRect(rcRect(1,2,101,103));
    rcUNITTEST_ASSERT( hd.cropRect() == rcRect(1,2,101,103) );
    hd.id(++v);
    rcUNITTEST_ASSERT( hd.id() == int32(v) );
    hd.name( "Cretin" );
    rcUNITTEST_ASSERT( !strcmp( hd.name(), "Cretin" ) );
  }
}

// Test EXP extension header
void UT_moviefileformat::testExpExt()
{
  {
    rcMovieFileExpExt hd;
    rcUNITTEST_ASSERT( hd.type() == movieExtensionEXP );
    rcUNITTEST_ASSERT( hd.offset() == sizeof( hd ) );
    rcUNITTEST_ASSERT( hd.id() == 0 );
    rcUNITTEST_ASSERT( hd.lensMag() == -1.0f );
    rcUNITTEST_ASSERT( hd.otherMag() == -1.0f );
    rcUNITTEST_ASSERT( hd.temperature() == -1.0f );
    rcUNITTEST_ASSERT( hd.CO2() == -1.0f );
    rcUNITTEST_ASSERT( hd.O2() == -1.0f );
    rcUNITTEST_ASSERT( strlen(hd.title()) == 0 );
    rcUNITTEST_ASSERT( strlen(hd.userName()) == 0 );
    rcUNITTEST_ASSERT( strlen(hd.treatment1()) == 0 );
    rcUNITTEST_ASSERT( strlen(hd.treatment2()) == 0 );
    rcUNITTEST_ASSERT( strlen(hd.comment()) == 0 );
    rcUNITTEST_ASSERT( !strcmp(hd.cellType(), "Unspecified") );
    rcUNITTEST_ASSERT( !strcmp(hd.imagingMode(), "Unspecified") );

    // Mutators
    float v = 1.11;
    hd.lensMag( v );
    rcUNITTEST_ASSERT( hd.lensMag() == v );
    v += 0.13;
    hd.otherMag( v );
    rcUNITTEST_ASSERT( hd.otherMag() == v );
    v = 38.6;
    hd.temperature( v );
    rcUNITTEST_ASSERT( hd.temperature() == v );
    v = 4.7;
    hd.CO2( v );
    rcUNITTEST_ASSERT( hd.CO2() == v );
    v = 19.0;
    hd.O2( v );
    rcUNITTEST_ASSERT( hd.O2() == v );
    hd.title( "Evil Experiment" );
    rcUNITTEST_ASSERT( !strcmp(hd.title(), "Evil Experiment" ));
    hd.userName( "Dr. Hyde" );
    rcUNITTEST_ASSERT( !strcmp(hd.userName(), "Dr. Hyde" ));
    hd.treatment1( "Acid bath" );
    rcUNITTEST_ASSERT( !strcmp(hd.treatment1(), "Acid bath" ));
    hd.treatment2( "Eye of newt" );
    rcUNITTEST_ASSERT( !strcmp(hd.treatment2(), "Eye of newt" ));
    hd.cellType( "Undead cells" );
    rcUNITTEST_ASSERT( !strcmp(hd.cellType(), "Undead cells" ));
    hd.imagingMode( "Black Light" );
    rcUNITTEST_ASSERT( !strcmp(hd.imagingMode(), "Black Light" ));
    hd.comment( "Eat your heart out, Dr. Frankenstein" );
    rcUNITTEST_ASSERT( !strcmp(hd.comment(), "Eat your heart out, Dr. Frankenstein" ));
  }
}
