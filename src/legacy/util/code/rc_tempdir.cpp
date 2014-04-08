

#include <rc_tempdir.h>
#include <stdlib.h>

static std::string staticTempName = std::string ("/var/tmp/rfytmpXXXXXX"); 

std::string rfGetTempFile (std::string& foo)
{
	
    std::string& tmptemplate = (foo == std::string ()) ? staticTempName : foo;        
    return std::string (mktemp((char *) tmptemplate.c_str()));
}

 


