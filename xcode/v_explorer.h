//
//  v_explorer.h
//  vExplorer
//
//  Created by Arman Garakani on 1/4/15.
//
//


#ifndef vExplorer_v_explorer_h
#define vExplorer_v_explorer_h

#include <map>
#include <string>
#include "vf_utils.hpp"

using namespace vf_utils;


class gl_format_string_from_numeric : public vf_utils::singleton<gl_format_string_from_numeric>
{
public:
    
    bool has_description (int val)
    {
        container_t::iterator index = m_map.find(val);
        return index != m_map.end();
    }
    
    std::string description_from_numeric (int val)
    {
        std::string empty;
        
        container_t::iterator index = m_map.find(val);
        if (index != m_map.end())
            return m_map[val];
        return empty;
    }
    
private:
    typedef std::pair<int, std:;string> pair_t;
    typedef std::map<pair_t::first_type,pair_t::second_type> container_t;
    
    gl_format_string_from_numeric ()
    {
        m_map[0x803B] = std::string("GL_ALPHA4_EXT");
        m_map[0x803C] = std::string("GL_ALPHA8_EXT");
        m_map[0x803D] = std::string("GL_ALPHA12_EXT");
        m_map[0x803E] = std::string("GL_ALPHA16_EXT");
        m_map[0x803F] = std::string("GL_LUMINANCE4_EXT");
        m_map[0x8040] = std::string("GL_LUMINANCE8_EXT");
        m_map[0x8041] = std::string("GL_LUMINANCE12_EXT");
        m_map[0x8042] = std::string("GL_LUMINANCE16_EXT");
        m_map[0x8043] = std::string("GL_LUMINANCE4_ALPHA4_EXT");
        m_map[0x8044] = std::string("GL_LUMINANCE6_ALPHA2_EXT");
        m_map[0x8045] = std::string("GL_LUMINANCE8_ALPHA8_EXT");
        m_map[0x8046] = std::string("GL_LUMINANCE12_ALPHA4_EXT");
        m_map[0x8047] = std::string("GL_LUMINANCE12_ALPHA12_EXT");
        m_map[0x8048] = std::string("GL_LUMINANCE16_ALPHA16_EXT");
        m_map[0x8049] = std::string("GL_INTENSITY_EXT");
        m_map[0x804A] = std::string("GL_INTENSITY4_EXT");
        m_map[0x804B] = std::string("GL_INTENSITY8_EXT");
        m_map[0x804C] = std::string("GL_INTENSITY12_EXT");
        m_map[0x804D] = std::string("GL_INTENSITY16_EXT");
        m_map[0x804E] = std::string("GL_RGB2_EXT");
        m_map[0x804F] = std::string("GL_RGB4_EXT");
        m_map[0x8050] = std::string("GL_RGB5_EXT");
        m_map[0x8051] = std::string("GL_RGB8_EXT");
        m_map[0x8052] = std::string("GL_RGB10_EXT");
        m_map[0x8053] = std::string("GL_RGB12_EXT");
        m_map[0x8054] = std::string("GL_RGB16_EXT");
        m_map[0x8055] = std::string("GL_RGBA2_EXT");
        m_map[0x8056] = std::string("GL_RGBA4_EXT");
        m_map[0x8057] = std::string("GL_RGB5_A1_EXT");
        m_map[0x8058] = std::string("GL_RGBA8_EXT");
        m_map[0x8059] = std::string("GL_RGB10_A2_EXT");
        m_map[0x805A] = std::string("GL_RGBA12_EXT");
        m_map[0x805B] = std::string("GL_RGBA16_EXT");
        m_map[0x805C] = std::string("GL_TEXTURE_RED_SIZE_EXT");
        m_map[0x805D] = std::string("GL_TEXTURE_GREEN_SIZE_EXT");
        m_map[0x805E] = std::string("GL_TEXTURE_BLUE_SIZE_EXT");
        m_map[0x805F] = std::string("GL_TEXTURE_ALPHA_SIZE_EXT");
        m_map[0x8060] = std::string("GL_TEXTURE_LUMINANCE_SIZE_EXT");
        m_map[0x8061] = std::string("GL_TEXTURE_INTENSITY_SIZE_EXT");
        m_map[0x8062] = std::string("GL_REPLACE_EXT");
        m_map[0x8063] = std::string("GL_PROXY_TEXTURE_1D_EXT");                            
        m_map[0x8064] = std::string("GL_PROXY_TEXTURE_2D_EXT");                            
        m_map[0x8065] = std::string("GL_TEXTURE_TOO_LARGE_EXT");                           
    }
 
  
    container_t m_map;
    bool self_test ()
    {
        bool ok = ! m_map.empty();
        
    }
};



#endif

