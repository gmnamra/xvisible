/******************************************************************************
 *   Copyright (c) 2002 Reify Corp. All Rights reserved.
 *
 *	$Id: rc_security.cpp 6565 2009-01-30 03:24:44Z arman $
 *
 *	This file contains security infrastructure implementation.
 *
 ******************************************************************************/
#include <unistd.h>
#include <iostream>
#include <strstream>
#include <rc_exception.h>

#include <rc_thread.h>
#include <rc_security.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

//
// Local utilities
//

// Returns an iterator containing the primary (built-in) Ethernet interface. The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t findEthernetInterfaces( io_iterator_t *matchingServices )
{
    kern_return_t		kernResult;
    mach_port_t			masterPort;

    // Retrieve the Mach port used to initiate communication with I/O Kit
    kernResult = IOMasterPort( MACH_PORT_NULL, &masterPort );

    if ( KERN_SUCCESS != kernResult ) {
        fprintf( stderr, "Error: IOMasterPort returned %d\n", kernResult );
        return kernResult;
    }

    CFMutableDictionaryRef matchingDict = IOServiceMatching( kIOEthernetInterfaceClass );

    if ( NULL == matchingDict ) {
        fprintf( stderr, "Error: IOServiceMatching returned a NULL dictionary.\n" );
    }
    else {
        CFMutableDictionaryRef propertyMatchDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                                                                              &kCFTypeDictionaryKeyCallBacks,
                                                                              &kCFTypeDictionaryValueCallBacks );

        if ( NULL == propertyMatchDict ) {
            fprintf( stderr, "Error: CFDictionaryCreateMutable returned a NULL dictionary.\n");
        }
        else {
            CFDictionarySetValue( propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue );
            CFDictionarySetValue( matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict );
            CFRelease( propertyMatchDict );
        }
    }

    kernResult = IOServiceGetMatchingServices( masterPort, matchingDict, matchingServices );

    if ( KERN_SUCCESS != kernResult ) {
        fprintf( stderr, "Error: IOServiceGetMatchingServices returned %d\n", kernResult );
    }

    return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
static kern_return_t getMACAddress( io_iterator_t intfIterator, UInt8 *MACAddress )
{
    io_object_t		intfService;
    kern_return_t	kernResult = KERN_FAILURE;

    // IOIteratorNext retains the returned object, so release it when we're done with it.
    while ( (intfService = IOIteratorNext(intfIterator)) )
    {
        io_object_t		controllerService;

        // IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
        kernResult = IORegistryEntryGetParentEntry( intfService,
                                                    kIOServicePlane,
                                                    &controllerService );

        if ( KERN_SUCCESS != kernResult ) {
            fprintf( stderr, "Error: IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult );
        }
        else {
            // Retrieve the MAC address property from the I/O Registry in the form of a CFData
            CFTypeRef MACAddressAsCFData = IORegistryEntryCreateCFProperty( controllerService,
                                                                            CFSTR(kIOMACAddress),
                                                                            kCFAllocatorDefault,
                                                                            0 );
            if ( MACAddressAsCFData ) {
                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes( (CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress );
                CFRelease( MACAddressAsCFData );
            }

            // Done with the parent Ethernet controller object so we release it.
            (void) IOObjectRelease( controllerService );
        }

        // Done with the Ethernet interface object so we release it.
        (void) IOObjectRelease( intfService );
    }

    return kernResult;
}

// Mutex for DES calls
static rcMutex mDESMutex;

//
// rcSecurityKey class implementation
//

// public

// Build a binary key array from text key
std::string rcSecurityKey::binaryKey( const std::string& textKey ) {
    // Support for 8-byte keys only
    rmAssert( textKey.length() == eTextKeyLength );
    std::string binKey;

    // Produce a 64-byte binary array from 8 bytes
    for ( int byte = 0; byte < eTextKeyLength; byte++ ) {
        for ( int bit = 0; bit < 8; bit++ ) {
            binKey += (textKey[byte] & (1 << bit)) > 0 ? 1 : 0;
        }
    }

    rmAssert( binKey.length() == eBinaryKeyLength );

    return binKey;
}

// Build a text key from binary key
std::string rcSecurityKey::textKey( const std::string& binaryKey ) {
    // Support for 64-byte keys only
    rmAssert( binaryKey.length() == eBinaryKeyLength );
    std::string textKey;

    // Produce a 8-byte array from a 64-byte binary array
    for ( int byte = 0; byte < eTextKeyLength; byte++ ) {
        char b = 0;
        for ( int bit = 0; bit < 8; bit++ ) {
            if ( binaryKey[byte*8+bit] )
                b += (1 << bit);
        }
        textKey += b;
    }
    rmAssert( textKey.length() == eTextKeyLength );

    return textKey;
}

//
//  rcEncryptionEngine class implementation
//

// public

// Return text encrypted with key
std::string rcEncryptionEngine::encrypt( const std::string& clearText )
{
    rmAssert( clearText.length() == rcSecurityKey::eTextKeyLength );

    std::string cipherText;
    rcLock lock( mDESMutex );

    setkey( mKey.binaryKey().c_str());
	{
        rcSecurityKey binaryTextKey( clearText );
        char binaryText[rcSecurityKey::eBinaryKeyLength];
        memcpy( binaryText, binaryTextKey.binaryKey().c_str(), rmDim( binaryText ) );
        
        // Encrypt DARWIN_UNIX03 mods
        ::encrypt( binaryText, 0);
        cipherText = rcSecurityKey::textKey( std::string( binaryText, rmDim( binaryText ) ) );
        rmAssert( clearText.length() == cipherText.length() );
    } 
	
    return cipherText;
}
    
// Return text decrypted with key
std::string rcEncryptionEngine::decrypt( const std::string& cipherText )
{
    rmAssert( cipherText.length() == rcSecurityKey::eTextKeyLength );
    
    std::string clearText;
    rcLock lock( mDESMutex );

    setkey( mKey.binaryKey().c_str());
    {
        rcSecurityKey binaryTextKey( cipherText );
        char binaryText[rcSecurityKey::eBinaryKeyLength];
        memcpy( binaryText, binaryTextKey.binaryKey().c_str(), rmDim( binaryText ) );
        
        // Decrypt
        ::encrypt( binaryText, 1);
        clearText = rcSecurityKey::textKey( std::string( binaryText, rmDim( binaryText ) ) );
        rmAssert( clearText.length() == cipherText.length() );
    }
    return clearText;
}


//
// rcLicenseManager class implementation
//

rcLicenseManager::rcLicenseManager( const std::string& licenseFileName, bool readAlways ) :
        mLicenseFile( licenseFileName ),
        mLicenseString(),
        mHostKey( rcSecurityKey( std::string( rcE, rcSecurityKey::eTextKeyLength ) ) ),
        mEncryptor( mHostKey ),
        mExpirationTime( 0 ),
        mReadAlways( true )
{
    // Dummy ctors for obfuscation
    mHostKey = rcSecurityKey( std::string( rcB, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );
    mHostKey = rcSecurityKey( std::string( rcK, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );
    // The real thing
    mHostKey = rcSecurityKey( std::string( rcA, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );

    // Read file once
    readFile();
    mReadAlways = readAlways;
}

rcLicenseManager::rcLicenseManager( const std::string& licenseString ) :
        mLicenseFile(),
        mLicenseString( licenseString ),
        mHostKey( rcSecurityKey( std::string( rcD, rcSecurityKey::eTextKeyLength ) ) ),
        mEncryptor( mHostKey ),
        mExpirationTime( 0 ),
        mReadAlways( false )
{
    // Dummy ctors for obfuscation
    mHostKey = rcSecurityKey( std::string( rcC, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );
    mHostKey = rcSecurityKey( std::string( rcI, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );
    // The real thing
    mHostKey = rcSecurityKey( std::string( rcA, rcSecurityKey::eTextKeyLength ) );
    mEncryptor = rcEncryptionEngine( mHostKey );
}

// Check if license host is valid
rcSecurityError rcLicenseManager::validHost()
{
    rcSecurityError status = validFile();

    if ( status == eSecurityErrorOK ) {
        std::string host = getHostId();

        if ( host.length() ) {
            if ( mDecryptedHostString.length() ) {
                if ( mDecryptedHostString == host ) {
                    status = eSecurityErrorOK;
                } else {
                    status = eSecurityErrorInvalidHost;
                }
            } else {
                status = eSecurityErrorInvalidLicense;
            }
        } else {
            status = eSecurityErrorInternal;
        }
    }

    return status;
}

// Check if license time is valid
rcSecurityError rcLicenseManager::validTime()
{
    rcSecurityError status = validFile();

    if ( status == eSecurityErrorOK ) {
        time_t cur = time( 0 );
        if ( cur >= mExpirationTime ) {
            status = eSecurityErrorLicenseExpired;
        } else {
            status = eSecurityErrorOK;
        }
    }

    return status;
}

// Return expiration time from license file
// This is used for GUI display
std::string rcLicenseManager::getLicenseExpirationTime()
{
    std::string exp = "N/A";
    rcSecurityError status = validFile();

    if ( status == eSecurityErrorOK ) {
        char buf[27];
        if ( ctime_r( &mExpirationTime, buf ) )
            exp = std::string( buf );
        else
            exp = "invalid license";
    } else {
        switch ( status ) {
            case eSecurityErrorNoLicense:
                exp = "no license";
                break;
            default:
                exp = "invalid license";
                break;
        }
    }

    return exp;
}

// Return host id from license file
std::string rcLicenseManager::getLicenseHost()
{
    std::string host = "N/A";

    rcSecurityError status = validFile();

    if ( status == eSecurityErrorOK ) {
        host = mDecryptedHostString;
    } else {
        switch ( status ) {
            case eSecurityErrorNoLicense:
                host = "no license";
                break;
            default:
                host = "invalid license";
                break;
        }
    }

    return host;
}

// Static method to transform a hex string into raw character string
std::string rcLicenseManager::unhexifyString( const std::string& str )
{
    std::string keyString;
    // Hex string is supposed to have two chars per digit
    // If the length is odd, we'll discard the last char
    uint32 len = str.length()/2;

    char digit[3];
    digit[2] = 0;

    for ( uint32 i = 0; i < len; ++i ) {
        digit[0] = str[i*2];
        digit[1] = str[i*2+1];

        uint32 d;
        if ( sscanf( digit, "%x", &d ) == 1 ) {
            keyString += static_cast<unsigned char>(d);
        } else {
            // Error, bail out
            break;
        }
    }

    return keyString;
}

// Static method to transform a hex time into a number
time_t rcLicenseManager::unhexifyTime( const std::string& str )
{
    time_t t = 0;

    if ( sscanf( str.c_str(), "%lx", &t ) == 1 ) {
    } else {
        // Error
    }

    return t;
}

// Static method to transform a raw character string into hex string
std::string rcLicenseManager::hexifyString( const std::string& str )
{
    std::string hexString;

    for ( uint32 i = 0; i < str.length(); i++ ) {
        unsigned char c = str[i];
        char buf[3];

        snprintf( buf, rmDim(buf), "%.2x", c );
        hexString += buf;
    }

    return hexString;
}

// Static method for mapping an error value to a string
std::string rcLicenseManager::getErrorString( rcSecurityError error )
{
    switch ( error ) {
        case eSecurityErrorOK:
            return std::string( "License manager: OK" );
        case eSecurityErrorUnknown:
            return std::string( "License manager: unknown error" );
        case eSecurityErrorNoLicense:
            return std::string( "License manager: cannot read license file" );
        case eSecurityErrorInvalidLicense:
            return std::string( "License manager: invalid license file" );
        case eSecurityErrorInvalidHost:
            return std::string( "License manager: no license for this host" );
        case eSecurityErrorLicenseExpired:
            return std::string( "License manager: license has expired" );
        case eSecurityErrorInternal:
            return std::string( "License manager: internal error" );
            // Note: no default case to force a compiler warning if a new enum value
            // is defined without adding a corresponding string here.
    }

    return std::string( "License manager: undefined error" );
}

// Static method for generating a checksum character for string
std::string rcLicenseManager::generateChecksum( const std::string& str )
{
    int32 sum = 0;

    // Sum all char values
    for ( uint32 i = 0; i < str.length(); i++ ) {
        unsigned char c = str[i];
        sum += c;
    }
    // Checksum is the remainder
    char c = (sum % 256);
    std::string checkString = hexifyString( std::string( &c, 1 ) );
    rmAssert( checkString.length() == eLicenseChecksumLength );

    return checkString;
}

// private

// Return host id of this machine
std::string rcLicenseManager::getHostId() const
{
    std::string id;
    io_iterator_t	intfIterator;
    kern_return_t	kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
    size_t len = kIOEthernetAddressSize;
    if ( len < rcSecurityKey::eTextKeyLength )
        len = rcSecurityKey::eTextKeyLength;

    // Allocate memory for id
    char* buf = new char[len];
    memset(buf, 0, len);

    kernResult = findEthernetInterfaces(&intfIterator);

    if ( kernResult == KERN_SUCCESS  ) {
        kernResult = getMACAddress(intfIterator, (UInt8*)buf);
        if ( kernResult == KERN_SUCCESS ) {
            id = std::string( buf, len );
        }
    }

    delete [] buf;
    (void) IOObjectRelease(intfIterator);	// Release the iterator.

    return id;
}

// Get current time as a hex string
std::string rcLicenseManager::getTime() const
{
    std::string curTime;
    time_t t = time( 0 );
    char buf[32];

    int count = snprintf( buf, rmDim(buf), "%lx", t );
    // Use value only if buf does not overflow
    if ( static_cast<unsigned>(count) <  rmDim(buf) )
        curTime = buf;

    return curTime;
}

// Verify checksum
rcSecurityError rcLicenseManager::licenseChecksumIsValid()
{
    rcSecurityError status = eSecurityErrorUnknown;

    // Checksum verification
    std::string licenseChecksum( mLicenseString, eLicenseTotalLength - eLicenseChecksumLength, eLicenseChecksumLength );
    std::string licenseWithoutChecksum( mLicenseString, 0, eLicenseTotalLength - eLicenseChecksumLength );
    std::string generatedChecksum = generateChecksum( licenseWithoutChecksum );

    if ( generatedChecksum == licenseChecksum )
        status = eSecurityErrorOK;
    else
        status = eSecurityErrorInvalidLicense;

    return status;
}


// Verify license string host component format validity
rcSecurityError rcLicenseManager::licenseHostHasValidFormat()
{
    rcSecurityError status = eSecurityErrorUnknown;

    // Get host component
    std::string hostString( mLicenseString, 0, eLicenseComponentLength );
    // Unhexify encrypted host string
    std::string encryptedHostString = unhexifyString( hostString );

    if ( encryptedHostString.length() == rcSecurityKey::eTextKeyLength ) {
        // Dummy string
        std::string dString = encryptedHostString;
        dString[5] = 32;
        // Do dummy encryptions/decryptions
        mDecryptedHostString = mEncryptor.decrypt( dString );
        dString = mEncryptor.encrypt( mDecryptedHostString );
        dString = mEncryptor.decrypt( mDecryptedHostString );
        // Decrypt the real thing
        dString = mEncryptor.decrypt( encryptedHostString );
        // Encrypt the real thing
        mDecryptedHostString = mEncryptor.encrypt( dString );
        // Decrypt the real thing
        mDecryptedHostString = mEncryptor.decrypt( mDecryptedHostString );
        // Do a dummy decryption
        dString = mEncryptor.decrypt( mDecryptedHostString );

        // Check validity of decryption
        if ( mDecryptedHostString.length() == rcSecurityKey::eTextKeyLength ) {
            // Host is valid
            status = eSecurityErrorOK;
        } else {
            status = eSecurityErrorInvalidLicense;
        }
    } else {
        status = eSecurityErrorInvalidLicense;
    }

    return status;
}

// Verify license string time component format validity
rcSecurityError rcLicenseManager::licenseTimeHasValidFormat()
{
    rcSecurityError status = eSecurityErrorUnknown;

    // Get time component
    std::string timeString( mLicenseString, eLicenseComponentLength, eLicenseComponentLength );
    // Get checksum
    std::string checkString( mLicenseString, eLicenseTotalLength-eLicenseChecksumLength, eLicenseChecksumLength );

    // Unhexify encrypted time string
    std::string encryptedTimeString = unhexifyString( timeString );

    if ( encryptedTimeString.length() == rcSecurityKey::eTextKeyLength ) {
        // Dummy string
        std::string dString = encryptedTimeString;
        dString[3] = 37;
        // Do dummy encryptions/decryptions
        mDecryptedTimeString = mEncryptor.decrypt( dString );
        dString = mEncryptor.encrypt( mDecryptedTimeString );
        dString = mEncryptor.decrypt( mDecryptedTimeString );
        // Decrypt the real thing
        dString = mEncryptor.decrypt( encryptedTimeString );
        // Encrypt the real thing
        mDecryptedTimeString = mEncryptor.encrypt( dString );
        // Decrypt the real thing
        mDecryptedTimeString = mEncryptor.decrypt( mDecryptedTimeString );
        // Do a dummy decryption
        dString = mEncryptor.decrypt( mDecryptedTimeString );

        // Check validity of decryption
        if ( mDecryptedTimeString.length() == rcSecurityKey::eTextKeyLength ) {
            // Time is valid
            mExpirationTime = unhexifyTime( mDecryptedTimeString );
            status = eSecurityErrorOK;
        } else {
            status = eSecurityErrorInvalidLicense;
        }
    } else {
        status = eSecurityErrorInvalidLicense;
    }

    return status;
}

// Read license file
rcSecurityError rcLicenseManager::readFile()
{
    rcSecurityError status = eSecurityErrorUnknown;

    if ( mReadAlways ) {
        FILE* file = fopen( mLicenseFile.c_str(), "r" );

        if ( file ) {
            char buf[eLicenseTotalLength+1];
            // Construct license format string
            strstream format;
            format << "%" << eLicenseTotalLength << "s" << ends;

            // Read from license file
            int count = fscanf( file, format.str(), buf );
            format.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns

            // If read succeeded, set license string
            if ( count == 1 ) {
                mLicenseString = buf;
                status = eSecurityErrorOK;
            } else {
                status = eSecurityErrorInvalidLicense;
            }
            fclose( file );
        } else {
            status = eSecurityErrorNoLicense;
        }
    }

    return status;
}

// Check if license file is valid
// Return true for valid file, false otherwise
rcSecurityError rcLicenseManager::validFile()
{
    rcSecurityError status = eSecurityErrorUnknown;

    if ( mReadAlways )
        status = readFile();
    else
        status = eSecurityErrorOK;

    if ( status == eSecurityErrorOK ) {
        // Note: these checks only verify whether the values have
        // a valid format/range, their actual values may be incorrect.

        if ( mLicenseString.length() == eLicenseTotalLength ) {
            // Check host
            status = licenseHostHasValidFormat();

            if ( status == eSecurityErrorOK ) {
                // Host is valid, now check time
                status = licenseTimeHasValidFormat();

                if ( status == eSecurityErrorOK ) {
                    // Host and time are valid, now verify checksum
                    status = licenseChecksumIsValid();
                }
            } else {
                // Invalid checksum
            }
        } else {
            // Invalid license string length
            if ( mLicenseString.empty() )
                status = eSecurityErrorNoLicense;
            else
                status = eSecurityErrorInvalidLicense;
        }
    }

    return status;
}
