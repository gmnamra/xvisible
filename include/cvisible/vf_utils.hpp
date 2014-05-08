#ifndef __VF_UTILS__
#define __VF_UTILS__

#include <cinder/Channel.h>
#include <cinder/Area.h>

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <vfi386_d/rc_window.h>
#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <vfi386_d/rc_fileutils.h>
#include "sshist.hpp"

#include <fstream>

using namespace ci;
using namespace std;



namespace vf_utils
{
    
    static rcSharedFrameBufPtr newFromChannel8u ( ci::Channel8u& onec )
    {
        return rcSharedFrameBufPtr (new rcFrame (reinterpret_cast<char*>(onec.getData()), 
                                                 (int32) onec.getRowBytes (),
                                                 (int32) onec.getWidth (), 
                                                 (int32) onec.getHeight (), rcPixel8, true));
        
    }
    
    
    static ci::Channel8u*  newCiChannel (const rcSharedFrameBufPtr& sf)
    {
        ci::Channel8u* ch8 = new ci::Channel8u ( sf->width(), sf->height () );
        
        const cinder::Area clippedArea = ch8->getBounds();
        int32_t rowBytes = ch8->getRowBytes();
        
        for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
        {
            uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
            const uint8 *srcPtr = sf->rowPointer(y);
            for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
            {
                *dstPtr++ = *srcPtr++;
            }
        }	
        
        return ch8;
    }
    
    
    static const ci::Channel8u* newChannel8uFrom (const rcWindow& w)
    {
        if (!w.isBound()) return 0;
        ci::Channel8u* ch8 = new ci::Channel8u ( w.width(), w.height () );
        
        const cinder::Area clippedArea = ch8->getBounds();
        int32_t rowBytes = ch8->getRowBytes();
        
        for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
        {
            uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
            const uint8 *srcPtr = w.rowPointer(y);
            for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
            {
                *dstPtr++ = *srcPtr++;
            }
        }
        
        return ch8;
    }
    
    
    
    static bool ImageExport2JPG (const rcWindow& image, std::string filePathName)
    {
        std::string extn = rfGetExtension (filePathName);
        if (extn != std::string("jpg"))
            return false;
        
        const ci::Channel8u* chcopy = newChannel8uFrom (image);
        ci::writeImage (filePathName, ImageSourceRef( *chcopy ) );
        return rfFileExists (filePathName);
        
    }
    
    
    namespace csv
    {
        
        typedef std::vector<std::string> row_type;
        typedef std::vector<row_type> rows_type;
        
        // c++11 
        static bool is_number(const std::string& s)
        {
            return !s.empty() && std::find_if(s.begin(), 
                                              s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
        }
        
        //! Convert an input stream to csv rows.
        
        rows_type to_rows(std::ifstream &input)
        {
            if (!input.is_open())
                perror("error while opening file");
            
            csv::rows_type rows;
            std::string line;
            //for each line in the input stream
            while (std::getline(input, line))
            {
                boost::trim_if(line, boost::is_any_of("\t\r ")); 
                csv::row_type row;                
                boost::split(row, line, boost::is_any_of("\t\r ,"));
                
                rows.push_back(row);
            }
            if (input.bad())
                perror("error while reading file");
            
            // Handle cases where file conained one long delimated sequence of numbers
            // which were parsed in to 1 row and many columns
            if ( ! rows.empty () && (rows[0].size() / rows.size() ) == rows[0].size() )
            {
                int columns = rows[0].size ();
                assert (rows.size () == 1);
                rows_type crows;
                const row_type& col = rows[0];
                for (int i = 0; i < columns; i++) 
                {
                    row_type row (1, col[i]);
                    crows.push_back (row);
                }
                
                rows = crows;
            }
            
            return rows;
        }
        
        // Returns starting row of numeric data as a positive number if it is a legacy visible file or 
        // -1 * number of rows if it is not
        static int is_legacy_visible_output (csv::rows_type& rows)
        {
            if (rows.size () < 1) return false;
            const csv::row_type& row = rows[0];
            bool is_visible = false;
            for (int i = 0; i < row.size(); i++)
            {
                if (row[i].find ("Visible") != string::npos ) { is_visible = true; break; }
            }
            if (! is_visible ) return -1 * rows.size ();
            int last_row = -1;
            for (int rr = 1; rr < rows.size(); rr++)
            {
                const csv::row_type& row = rows[rr];
                for (int i = 0; i < row.size(); i++)
                {
                    if (row[i].find ("seconds") != string::npos )
                    { 
                        last_row = rr + ( rr < (rows.size() - 1) ? 1 : 0) ; break;
                    }
                }
                if (last_row > 0) break;
            }
            return last_row; 
        }
        
        // Returns starting row of numeric data as a positive number if it is a legacy visible file or 
        // -1 * number of rows if it is not
        static int file_is_legacy_visible_output (std::string& fqfn)
        {
            std::ifstream istream (fqfn);
            csv::rows_type rows = csv::to_rows (istream);
            return is_legacy_visible_output ( rows);
            
        }    
        
        
        static bool csv2vectors ( const std::string &csv_file, std::vector<vector<float> >& m_results, bool only_visible_format, bool columns_if_possible, bool verbose = false)
        {
            {
                m_results.resize (0);
                std::ifstream istream (csv_file);
                csv::rows_type rows = csv::to_rows (istream);
                int visible_row = is_legacy_visible_output (rows);
                int start_row = visible_row < 0 ? 0 : visible_row;
                if ( only_visible_format && visible_row < 0 ) return false;
                if (visible_row < 0 ) assert (start_row >= 0 && start_row < rows.size ());
                
                
                
                std::vector<vector<float> > datas;
                std::vector<vector<float> > column_wise;
                datas.resize (0);
                column_wise.resize (0);
                
                sshist  column_width (16);
                
                // Get All rows 
                for (int rr = start_row; rr < rows.size(); rr++)
                {
                    const csv::row_type& row = rows[rr];
                    vector<float> data;
                    for (int t = 0; t < row.size(); t++) 
                    {
                        char *end_ptr;
                        float f = strtof(row[t].c_str(), &end_ptr);
                        if (end_ptr == row[t].c_str()) continue;
                        data.push_back (f);
                    }
                    datas.push_back(data);
                    column_width.add (data.size());  
                }
                
                sshist::sorted_container_t shist;
                column_width.get_value_sorted (shist);
                if (verbose)
                {
                    for (auto pt : shist) std::cout << "[" << pt.first << "]: " << pt.second << std::endl;
                }
                
                if (! shist.empty () && datas.size() && columns_if_possible) // if there were all same number of columns
                {
                    uint32 cw = shist.front().first;
                    column_wise.resize(cw);
                    for (uint i = 0; i < datas.size (); i++)
                    {
                        const vector<float>& vc = datas[i];
                        if (vc.size () != cw) continue;
                        
                        for (uint cc=0; cc<cw;cc++)
                            column_wise[cc].push_back(vc[cc]);
                    }
                }
                
                if (!column_wise.empty ())
                {
                    assert (columns_if_possible);
                    m_results = column_wise;
                }
                else if ( ! datas.empty () )
                {
                    m_results = datas;
                }
                
                return ! m_results.empty ();
            }
            
        }
    }
    
};


#endif

