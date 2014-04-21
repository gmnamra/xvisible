#ifndef __UT_FILE_H
#define __UT_FILE_H


#include <rc_unittest.h>
#include <rc_fileutils.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <boost/foreach.hpp>
#include <cstdio>
#include <rc_stats.h>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

class UT_fileutils : public rcUnitTest {
public:
    
    UT_fileutils(std::string py) : test_data_path (py) {}
    ~UT_fileutils() {}
    
    virtual uint32 run()
    {
        int errs = 0;
        
        errs += (folder_exists (test_data_path) == false);
        
        std::string pathy ("/Users/arman/boo/foo/test_o_0.rfymov"); // made up name. File does not exist
        std::string rfmovfile ("test_o_0");
        std::string filename ("test_o_0.rfymov");
        std::string Filename ("test_O_0.rfymov");        
        std::string fileext ("rfymov");
        std::string dotfileext (".rfymov");        
        std::string dormovfileext (".mov");   
        std::string csvfile ("test.csv");

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

        std::string csv_filename = create_filespec (test_data_path, csvfile);
        errs += (file_exists(csv_filename) == false);

        std::ifstream istream (csv_filename.c_str());        
        csv::rows_type rows = csv::to_rows (istream);
        errs += ((rows.size() == 3310) == false);
        
        std::vector<vector<float> > datas;
        BOOST_FOREACH(const csv::row_type &row, rows)
        {
            vector<float> data (4);
            if (row.size () != 4) continue;
            int c=0;
            for (int i = 0; i < 4; i++) 
            {
                std::istringstream iss(row[i]);
                iss >> data[i];
                c++;
            }
            if (c != 4) continue;
            if (rfSum(data) == 0) continue;
            datas.push_back(data);
        }
        
        //        std::cout << rfMean(datas) << std::endl;

        // ignoring data 0 lines, top 14 lines of Visible legacy csv output should be gone. 
        uint32 expected = rows.size() - 14;
        errs += ( ( datas.size () == expected ) == false);
        
        return errs;        
    }
    
private:
    std::string test_data_path;

};

#endif /* __UT_AFFINE_H */
