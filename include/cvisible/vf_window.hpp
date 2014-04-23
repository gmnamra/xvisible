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
    
    static bool is_legacy_Visible_output (csv::rows_type& rows, int& offset)
    {
        if (rows.size () < 1) return false;
        const csv::row_type& row = rows[0];
        bool is_visible = false;
        for (int i = 0; i < row.size(); i++)
        {
            if (row[i].contains ("Visible")) { is_visible = true; break; }
        }
        if (! is_visible ) return is_visible;
        int last_row = -1;
        for (int rr = 1; rr < rows.size(); rows++)
        {
            const csv::row_type& row = rows[rr];
            for (int i = 0; i < row.size(); i++)
            {
                if (row[i].contains ("seconds")) { last_row = i; break; }
            }
            if (last_row > 0) break;
        }
        offset = last_row;        
    }
    
    
    static bool ( const fs::path &csv_file, std::vector<vector<float> >& datas, bool force_all_numeric)
    {
        std::ifstream istream (csv_file.string());
        csv::rows_type rows = csv::to_rows (istream);
        int start_row = 0;
        bool is_visible_legacy = force_all_numeric ? false : vf_utils::is_legacy_Visible_output (rows, start_row);
        datas.resize (0);            

        if ( ! is_visible_legacy )
        // if is_visible is true start_row has to be -1 or 0 -> rows.size() - 1
          
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
        
        if (datas.size())
        {
            m_results.resize(4);
            
            for (uint i = 0; i < datas.size (); i++)
            {
                const vector<float>& vc = datas[i];
                for (uint cc=0; cc<4;cc++)
                    m_results[cc].push_back(vc[cc]);
            }
            vector<float>& column = m_results[3];
    
    
};


#endif

