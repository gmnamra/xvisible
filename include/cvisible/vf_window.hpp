#ifndef __VF_UTILS__
#define __VF_UTILS__

#include <cinder/Channel.h>
#include <cinder/Area.h>

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <vfi386_d/rc_window.h>
#include <vfi386_d/rc_fileutils.h>

using namespace ci;
using namespace std;



struct vf_utils
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
    
  
    // Returns number of rows as a positive number if it is a legacy visible file or negative if it is not
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
    
    // Returns number of rows as a positive number if it is a legacy visible file or negative if it is not
    static int file_is_legacy_visible_output (std::string& fqfn)
    {
        std::ifstream istream (fqfn);
        csv::rows_type rows = csv::to_rows (istream);
        return is_legacy_visible_output ( rows);
        
    }    
    
    static bool csv2vectors ( const std::string &csv_file, std::vector<vector<float> >& m_results, bool force_all_numeric)
    {
        std::ifstream istream (csv_file);
        csv::rows_type rows = csv::to_rows (istream);
        int visible_row = vf_utils::is_legacy_visible_output (rows);
        int start_row = visible_row < 0 ? 0 : visible_row;
        if ( ! force_all_numeric && visible_row < 0 ) return false;
        if (visible_row < 0 ) assert (start_row >= 0 && start_row < rows.size ());
        
        std::vector<vector<float> > datas;
        vector<uint32> column_width;
        
        // Get All rows 
        for (int rr = start_row; rr < rows.size(); rr++)
        {
            const csv::row_type& row = rows[rr];
            if (column_width.empty () || column_width.back() != row.size()) column_width.push_back (row.size());
            vector<float> data;
            data.resize (row.size ());
            for (int t = 0; t < row.size(); t++) 
            {
                std::istringstream iss(row[t]);
                iss >> data[t];
            }
            datas.push_back(data);
        }
        
        if (datas.size() && column_width.size () == 1) // if there were all same number of columns
        {
            uint32 cw = column_width.front();
            m_results.resize(cw);
            for (uint i = 0; i < datas.size (); i++)
            {
                const vector<float>& vc = datas[i];
                for (uint cc=0; cc<cw;cc++)
                    m_results[cc].push_back(vc[cc]);
            }
        }
        else
            m_results = datas;
        
        return true;
    }
    
};
        
        
#endif
        
