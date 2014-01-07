#include <strstream>

#include <rc_security.h>

static const char licenseFile[] = "visible.rfylic";

static int requiredArgCount = 4;

static const char hostInfoArg[] = "-host";
static const char expirationArg[] = "-expiration";
static const char usageArg[] = "-usage";
static const char helpArg[] = "-help";

// Display usage
static int usage( const char* app )
{
    const char* shortAppName;
    
    const char* slash = strrchr( app, '/' );
    if ( slash )
        shortAppName = slash+1;
    else
        shortAppName = app;
       
    fprintf( stdout, "\nUsage: %s %s <host-info-string> %s <mm/dd/yy|never>\n",
             shortAppName,
             hostInfoArg,
             expirationArg );

    return 0;
}

// Display argument count error
static int argCountError( const char* app, int argCount, int requiredCount )
{
    if ( argCount < requiredCount )
        fprintf( stdout, "Error: Not enough arguments, %i arguments required\n",
                 requiredCount );
    else if ( argCount > requiredCount )
         fprintf( stdout, "Error: too many arguments, %i arguments required\n",
                  requiredCount );
    return usage( app );
}

// Display argument error
static int argError( const char* app, const char* arg )
{
    fprintf( stdout, "Error: Invalid argument %s\n",
             arg );
    return usage( app );
}

// Display host string format error
static int hostError( const char* app, const char* type, const char* str, int length )
{
    fprintf( stdout, "Error: Invalid %s host info string \"%s\", length %i is invalid\n",
             type,
             str,
             length );
    return usage( app );
}

// Display expiration time format error
static int timeError( const char* app, const char* str )
{
    fprintf( stdout, "Error: Invalid expiration date string \"%s\"\n",
             str );
    return usage( app );
}

int main( int argc, char** argv )
{
    int errors = 0;
    
	try
	{
        if ( argc == 1 ) {
            // No args, display usage
            return usage( argv[0] );
        } 

        char* hostInfo = 0;
        char* expirationDate = 0;
        
        for ( int i = 1; i < argc; i++ ) {
            if ( !strcmp( argv[i], usageArg ) ||
                 !strcmp( argv[i], helpArg )) {
                return usage( argv[0] );
            } else if ( !strcmp( argv[i], hostInfoArg ) ) {
                hostInfo = argv[++i];
                if ( i == argc )
                    return argCountError( argv[0] , argc-1, requiredArgCount );
            } else if ( !strcmp( argv[i], expirationArg ) ) {
                expirationDate = argv[++i];
                if ( i == argc )
                    return argCountError( argv[0] , argc-1, requiredArgCount );
            }
            else {
                return argError( argv[0] , argv[i] );
            }
        }

        if ( hostInfo && expirationDate ) {
            std::string decryptedHostString;
            //
            // Parse host info string
            //
            if ( strlen( hostInfo ) != rcSecurityKey::eTextKeyLength * 2 )
                return hostError( argv[0], "encrypted", hostInfo, strlen( hostInfo ) );
            else {
                rcSecurityKey hostKey = rcSecurityKey( std::string( rcH, rcSecurityKey::eTextKeyLength ) );
                rcEncryptionEngine decryptor = rcEncryptionEngine( hostKey );

                // Unhexify encrypted host string
                std::string encryptedHostString = rcLicenseManager::unhexifyString( std::string( hostInfo ) );
        
                if ( encryptedHostString.length() == rcSecurityKey::eTextKeyLength ) {
                    decryptedHostString = decryptor.decrypt( encryptedHostString );
                    
                } else {
                    return hostError( argv[0], "decrypted", encryptedHostString.c_str(), encryptedHostString.length() );
                }
            }
            
            //
            // Parse expiration date
            //

            time_t expirationTime = 0;
            
            // Use application key
            std::string keyString( rcA, rcSecurityKey::eTextKeyLength );
            rcSecurityKey key( keyString );
            rcEncryptionEngine encryptor( key );

            // Create encrypted host string
            std::string encryptedHostString = encryptor.encrypt( decryptedHostString );
            // Hexify host string
            std::string encryptedHostHexString = rcLicenseManager::hexifyString( encryptedHostString );

            if ( !strcmp( expirationDate, "never" ) ) {
                // Never say never, this expires in 2038
                expirationTime = 0x7fffffff;
            } else {
                // Parse expiration date string
                struct tm parsedTime;
                parsedTime.tm_sec = 1;
                parsedTime.tm_min = 0;
                parsedTime.tm_hour = 0;
                // We require MM/DD/YY, ie. 12/01/03
                const char* format ="%m/%d/%y";
                
                if ( strptime( expirationDate, format, &parsedTime ) ) {
                    expirationTime = mktime( &parsedTime );
                } else {
                    // Parsing failed
                    return timeError( argv[0], expirationDate );
                }
            }
            char buf[1024];

            // Construct format string
            strstream format;
            format << "%." << rcSecurityKey::eTextKeyLength << "lx" << ends;
            // Construct time string
            snprintf( buf, rmDim(buf), format.str(), expirationTime );
            format.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
         
            // Encrypt time string
            std::string encryptedTimeString = encryptor.encrypt( buf );
            // Hexify time string
            std::string encryptedTimeHexString = rcLicenseManager::hexifyString( encryptedTimeString );
            // Create combined license string
            std::string encryptedHexLicense = encryptedHostHexString + encryptedTimeHexString;
            // Generate checksum
            std::string checkSum = rcLicenseManager::generateChecksum( encryptedHexLicense );
            // Add checksum
            encryptedHexLicense += checkSum;
            
            // Output file for license string
            FILE* fout = fopen ( licenseFile, "w" );
            
            if ( fout ) {
                std::string  decryptedHexHostString = rcLicenseManager::hexifyString( decryptedHostString );
                fprintf( stdout, "\nCreated license file %s\n\n", licenseFile );
                fprintf( stdout, "\t%-28s: %s\n", "License string", encryptedHexLicense.c_str() );
                fprintf( stdout, "\t%-28s: %s\n", "Licensed host", decryptedHexHostString.c_str() );
#ifdef SHOW_ASC_DCAM_LICENSE
                // This string is compatible with host id produced by ASC DCAM
                // Register Mac SN.app
                fprintf( stdout, "\t%-28s: 0000%.12s\n", "ASC DCAM host", decryptedHexHostString.c_str() );
#endif                
                fprintf( stdout, "\t%-28s: %s\n", "License expiration date", ctime( &expirationTime ) );
                fprintf( fout, "%s\n", encryptedHexLicense.c_str() );
                fclose( fout );
            } else {
                fprintf( stderr, "Error: could not open file %s\n", licenseFile );
                fprintf( stdout, "%-28s: %s\n", "License string", encryptedHexLicense.c_str() );
            }
        } else {
            return argCountError( argv[0] , argc-1, requiredArgCount );
        }
	}
	catch ( exception& x )
	{
        fprintf( stderr, "Caught exception %s\n", x.what() );
		exit( 1 );
	}
    catch ( ... )
    {
        fprintf( stderr, "Caught unknown exception \n" );
        exit( 1 );
    }

    return errors;
}

