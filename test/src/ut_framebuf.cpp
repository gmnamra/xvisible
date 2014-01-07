// Copyright 2002 Reify, Inc.

#include "ut_framebuf.h"
#include <rc_window.h>

UT_FrameBuf::UT_FrameBuf()
{
}

UT_FrameBuf::~UT_FrameBuf()
{
        printSuccessMessage( "rcFrame test", mErrors );
}

uint32
UT_FrameBuf::run() {
     // Dummy object, everything should be undefined
    {        
        rcFrame dummyBuf;

        rcUNITTEST_ASSERT( dummyBuf.refCount() == 0 );
        rcUNITTEST_ASSERT( dummyBuf.height() == 0 );
        rcUNITTEST_ASSERT( dummyBuf.width() == 0 );
        rcUNITTEST_ASSERT( dummyBuf.depth() == rcPixelUnknown );
        rcUNITTEST_ASSERT( dummyBuf.rawData() == 0 );
        rcUNITTEST_ASSERT( dummyBuf.timestamp() == rcTimestamp( 0.0 ) );
    }

   
    {
        // Test Row Pointers
        for ( uint32 i = rcPixel8; i <= rcPixel32; i*=2 ) {
            testRowPointers( 1, 1, i );
            testRowPointers( 3, 3, i );
            testRowPointers( 9, 10, i );  
            testRowPointers( 16, 16, i );  
        }
      
    }
    
    // Small buffers
    {
        // Test all depths
        for ( uint32 i = rcPixel8; i <= rcPixel32; i*=2 ) {
            testFrameBuffer( 1, 1, i );
            testFrameBuffer( 3, 3, i );
            testFrameBuffer( 9, 10, i );
            testFrameBuffer( 16, 16, i );
        }
    }
   
    // Large buffers
    {        
        // Test all depths
        for ( uint32 i = rcPixel8; i <= rcPixel32; i*=2 ) {
            testFrameBuffer( 640, 480, i );
            //testFrameBuffer( 600, 800, i );
            //testFrameBuffer( 1024, 768, i );  
            //testFrameBuffer( 1152, 864, i );  
            //testFrameBuffer( 1600, 1200, i );  
        }
    }

    // rcSharedFrameBufPtr basic tests   
    {
        // Constructor rcSharedFrameBufPtr( rcFrame* p )
        rcSharedFrameBufPtr buf( new rcFrame( 640, 480, rcPixel8 ) );

        rcUNITTEST_ASSERT( buf->refCount() == 1 );

        // Constructor rcSharedFrameBufPtr( const rcSharedFrameBufPtr& p )
        rcSharedFrameBufPtr share1( buf );

        rcUNITTEST_ASSERT( share1.refCount() == 2 );
        {
            rcSharedFrameBufPtr share2( buf );
            rcUNITTEST_ASSERT( share2.refCount() == 3 );
        }
        rcUNITTEST_ASSERT( share1.refCount() == 2 );
    }

    // rcSharedFrameBufPtr operators
    {
        rcFrame* buf = new rcFrame( 640, 480, rcPixel8 ); 
        rcSharedFrameBufPtr p3 = buf;
        rcUNITTEST_ASSERT( p3.refCount() == 1 );
        rcSharedFrameBufPtr p4 = buf;
        rcUNITTEST_ASSERT( p3.refCount() == 2 );

        // Comparison operators
        rcUNITTEST_ASSERT( p3 == p4 );
        rcUNITTEST_ASSERT( !(p3 != p4) );
        // Use const_cast to silence compiler warnings
        rcUNITTEST_ASSERT( p3 == const_cast<const rcFrame*>(buf) );
        rcUNITTEST_ASSERT( !(p3 != const_cast<const rcFrame*>(buf)) );
        rcUNITTEST_ASSERT( !(!p3) );

        // Assignment operators
        {
            rcSharedFrameBufPtr p  = new rcFrame( 640, 480, rcPixel8 ); // sample #1
            rcSharedFrameBufPtr p2 = new rcFrame( 640, 480, rcPixel8 ); // sample #2
            p = p2; 
            rcUNITTEST_ASSERT( p.refCount() == 2 );
        }

    }

    return mErrors;
}



void UT_FrameBuf::testPixel( int32 x, int32 y, rcFrame& buf )
{
	uint32 pixelValue = 0;
    rcTimestamp now = rcTimestamp::now();

    // Timestamp mutator
    buf.setTimestamp( now );
    rcUNITTEST_ASSERT( buf.timestamp() == now );
    
	if ( buf.depth() >= rcPixel8 && buf.depth() <= rcPixel32) {
		// Verify that accessors and mutators work
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
       
		pixelValue = 7;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
        if ( buf.getPixel( x, y ) != pixelValue )
            cerr << buf.width() << "x" << buf.height() << "x" << buf.depth()*8
                 << " getPixel(" << x << "," << y << ") returned " <<  buf.getPixel( x, y )
                 << ", expected " << pixelValue << endl;
   
		pixelValue = rcUINT8_MAX;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
        if ( buf.getPixel( x, y ) != pixelValue )
            cerr << buf.width() << "x" << buf.height() << "x" << buf.depth()*8
                 << " getPixel(" << x << "," << y << ") returned " <<  buf.getPixel( x, y )
                 << ", expected " << pixelValue << endl;
   		// Verify that value is returned
		rcUNITTEST_ASSERT( buf.setPixel( x, y, pixelValue-1 ) == (pixelValue - 1) );
	}

	if ( buf.depth() >= rcPixel16 && buf.depth() <= rcPixel32) {
		// Verify that accessors and mutators work
		pixelValue = rcUINT8_MAX + 7;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
         if ( buf.getPixel( x, y ) != pixelValue )
            cerr << buf.width() << "x" << buf.height() << "x" << buf.depth()*8
                 << " getPixel(" << x << "," << y << ") returned " <<  buf.getPixel( x, y )
                 << ", expected " << pixelValue << endl;         
		pixelValue = rcUINT16_MAX;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
        if ( buf.getPixel( x, y ) != pixelValue )
            cerr << buf.width() << "x" << buf.height() << "x" << buf.depth()*8
                 << " getPixel(" << x << "," << y << ") returned " <<  buf.getPixel( x, y )
                 << ", expected " << pixelValue << endl;
 
		// Verify that value is returned
		rcUNITTEST_ASSERT( buf.setPixel( x, y, pixelValue-1 ) == pixelValue - 1);
	}

	if ( buf.depth() >= rcPixel32 && buf.depth() <= rcPixel32) {
		// Verify that accessors and mutators work
		pixelValue = rcUINT24_MAX + 7;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );
		pixelValue = rcUINT32_MAX;
		buf.setPixel( x, y, pixelValue );
		rcUNITTEST_ASSERT( buf.getPixel( x, y ) == pixelValue );

		// Verify that value is returned
		rcUNITTEST_ASSERT( buf.setPixel( x, y, pixelValue-1 ) == pixelValue - 1);
	}
}

void UT_FrameBuf::testFrameBuffer( int32 width, int32 height, uint32 d )
{
    rcPixel depth = rcPixel( d );

    rcFrame* buf = new rcFrame( width, height, depth );

	// Accessor tests
    rcUNITTEST_ASSERT( buf->refCount() == 0 );
    rcUNITTEST_ASSERT( buf->height() == height );
    rcUNITTEST_ASSERT( buf->width() == width );
    rcUNITTEST_ASSERT( buf->depth() == depth );
    rcUNITTEST_ASSERT( buf->rawData() != 0 );
    rcUNITTEST_ASSERT( buf->timestamp() == rcTimestamp( 0.0 ) );

    // Color map tests
    if ( depth < rcPixel32 ) {
        rcUNITTEST_ASSERT( buf->colorMap() != 0 );
        rcUNITTEST_ASSERT( buf->colorMapSize() == uint32( pow( 2., depth*8 ) ) );
    } else {
        rcUNITTEST_ASSERT( buf->colorMap() == 0 );
        rcUNITTEST_ASSERT( buf->colorMapSize() == 0 );
    }
     
    int32 alignment = (buf->width() * buf->depth()) % rcFrame::ROW_ALIGNMENT_MODULUS;
    if ( alignment )
        alignment = rcFrame::ROW_ALIGNMENT_MODULUS - alignment;
    rcUNITTEST_ASSERT( alignment == buf->rowPad() );
    if ( alignment != buf->rowPad() )
        cerr << buf->width() << "x" << buf->height() << "x" << buf->depth()*8
             << " rowPad " << buf->rowPad() << ", expected " << alignment << endl;

    
	int32 row, column;

	// Pixel mutator tests
	for ( row = 0; row < buf->height(); row++ ) {
		for ( column = 0; column < buf->width(); column++ ) {
			testPixel( column, row, *buf );
		}
	}

    if ( depth < rcPixel32 ) {
        // Color map value mutator tests
        for( uint32 i = 0; i < buf->colorMapSize(); ++i ) {
            buf->setColor( i, i );
            rcUNITTEST_ASSERT( buf->getColor( i ) == i );
            buf->setColor( i, buf->colorMapSize() - i );
            rcUNITTEST_ASSERT( buf->getColor( i ) == (buf->colorMapSize() - i) );
        }
        
        // Default gray color map test
        buf->initGrayColorMap();
        uint32 lastColor = 0;
        for( uint32 i = 0; i < buf->colorMapSize(); ++i ) {
            uint32 c = buf->getColor( i );
            uint32 r = rfRed( c );
            uint32 g = rfRed( c );
            uint32 b = rfRed( c );
            rcUNITTEST_ASSERT( r == g );
            rcUNITTEST_ASSERT( g == b );
            rcUNITTEST_ASSERT( c >= lastColor );
            lastColor = r;
        }
        rcUNITTEST_ASSERT( lastColor == 255 );
        if ( lastColor != 255 )
            cerr << buf->width() << "x" << buf->height() << "x" << buf->depth()*8 << " lastColor " << lastColor << endl;

    }
    
    delete buf;
}


void UT_FrameBuf::testRowPointers( int32 width, int32 height, uint32 d )
{
   rcPixel depth = rcPixel( d );

   rcFrame* buf = new rcFrame( width, height, depth );

   // Accessor tests
   rcUNITTEST_ASSERT( buf->refCount() == 0 );
   rcUNITTEST_ASSERT( buf->height() == height );
   rcUNITTEST_ASSERT( buf->width() == width );
   rcUNITTEST_ASSERT( buf->depth() == depth );
   rcUNITTEST_ASSERT( buf->rawData() != 0 );

#if 0
   // RowPointer Tests
   for ( int32 row = 0; row < buf->height(); row++ )
      {
         rcUNITTEST_ASSERT (!((uint32) (buf->rowPointer (row)) % buf->alignment ()));
         if (!row) continue;
         rcUNITTEST_ASSERT ((uint32)(buf->rowPointer (row) - buf->rowPointer (row - 1)) ==
                             buf->rowUpdate ());
      }

#endif   
   delete buf;
}
