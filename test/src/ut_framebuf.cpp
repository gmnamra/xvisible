// Copyright 2002 Reify, Inc.

#include "ut_framebuf.h"
#include "rc_window.h"
#include "rc_pixel.hpp"
#include <cvisible/cached_frame_buffer.h>


UT_FrameBuf::UT_FrameBuf()
{
}

UT_FrameBuf::~UT_FrameBuf()
{
        printSuccessMessage( "rcFrame test", mErrors );
}

uint32
UT_FrameBuf::run()
{
    {
        rcUNITTEST_ASSERT(get_bytes().count(rcPixel8) == 1);
        rcUNITTEST_ASSERT(get_bytes().count(rcPixel16) == 2);
        rcUNITTEST_ASSERT(get_bytes().count(rcPixelFloat) == 4); 

            
        
        rcUTCheck(rc_pixel().validate((int) rcPixel8));
        rcUTCheck(rc_pixel().validate((int) rcPixel16));
        rcUTCheck(rc_pixel().validate((int) rcPixel32S));  
        
        rcUTCheck(rc_pixel().validate(get_bytes().count(rcPixel8)));
        rcUTCheck(rc_pixel().validate(get_bytes().count(rcPixel16))); 
        rcUTCheck(rc_pixel().validate(get_bytes().count(rcPixel32S)));       

    }
    
    
    // rcFrameRef basic tests
    {
        // Constructor rcFrameRef( rcFrame* p )
        cached_frame_ref buf( new rcFrame( 640, 480, rcPixel8 ) );
        
        rcUNITTEST_ASSERT( buf->refCount() == 1 );
        
        // Constructor rcFrameRef( const rcFrameRef& p )
        cached_frame_ref share1( buf );
        
        rcUNITTEST_ASSERT( share1.refCount() == 2 );
        {
            cached_frame_ref share2( buf );
            rcUNITTEST_ASSERT( share2.refCount() == 3 );
        }
        rcUNITTEST_ASSERT( share1.refCount() == 2 );
    }

    
     // Dummy object, everything should be undefined
    {        
        rcFrame dummyBuf;

      //  rcUNITTEST_ASSERT( dummyBuf.refCount() == 0 );
     //   rcUNITTEST_ASSERT( dummyBuf.isBound()  == false);
    }

   
    {
        // Test Row Pointers
            testRowPointers( 1, 1, rcPixel8);
            testRowPointers( 3, 3, rcPixel8 );
            testRowPointers( 9, 10, rcPixel8 );  
            testRowPointers( 16, 16, rcPixel8 );  

        testRowPointers( 1, 1, rcPixel16);
        testRowPointers( 3, 3, rcPixel16 );
        testRowPointers( 9, 10, rcPixel16 );  
        testRowPointers( 16, 16, rcPixel16 );  
      
        testRowPointers( 1, 1, rcPixel32S);
        testRowPointers( 3, 3, rcPixel32S );
        testRowPointers( 9, 10, rcPixel32S );  
        testRowPointers( 16, 16, rcPixel32S );  
        
    }
    
    // Small buffers
    {
        // Test all depths
        testFrameBuffer( 1, 1, rcPixel8);
        testFrameBuffer( 3, 3, rcPixel8 );
        testFrameBuffer( 9, 10, rcPixel8 );  
        testFrameBuffer( 16, 16, rcPixel8 );  
        
        testFrameBuffer( 1, 1, rcPixel16);
        testFrameBuffer( 3, 3, rcPixel16 );
        testFrameBuffer( 9, 10, rcPixel16 );  
        testFrameBuffer( 16, 16, rcPixel16 );  
        
        testFrameBuffer( 1, 1, rcPixel32S);
        testFrameBuffer( 3, 3, rcPixel32S );
        testFrameBuffer( 9, 10, rcPixel32S );  
        testFrameBuffer( 16, 16, rcPixel32S );  
        
    }
   
    // Large buffers
    {        
        // Test all depths
        testFrameBuffer( 1600, 1200, rcPixel8 );
        testFrameBuffer( 1152, 864, rcPixel16 );        
        testFrameBuffer( 640, 480, rcPixel32S );        

    }

    // rcFrameRef basic tests   
    {
        // Constructor rcFrameRef( rcFrame* p )
        rcFrameRef buf( new rcFrame( 640, 480, rcPixel8 ) );

        rcUNITTEST_ASSERT( buf->refCount() == 1 );

        // Constructor rcFrameRef( const rcFrameRef& p )
        rcFrameRef share1( buf );

        rcUNITTEST_ASSERT( share1.refCount() == 2 );
        {
            rcFrameRef share2( buf );
            rcUNITTEST_ASSERT( share2.refCount() == 3 );
        }
        rcUNITTEST_ASSERT( share1.refCount() == 2 );
    }

    // rcFrameRef operators
    {
        rcFrame* buf = new rcFrame( 640, 480, rcPixel8 ); 
        rcFrameRef p3 = buf;
        rcUNITTEST_ASSERT( p3.refCount() == 1 );
        rcFrameRef p4 = buf;
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
            rcFrameRef p  = new rcFrame( 640, 480, rcPixel8 ); // sample #1
            rcFrameRef p2 = new rcFrame( 640, 480, rcPixel8 ); // sample #2
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
    
	if ( buf.depth() >= rcPixel8 && buf.depth() <= rcPixel32S) {
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

	if ( buf.depth() >= rcPixel16 && buf.depth() <= rcPixel32S) {
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

	if ( buf.depth() >= rcPixel32S && buf.depth() <= rcPixel32S) {
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

void UT_FrameBuf::testFrameBuffer( int32 width, int32 height, rcPixel depth )
{
    rcFrame* buf = new rcFrame( width, height, depth );

	// Accessor tests
    rcUNITTEST_ASSERT( buf->refCount() == 0 );
    rcUNITTEST_ASSERT( buf->height() == height );
    rcUNITTEST_ASSERT( buf->width() == width );
    rcUNITTEST_ASSERT( buf->depth() == depth );
    rcUNITTEST_ASSERT( buf->rawData() != 0 );
    rcUNITTEST_ASSERT( buf->timestamp() == rcTimestamp() );

  
    int32 alignment = (buf->width() * buf->bytes()) % rcFrame::ROW_ALIGNMENT_MODULUS;
    if ( alignment )
        alignment = rcFrame::ROW_ALIGNMENT_MODULUS - alignment;
    rcUNITTEST_ASSERT( alignment == buf->rowPad() );
    if ( alignment != buf->rowPad() )
        cerr << buf->width() << "x" << buf->height() << "x" << buf->bytes()*8
             << " rowPad " << buf->rowPad() << ", expected " << alignment << endl;

    
	int32 row, column;

	// Pixel mutator tests
	for ( row = 0; row < buf->height(); row++ ) {
		for ( column = 0; column < buf->width(); column++ ) {
			testPixel( column, row, *buf );
		}
	}
    delete buf;
}


void UT_FrameBuf::testRowPointers( int32 width, int32 height, rcPixel depth )
{
    rcFrameRef buf (new rcFrame( width, height, depth ) );

   // Accessor tests
   rcUNITTEST_ASSERT( buf->refCount() == 1 );
   rcUNITTEST_ASSERT( buf->height() == height );
   rcUNITTEST_ASSERT( buf->width() == width );
   rcUNITTEST_ASSERT( buf->depth() == depth );
   rcUNITTEST_ASSERT( buf->rawData() != 0 );


   // RowPointer Tests
   for ( int32 row = 0; row < buf->height(); row++ )
      {
         rcUNITTEST_ASSERT (!((uint64) (buf->rowPointer (row)) % buf->alignment ()));
         if (!row) continue;
         rcUNITTEST_ASSERT ((uint32)(buf->rowPointer (row) - buf->rowPointer (row - 1)) ==
                             buf->rowUpdate ());
      }

}
