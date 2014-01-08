/*
 *
 * $Id: rc_security.h 6756 2009-04-01 22:17:05Z arman $
 *
 * This file contains security infrastructure declarations.
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcSECURITY_H_
#define _rcSECURITY_H_

#include <rc_types.h>

// Global declarations

// The names have been scarmbled for obfuscation
// Key for encrypting/decrypting application license
const char rcA[] = "r$fqy(3=";
// Dummy keys for obfuscation
const char rcB[] = "45yi(j'=";
const char rcC[] = "856#jdnb";
const char rcD[] = "yup9$#f,";
const char rcE[] = "5r0-fbny";
const char rcF[] = "@9irvb./";
const char rcG[] = "8qf$.v(3";
// Key for encrypting/decrypting host info
// Warning: do not change this without changing rfyhostinfo executable, too!
const char rcH[] = "y/fnr.8v";
// Dummy keys for obfuscation
const char rcI[] = "65yh(j'r";
const char rcJ[] = "%93rwb.9";
const char rcK[] = "*ei,vbbp";

// Security error type
enum rcSecurityError {
    eSecurityErrorOK = 0,         // No error
    eSecurityErrorUnknown,        // Unknown error
    eSecurityErrorNoLicense,      // No license file found
    eSecurityErrorInvalidLicense, // Invalid/corrupt license file
    eSecurityErrorInvalidHost,    // License not valid for host
    eSecurityErrorLicenseExpired, // License has expired
    eSecurityErrorInternal,       // Internal error
};

//
// Encryption/decryption key for DES algorithm
//
// Limitation: text key must be 8 bytes and binary key 64 bytes long

class RFY_API rcSecurityKey {
  public:

    // Internal length constants
    enum {
        eTextKeyLength = 8,                  
        eBinaryKeyLength = eTextKeyLength*8
    };
    
    // ctors
    rcSecurityKey( const char* key ) : mKey( key ), mBinaryKey( binaryKey( mKey ) ) {
        rmAssert( mKey.length() == eTextKeyLength );
        rmAssert( mBinaryKey.length() == eBinaryKeyLength );
    };
    
    rcSecurityKey( const std::string& key ) : mKey( key ), mBinaryKey(  binaryKey( mKey ) ) {
        rmAssert( mKey.length() == eTextKeyLength );
        rmAssert( mBinaryKey.length() == eBinaryKeyLength );
    };

    // Accessors

    const std::string& key() const { return mKey; };
    const std::string& binaryKey() const { return mBinaryKey; };
    // Utilities
    
    // Build a binary key from text key
    static std::string binaryKey( const std::string& textKey );
    // Build a text key from binary key
    static std::string textKey( const std::string& binaryKey );

    // Operators
    bool operator==(const rcSecurityKey& k) const {
        return ( mKey == k.mKey && mBinaryKey == k.mBinaryKey );
    };
    bool operator!=(const rcSecurityKey& k) const {
        return ! (*this == k );
    };

    friend class rcEncryptionEngine;
    
  private:
    std::string mKey;          // Text key
    std::string mBinaryKey;    // Binary key
};

//
// Class for encrypting/decrypting data
//
// Limitation: input text for encryption/decryption must be exactly
// 8 bytes long

class RFY_API rcEncryptionEngine {
  public:
    // ctor : same key used for the lifetime of the engine
    rcEncryptionEngine( const rcSecurityKey& key ) : mKey( key ) { };
    
    // Return text encrypted with key
    std::string encrypt( const std::string& clearText );
    // Return text decrypted with key
    std::string decrypt( const std::string& cipherText );

  private:
    rcSecurityKey mKey; // Encryption/decryption key
};

//
// Class for user license management
//

class RFY_API rcLicenseManager {
  public:
    // Internal length constants
    enum {
        eLicenseChecksumLength = 2,
        eLicenseComponentLength = rcSecurityKey::eTextKeyLength*2, // Length of hexified component
        eLicenseTotalLength = eLicenseComponentLength*2 + eLicenseChecksumLength // Length of hexified license + checksum
    };
    
    // ctors
    // If readAlways is true, license file will be reread from disk for
    // every validity check
    rcLicenseManager( const std::string& licenseFileName, bool readAlways );
    rcLicenseManager( const std::string& licenseString );
    
    // Check license host validity
    rcSecurityError validHost();
    // Check license expiration time
    rcSecurityError validTime();

    // Return expiration time from license file
    std::string getLicenseExpirationTime();
    // Return host id from license file
    std::string getLicenseHost();
    // Return host id of this machine as a hex string
    std::string getHostId() const;
    
    // Static method for mapping an error value to a string
    static std::string getErrorString( rcSecurityError error );
    // Static method to ransform a hex string into raw character string
    static std::string unhexifyString( const std::string& str );
    // Transform a raw character string into hex string
    static std::string hexifyString( const std::string& str );
    // Transform a hex time into a number
    static time_t unhexifyTime( const std::string& str );
    // Generate hex checksum for string
    static std::string generateChecksum( const std::string& str );

    friend class UT_Security; // For unit-testing of private methods
     
  private:
    // Return current time as a hex string
    std::string getTime() const;

    // Verify checksum
    rcSecurityError licenseChecksumIsValid();
    // Verify license string host component format validity
    rcSecurityError licenseHostHasValidFormat();
    // Verify license string time component format validity
    rcSecurityError licenseTimeHasValidFormat();
    // Read license file
    rcSecurityError readFile();
    // Check license file validity
    rcSecurityError validFile();
    
    std::string           mLicenseFile;    // License file name
    std::string           mLicenseString;  // License string
    rcSecurityKey      mHostKey;        // Encryption key of the current application host
    rcEncryptionEngine mEncryptor;      // Encryption engine

    time_t             mExpirationTime; // Software expiration date
    bool               mReadAlways;     // Always reread file with every check
    std::string           mDecryptedTimeString; // Time string parsed and decrypted from file
    std::string           mDecryptedHostString; // Host string parsed and decrypted from file
};

#endif // _rcSECURITY_H_
