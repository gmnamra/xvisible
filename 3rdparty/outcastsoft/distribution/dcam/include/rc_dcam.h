/*
 *
 * $Id: rc_dcam.h 4420 2006-05-15 18:48:40Z armanmg $
 *
 * This file contains ASC DCAM support code.
 *
 * Copyright (c) 2003-2004 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcDCAM_H_
#define _rcDCAM_H_

#include <iostream>
#include <vector>
#include <ASC_DCAM_DEV/ASC_DCAM_API.h>

#include <rc_types.h>
#include <rc_framebuf.h>
#include <rc_vector2d.h>

using namespace std;
class rcMovieFileCamExt;

// Firewire camera color format
enum rcCameraColorFormat {
    rcCameraColorFormatUnknown = 0,
    rcCameraColorFormatYUV444,
    rcCameraColorFormatYUV422,
    rcCameraColorFormatYUV411,
    rcCameraColorFormatRGB24,
    rcCameraColorFormatGray8,
    rcCameraColorFormatGray16
};

// Camera format/mode combination class

class rcCameraFormat
{
  public:
    rcCameraFormat() : mWidth(0), mHeight(0), mFormat(255), mMode(255), mColor(rcCameraColorFormatUnknown)
	{};
    rcCameraFormat( uint16 width,
                    uint16 height,
                    uint8  format,
                    uint8  mode,
                    rcCameraColorFormat color ) : mWidth(width), mHeight(height),
        mFormat( format ), mMode( mode ), mColor(color) {};
    // Accessors
    const uint16& width() const { return mWidth; }
    const uint16& height() const { return mHeight; }
    const uint8&  format() const { return mFormat; }
    const uint8&  mode() const { return mMode; }
    const rcCameraColorFormat& color() const { return mColor; }

    rcPixel depth() const {
        switch ( mColor ) {
            case rcCameraColorFormatYUV444:
            case rcCameraColorFormatYUV422:
            case rcCameraColorFormatYUV411:
            case rcCameraColorFormatRGB24:
                return rcPixel32;
            case rcCameraColorFormatGray8:
                return rcPixel8;
            case rcCameraColorFormatGray16:
                return rcPixel16;
            default:
                return rcPixelUnknown;
        }
    }
    
    bool isMono() const {
         switch ( mColor ) {
            case rcCameraColorFormatGray8:
            case rcCameraColorFormatGray16:
                return true;
            default:
                return false;
        }
    }

    bool isMono8 () const {
         switch ( mColor ) {
            case rcCameraColorFormatGray8:
                return true;
            default:
                return false;
        }
    }
    
  private:
    uint16            mWidth;
    uint16            mHeight;
    uint8             mFormat;
    uint8             mMode;
    rcCameraColorFormat mColor;
};

// Camera (control) property class

class rcCameraProperty {
  public:
    rcCameraProperty() : mType(UInt32(-1)), mCntl(UInt32(-1)),
        mMinValue(UInt32(-1)), mMaxValue(UInt32(-1)), mCurValue(UInt32(-1)),
        mCanOnePush(0), mCanReadOut(0), mCanOnOff(0), mCanAuto(0), mCanManual(0), mActive(0) {
    };
    rcCameraProperty( UInt32 type, UInt32 cntl ) : mType(type), mCntl( cntl),
        mMinValue(UInt32(-1)), mMaxValue(UInt32(-1)), mCurValue(UInt32(-1)), 
        mCanOnePush(0), mCanReadOut(0), mCanOnOff(0), mCanAuto(0), mCanManual(0), mActive(0) {
    };

    // Accessors
    const UInt32& type() const { return mType; }
    const UInt32& cntl() const { return mCntl; }
    const UInt32& minValue() const { return mMinValue; }
    const UInt32& maxValue() const { return mMaxValue; }
    const UInt32& curValue() const { return mCurValue; }

    const Boolean& canOnePush() const { return mCanOnePush; }
    const Boolean& canReadOut() const { return mCanReadOut; }
    const Boolean& canOnOff() const { return mCanOnOff; }
    const Boolean& canAuto() const { return mCanAuto; }
    const Boolean& canManual() const { return mCanManual; }
    const Boolean& active() const    { return mActive; }
 
    // Mutators
    UInt32& type()      { return mType; }
    UInt32& cntl()      { return mCntl; }
    UInt32& minValue()  { return mMinValue; }
    UInt32& maxValue()  { return mMaxValue; }
    UInt32& curValue()  { return mCurValue; }

    Boolean& canOnePush()  { return mCanOnePush; }
    Boolean& canReadOut()  { return mCanReadOut; }
    Boolean& canOnOff()    { return mCanOnOff; }
    Boolean& canAuto()     { return mCanAuto; }
    Boolean& canManual()   { return mCanManual; }
    Boolean& active()      { return mActive; }

  private:
    UInt32 mType;
    UInt32 mCntl;
    UInt32 mMinValue;
    UInt32 mMaxValue;
    UInt32 mCurValue;
    Boolean mCanOnePush;
    Boolean mCanReadOut;
    Boolean mCanOnOff;
    Boolean mCanAuto;
    Boolean mCanManual;
    Boolean mActive;
};

// Camera query utilities

// Get one cntl property
rcCameraProperty rfGetCameraProperty( void* cameraPtr,
                                      UInt32 type,
                                      UInt32 cntl );
// Populate a vector of cntl properties
int rfGetCameraProperties( void* cameraPtr,
                           vector<rcCameraProperty>& properties );
// Get maxmimum frame rate
double rfGetCameraMaxFrameRate( void* cameraPtr );
// Get true camera capabilities 
OSStatus rfGetCameraCapabilities( void* cameraPtr,
                                  CameraCapabilitiesStruct& capabilities );
// Get current camera information 
OSStatus rfGetCameraInformation( void* cameraPtr,
                                 const CameraCapabilitiesStruct& capabilities,
                                 rcMovieFileCamExt& info );
// Get supported image formats
OSStatus rfGetCameraFormats( const CameraCapabilitiesStruct& capabilities,
                             vector<rcCameraFormat>& formats );
// Get format specification
rcCameraFormat rfGetFormat( UInt32 format, UInt32 mode );

// Display utilities
ostream& operator << ( ostream& os, const dcamCameraListStruct& cam );
ostream& operator << ( ostream& os, const ASC_CallbackStruct& cs );
ostream& operator << ( ostream& os, const CameraCapabilitiesStruct& cap );

ostream& operator << ( ostream& os, const rcCameraColorFormat& c );
ostream& operator << ( ostream& os, const rcCameraFormat& c );
ostream& operator << ( ostream& os, const rcCameraProperty& c );

#endif //  _rcDCAM_H_
