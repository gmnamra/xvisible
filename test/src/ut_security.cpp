//
// $Id: ut_security.cpp 4391 2006-05-02 18:40:03Z armanmg $
//
// Copyright (c) 2002 Reify Corp. All rights reserved.
//

#include <iostream>

#include <rc_timestamp.h>

#include "ut_security.h"

UT_Security::UT_Security()
{
}

UT_Security::~UT_Security()
{
    printSuccessMessage( "rcLicenseManager test", mErrors );
}

uint32 UT_Security::run()
{
    const char testKey1[] = "12345678";
    const char testKey2[] = "22345678";
    const char testKey3[] = "32345678";
    const char testKey4[] = "c0a801dc";
    
    // rcSecurityKey ctor tests
    {
        rcSecurityKey key1( testKey1 );
        rcSecurityKey key1copy( testKey1 );
        rcSecurityKey key2( testKey2 );
        rcSecurityKey key3( testKey3 );
        rcSecurityKey key4( testKey4 );

        // Equality tests
        rcUNITTEST_ASSERT( key1 == key1 );
        rcUNITTEST_ASSERT( key1 == key1copy );
        rcUNITTEST_ASSERT( key1 != key2 );
        rcUNITTEST_ASSERT( key2 != key3 );
        rcUNITTEST_ASSERT( key3 != key4 );
        rcUNITTEST_ASSERT( key1 != key3 );
        rcUNITTEST_ASSERT( key1 != key4 );
        rcUNITTEST_ASSERT( key2 != key4 );
        rcUNITTEST_ASSERT( key3 != key4 );

        // Internal consistency tests
        testKey( key1, testKey1 );
        testKey( key2, testKey2 );
        testKey( key3, testKey3 );
        testKey( key4, testKey4 );

        // Stress test with deterministic seeds
        for ( uint32 i = 0; i < 512; i++ ) {
            // Set a deterministic seed
            srandom( i );
            // Generate random 8-byte text
            std::string text;
            for ( uint32 byte = 0; byte < rcSecurityKey::eTextKeyLength; byte++ )
                text += uint8( random() );
            // Create key and test
            rcSecurityKey key( text );
            testKey( key, text );
        }

        // Stress test with unpredictable seeds
        for ( uint32 i = 0; i < 512; i++ ) {
            // Generate and test random keys
            // Set an unpredictable seed from two clocks
            rcTimestamp now = rcTimestamp::now();
            uint32 secs = uint32(now.secs());
            uint32 seed = uint32( (now.secs() - secs) / getTimestampResolution() + clock() );
            srandom( seed );

            // Generate random 8-byte text
            std::string text;
            for ( uint32 byte = 0; byte < rcSecurityKey::eTextKeyLength; byte++ )
                text += uint8( random() );
            // Create key and test
            rcSecurityKey key( text );
            testKey( key, text );
        }
    }

    // rcEncryptionEngine tests
    {
        rcSecurityKey key1( testKey1 );
        rcSecurityKey key2( testKey2 );
        rcSecurityKey key3( testKey3 );

        // Encrypt some ASCII
        std::string clearText( "Testing!" );
        testEncryptionEngine( key1, clearText );
        testEncryptionEngine( key2, clearText );
        testEncryptionEngine( key3, clearText );

        // Encrypt keys themselves
        testEncryptionEngine( key1, key1.key() );
        testEncryptionEngine( key2, key2.key() );
        testEncryptionEngine( key3, key3.key() );

        // Encrypt cyrus hostid
        testEncryptionEngine( key1, "c0a801dc" );
        testEncryptionEngine( key2, "c0a801dc" );
        testEncryptionEngine( key3, "c0a801dc" );

        // Encrypt cyrus hostid with itself
        testEncryptionEngine( rcSecurityKey("c0a801dc"), "c0a801dc" );

    }

    // rcLicenseManager tests
    {
        // Failure tests
        {
            // Empty license string
            std::string emptyFile( "" );
            rcLicenseManager lm( emptyFile );
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorNoLicense );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorNoLicense );
        }
        {
            // Empty file, read always
            std::string emptyFile;
            rcLicenseManager lm( emptyFile, true );
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorNoLicense );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorNoLicense );
        }
        {
            // Empty file, read once
            std::string emptyFile;
            rcLicenseManager lm( emptyFile, false );
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorNoLicense );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorNoLicense );
        }
        {
            // Bogus license string: wrong length
            std::string bogusFile( "totally_bogus_license_string" );
            rcLicenseManager lm( bogusFile );
            
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorInvalidLicense );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorInvalidLicense );
        }
        {
            // Bogus license string: non-hex chars
            std::string bogusFile( "zz13a389a47eadxx02d095357e512dc4" );
            // Generate correct checksum
            std::string checkString = rcLicenseManager::generateChecksum( bogusFile );
            bogusFile += checkString;
            
            rcLicenseManager lm( bogusFile );
            
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorInvalidLicense );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorInvalidLicense );
        }
        {
            // Bogus license string: invalid host
            std::string bogusFile( "0013a389e47eadaa02d095357e512dc4" );
            // Generate correct checksum
            std::string checkString = rcLicenseManager::generateChecksum( bogusFile );
            bogusFile += checkString;
            
            rcLicenseManager lm( bogusFile );
            
            rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorInvalidHost );
            rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorLicenseExpired );
        }

        // Utility method tests
        {
            // Create a dummy manager 
            std::string emptyLicense( "" );
            rcLicenseManager dm( emptyLicense );

            // Hostid tests
            std::string host = dm.getHostId();
            rcUNITTEST_ASSERT( !host.empty() );

            // Time tests
            time_t t = time( 0 );
            std::string timeString = dm.getTime();
            rcUNITTEST_ASSERT( !timeString.empty() );
            time_t unTime = dm.unhexifyTime( timeString );
            rcUNITTEST_ASSERT( unTime > 0 );
            rcUNITTEST_ASSERT( t <= unTime );
            
            // Hexify/unhexify tests
            rcUNITTEST_ASSERT( dm.hexifyString( "" ) == dm.unhexifyString( "" ) );
            std::string space( " " );
            std::string hexSpace = dm.hexifyString( space );
            std::string unSpace = dm.unhexifyString( hexSpace );

            rcUNITTEST_ASSERT( hexSpace == "20" );
            rcUNITTEST_ASSERT( space == unSpace );
            rcUNITTEST_ASSERT( unSpace == " " );
             
            std::string raw( "12345678" );
            std::string hexString = dm.hexifyString( raw );
            std::string cooked = dm.unhexifyString( hexString );
            rcUNITTEST_ASSERT( raw == cooked );
            
            hexString = dm.hexifyString( host );
            std::string plainString = dm.unhexifyString( hexString );
            rcUNITTEST_ASSERT( host == plainString );
            
            std::string hexString2 = dm.hexifyString( plainString );
            rcUNITTEST_ASSERT( hexString == hexString2 );
        }
        
        // Basic tests with license strings
        {

            // Create a dummy manager to get current host id
            std::string emptyLicense( "" );
            rcLicenseManager dm( emptyLicense );
            std::string host = dm.getHostId();
            
            // Use default application key
            std::string keyString( rcA, rcSecurityKey::eTextKeyLength );
            rcSecurityKey key( keyString );
            rcEncryptionEngine e( key );
            // Create encrypted host string
            std::string encryptedHostString = e.encrypt( host );
            // Hexify host string
            std::string encryptedHostHexString = dm.hexifyString( encryptedHostString );

            // Valid license string with infinite expiration
            {
                // Create encrypted time string without expiration
                std::string encryptedTimeString = e.encrypt( "7fffffff" );
                // Hexify time string
                std::string encryptedTimeHexString = dm.hexifyString( encryptedTimeString );
                // Create combined license string
                std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
                // Generate checksum
                std::string checkString = rcLicenseManager::generateChecksum( encryptedHexLicense );
                // Add checksum 
                encryptedHexLicense += checkString;
                
                // Create valid license for this machine
                rcLicenseManager lm( encryptedHexLicense );

                rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorOK );
                rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorOK );
                rcUNITTEST_ASSERT( !lm.getLicenseExpirationTime().empty() );
            }

            // Valid license string with no checksum
            {
                // Create encrypted time string without expiration
                std::string encryptedTimeString = e.encrypt( "7fffffff" );
                // Hexify time string
                std::string encryptedTimeHexString = dm.hexifyString( encryptedTimeString );
                // Create combined license string
                std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
                // Create license for this machine
                rcLicenseManager lm( encryptedHexLicense );

                rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorInvalidLicense );
                rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorInvalidLicense );
                rcUNITTEST_ASSERT( ! lm.getLicenseExpirationTime().empty() );
            }
            
            // Valid license string with wrong checksum
            {
                // Create encrypted time string without expiration
                std::string encryptedTimeString = e.encrypt( "7fffffff" );
                // Hexify time string
                std::string encryptedTimeHexString = dm.hexifyString( encryptedTimeString );
                // Create combined license string
                std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
                // Generate checksum
                std::string checkString = rcLicenseManager::generateChecksum( encryptedHexLicense );
                // Mutate checksum to make it invalid
                for ( uint32 i = 0; i < checkString.length(); i++ )
                    checkString[i] = checkString[i] -1;
                
                encryptedHexLicense += checkString;
                
                // Create license for this machine
                rcLicenseManager lm( encryptedHexLicense );

                rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorInvalidLicense );
                rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorInvalidLicense );
                rcUNITTEST_ASSERT( ! lm.getLicenseExpirationTime().empty() );
            }
            
            // Valid license string with already expired time
            {
                std::string encryptedTimeString = e.encrypt( "00000000" );
                // Hexify time string
                std::string encryptedTimeHexString = dm.hexifyString( encryptedTimeString );
                // Create combined license string
                std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
                // Generate checksum
                std::string checkString = rcLicenseManager::generateChecksum( encryptedHexLicense );
                // Add checksum 
                encryptedHexLicense += checkString;
                // Create valid license for this machine
                rcLicenseManager lm( encryptedHexLicense );
                
                rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorOK );
                rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorLicenseExpired );
                rcUNITTEST_ASSERT( lm.getLicenseHost() == host );
            }

            // Valid license string with already expired time
            {
                // Current time has expired already
                std::string timeString = dm.getTime();
                std::string encryptedTimeString = e.encrypt( timeString );
                // Hexify time string
                std::string encryptedTimeHexString = dm.hexifyString( encryptedTimeString );
                // Create combined license string
                std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
                 // Generate checksum
                std::string checkString = rcLicenseManager::generateChecksum( encryptedHexLicense );
                // Add checksum 
                encryptedHexLicense += checkString;
                // Create valid license for this machine
                rcLicenseManager lm( encryptedHexLicense );
                
                rcUNITTEST_ASSERT( lm.validHost() == eSecurityErrorOK );
                rcUNITTEST_ASSERT( lm.validTime() == eSecurityErrorLicenseExpired );
                rcUNITTEST_ASSERT( lm.getLicenseHost() == host );
            }
        }
    }
    
    return mErrors;  
}

// private

// Test key self-consistency
void UT_Security::testKey( const rcSecurityKey& key, const std::string& keyText )
{
    rcUNITTEST_ASSERT( key.key() == keyText );
     
    rcUNITTEST_ASSERT( key.key() != key.binaryKey() );
    rcUNITTEST_ASSERT( key.binaryKey() == key.binaryKey( key.key() ) );
    rcUNITTEST_ASSERT( key.key() == key.textKey( key.binaryKey() ) );
}

// Test engine roundtripping with a key and a clear text
void UT_Security::testEncryptionEngine( const rcSecurityKey& key, const std::string& clearText )
{
    // Create engine
    rcEncryptionEngine e( key );
        
    // Encrypt
    std::string cipherText = e.encrypt( clearText );
    rcUNITTEST_ASSERT( cipherText.length() == clearText.length() );        
    rcUNITTEST_ASSERT( cipherText != clearText );

    // Decrypt
    std::string decryptedText = e.decrypt( cipherText );
    rcUNITTEST_ASSERT( decryptedText.length() == clearText.length() );
    rcUNITTEST_ASSERT( decryptedText == clearText );
}
