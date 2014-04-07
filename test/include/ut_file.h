#ifndef __UT_FILE_H
#define __UT_FILE_H


#include <rc_unittest.h>
#include <rc_fileutils.h>
#include <string>

using namespace std;

class UT_fileutils : public rcUnitTest {
public:
    
    UT_fileutils() {}
    ~UT_fileutils() {}
    
    virtual uint32 run()
    {
        int errs = 0;
        std::string pathy ("/Users/arman/boo/foo/test_o_0.rfymov");
        std::string rfmovfile ("test_o_0");
        std::string filename ("test_o_0.rfymov");
        std::string Filename ("test_O_0.rfymov");        
        std::string fileext ("rfymov");
        std::string dotfileext (".rfymov");        
        std::string dormovfileext (".mov");                
        
        std::string sanspath = rfStripPath (pathy);

        errs += (rf_sensitive_case_compare(sanspath, Filename) == true);
        errs += (rf_insensitive_case_compare(sanspath, filename) == false);
        
        std::cerr << sanspath << std::endl;
        std::string sansext = rfStripExtension (pathy);
      
        
        errs += (rf_sensitive_case_compare(sansext, Filename) == true);
        errs += (rf_insensitive_case_compare(sansext, rfmovfile) == false);
                
        
        std::cerr << sansext << std::endl;        
        std::string ext = rfGetExtension (pathy);
        
        errs += (rf_sensitive_case_compare(ext, dotfileext) == true);
        errs += (rf_insensitive_case_compare(ext, fileext) == false) ;
        
        errs += (rf_ext_is_rfymov(pathy) == false);
        errs += (rf_ext_is_rfymov(filename) == false);
        errs += (rf_ext_is_rfymov(Filename) == false);
        
        return errs;        
    }
    
private:

};

#endif /* __UT_AFFINE_H */
