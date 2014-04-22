#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include "cinder/Perlin.h"
#include "cinder/Timeline.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/qtime/Quicktime.h"
#include "Resources.h"
#include "ciUI.h"
#include <vector>
#include <vfi386_d/rc_fileutils.h>
#include <vfi386_d/rc_stats.h>
#include <boost/foreach.hpp>


#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public AppBasic
{    
public:
    void prepareSettings( Settings *settings ); 
    Renderer* prepareRenderer(); 
    void gui_configuration (float);
    void setup();
    void update(); 
    void draw();
    void shutDown();
    
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
    
    void guiEvent(ciUIEvent *event);    
    
    void setupGUI1(); 
    void setupGUI2(); 
    void setupGUI3(); 
    
    void loadMovieFile( const fs::path &moviePath );
    void loadCSVFile (const fs::path &,  std::vector<vector<float> >& );
    void fileDrop( FileDropEvent event );
    
    //Vars 
    float vtick_scaled;
    float bgColorR; 
    float bgColorG; 
    float bgColorB;     
    
    Perlin myPerlin;
    gl::Texture mImage;
    Surface mImageSurface; 
    qtime::MovieGl m_movie;    
    bool m_movie_valid, m_result_valid;
    int m_fc;
    
    std::vector<float> buffer;
    ciUIMovingGraph *mvg; 
    ciUICanvas *gui;  
    ciUICanvas *gui1;  
    ciUICanvas *gui2;  
    ciUIImage  *m_screen;
    ciUIImageSampler *m_sampler;
    
    float menuWidth; 
    float menuHeight; 
    float widgetWidth, widgetHeight;
    float dim; 
    std::string movieFileName, mLastPath;
    std::string resultFileName;
    
    vector<string> recent_movie_files;
    std::string recent_movie_directory;
    std::string recent_result_directory;    
    vector<string> recent_result_files;
    vector<Rectf> gui_rects;
    vector< vector<float> > m_results;
};

Renderer* _TBOX_PREFIX_App::prepareRenderer()
{
    return new RendererGl( RendererGl::AA_MSAA_2 ); 
}

void _TBOX_PREFIX_App::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 768, 1024 );
    settings->setFrameRate( 60.0f );
}

void _TBOX_PREFIX_App::gui_configuration (float image_aspect_ratio)
{
    menuWidth = getWindowWidth(); 
    menuHeight = getWindowHeight();
    widgetWidth = menuWidth - CI_UI_GLOBAL_WIDGET_SPACING;
    widgetHeight = widgetWidth / image_aspect_ratio;
    dim = menuWidth*.0675;
        
    gui_rects.resize (3);
    Rectf g (0, 0, widgetWidth, menuHeight / 4.0f);
    gui_rects[0] = g;
    Rectf g2 (0, 0, widgetWidth, menuHeight / 2);    
    Vec2f t (0, g.getHeight());
    g2 += t;
    gui_rects[1] = g2;
    Vec2f t2 (0, g.getHeight() + g2.getHeight());
    g += t2;
    gui_rects[2] = g; 
    
    gui = new ciUICanvas (gui_rects[0].getUpperLeft().x, gui_rects[0].getUpperLeft().y, gui_rects[0].getWidth(), gui_rects[0].getHeight());
    
    gui1 = new ciUICanvas (gui_rects[1].getUpperLeft().x, gui_rects[1].getUpperLeft().y, gui_rects[1].getWidth(), gui_rects[1].getHeight());
    gui2 = new ciUICanvas (gui_rects[2].getUpperLeft().x, gui_rects[2].getUpperLeft().y, gui_rects[2].getWidth(), gui_rects[2].getHeight());    
    
}
void _TBOX_PREFIX_App::setup()
{
    gl::enableAlphaBlending(); 
	setFpsSampleInterval(1.0); 
    
    mImage = loadImage( loadResource( SAMPLE_IMAGE ) ); 
    mImageSurface = loadImage ( loadResource( SAMPLE_IMAGE ) );
    
    gui_configuration (mImage.getAspectRatio () );
 
    buffer.resize (128);
    bgColorR = 0.67f;     
    bgColorG = 0.67f;     
    bgColorB = 0.67f; 
    vtick_scaled = 0.0;
    m_fc = -1;
    movieFileName = std::string ("");
    m_movie_valid = false;
    const fs::path& exec_path = getAppPath();
    std::cout << "Application Executive @ " << exec_path.string () << std::endl;
    
    setupGUI1(); 
    setupGUI2(); 
    setupGUI3();
    
}

void _TBOX_PREFIX_App::update()
{
    for(int i = 0; i < buffer.size(); i++)
    {
        buffer[i] = myPerlin.noise((float)i/buffer.size());
    }

    gui2->update();    
    mvg->addPoint(getAverageFps());
}

void _TBOX_PREFIX_App::draw()
{
	gl::setMatricesWindow( getWindowSize() );
	gl::clear( Color( bgColorR, bgColorG, bgColorB ) ); 
    if (m_movie_valid)
        mImage = m_movie.getTexture ();
    if (m_movie_valid && m_fc > 0 )
    {
        int frame_index = vtick_scaled * m_fc;
        m_movie.seekToFrame (frame_index);
    }
    
    gui->draw();
    gui1->draw();
    gui2->draw();    
}

void _TBOX_PREFIX_App::shutDown()
{
    delete gui; 
    delete gui1; 
    delete gui2;     
}

void _TBOX_PREFIX_App::keyDown( KeyEvent event )
{
    if(gui2->hasKeyboardFocus())        
    {
        return; 
    }
    else
    {    
        if( event.getChar() == 'f' )
        {        
            setFullScreen( ! isFullScreen() );
        }
        else if(event.getChar() == 'p')
        {
            gui->setDrawWidgetPadding(true);
            gui1->setDrawWidgetPadding(true);
            gui2->setDrawWidgetPadding(true);        
        }
        else if(event.getChar() == 'P')
        {
            gui->setDrawWidgetPadding(false);
            gui1->setDrawWidgetPadding(false);
            gui2->setDrawWidgetPadding(false);        
        }
        else if(event.getChar() == '1')
        {
            gui->toggleVisible();
        }
        else if(event.getChar() == '2')
        {
            gui1->toggleVisible();        
        }
        else if(event.getChar() == '3')
        {
            gui2->toggleVisible();        
        }
        else if(event.getChar() == 's')
        {
            gui->saveSettings("ciUIGUISettings.xml");
            gui1->saveSettings("ciUIGUI1Settings.xml");
            gui2->saveSettings("ciUIGUI2Settings.xml");        
        }
        else if(event.getChar() == 'l')
        {
            gui->loadSettings("ciUIGUISettings.xml");        
            gui1->loadSettings("ciUIGUI1Settings.xml");        
            gui2->loadSettings("ciUIGUI2Settings.xml");                
        }
    }
}

void _TBOX_PREFIX_App::keyUp( KeyEvent event )
{
}

void _TBOX_PREFIX_App::guiEvent(ciUIEvent *event)
{
    const std::string& name = event->widget->getName();
    
    if(name == "Load Experiment Movie" )
    {
        fs::path moviePath = getOpenFilePath ();
        m_movie_valid = false;
        if ( ! moviePath.empty () )
        {
            std::cout << moviePath.string ();
            loadMovieFile (moviePath);
        }
    }
    
    if(name == "Load Experiment Result" )
    {
        fs::path resultPath = getOpenFilePath ();
        m_result_valid = false;
        if ( ! resultPath.empty () )
        {
            std::cout << resultPath.string ();
            loadCSVFile (resultPath, m_results);
        }
    }    
    
    if(name == "VTICK")
    {
        ciUISlider *slider = (ciUISlider *) event->widget; 
        vtick_scaled = slider->getScaledValue(); 
    }
}

void _TBOX_PREFIX_App::setupGUI1()
{
    float w = menuWidth; 
    float spacerHeight = 2; 
    //    gui = new ciUICanvas(0,0,menuWidth,menuHeight);
    
    
    gui->addWidgetDown(new ciUILabel("Visible ", CI_UI_FONT_LARGE), CI_UI_ALIGN_RIGHT);
    gui->addWidgetDown(new ciUILabel("Reify Corporation", CI_UI_FONT_SMALL), CI_UI_ALIGN_RIGHT);
  
    gui->addWidgetDown(new ciUISpacer(w, spacerHeight));            
    gui->addWidgetDown(new ciUILabelToggle(false, "Load Experiment Movie", CI_UI_FONT_SMALL)); 
    gui->addWidgetDown(new ciUILabelToggle(false, "Load Experiment Result", CI_UI_FONT_SMALL));
    
      
    //  gui->addWidgetDown(new ciUI2DPad(w, w*.5, Vec2f(w*.5,w*.25), "2DPAD"), CI_UI_ALIGN_LEFT);
    // gui->addWidgetDown(new ciUISpacer(w, spacerHeight));        
    //gui->addWidgetDown(new ciUIRotarySlider(120, 0, 100.0, 50.0, "ROTARY"));
    //  gui->addWidgetRight(new ciUISlider(h,120,0.0,100.0,25.0,"1"));
    // gui->addWidgetRight(new ciUIRangeSlider(h,120,0.0,100.0,50.0,75.0,"5"));    
    
    gui->registerUIEvents(this, &_TBOX_PREFIX_App::guiEvent); 
}

void _TBOX_PREFIX_App::setupGUI2()
{
     float w = widgetWidth; 
    float h = widgetHeight;
    float spacerHeight = 2; 
    vector<float> mbuffer; 
    for(int i = 0; i < buffer.size(); i++)
    {
        buffer[i] = myPerlin.noise((float)i/buffer.size());
        mbuffer.push_back(0);        
    }

    //   gui1 = new ciUICanvas(0,menuHeight,menuWidth,menuHeight);
    gui1->addWidgetDown(m_screen = new ciUIImage(w - CI_UI_GLOBAL_WIDGET_SPACING,h/2 - CI_UI_GLOBAL_WIDGET_SPACING,  &mImage, "IMAGE VIEW"),CI_UI_ALIGN_BOTTOM);
    //    gui1->addWidgetRight(m_sampler = new ciUIImageSampler(w/2 - CI_UI_GLOBAL_WIDGET_SPACING,h/2 - CI_UI_GLOBAL_WIDGET_SPACING, &mImageSurface, "IMAGE SAMPLER"));
  
    gui1->registerUIEvents(this, &_TBOX_PREFIX_App::guiEvent);     
}

void _TBOX_PREFIX_App::setupGUI3()
{
    float w = widgetWidth / 3;
    gui2->addWidgetDown(new ciUIImageSlider(menuWidth - CI_UI_GLOBAL_WIDGET_SPACING, 24, 0, 1.0, vtick_scaled, "..", "VTICK")); 
    
//    gui1->addWidgetDown(new ciUISpacer(w, spacerHeight));   
//    gui1->addWidgetDown(new ciUILabel("WAVEFORM PLOT", CI_UI_FONT_SMALL),CI_UI_ALIGN_BOTTOM);        
//    gui1->addWidgetDown(new ciUIWaveform(w, 30, &buffer[0], buffer.size(), -1.0, 1.0, "WAVEFORM"),CI_UI_ALIGN_BOTTOM);  
//    gui1->addWidgetDown(new ciUISpacer(w, spacerHeight),CI_UI_ALIGN_BOTTOM);   
             
    // gui2 = new ciUICanvas(0,menuHeight+menuHeight,menuWidth,menuHeight);
   
    gui2->registerUIEvents(this, &_TBOX_PREFIX_App::guiEvent);     
  }


void _TBOX_PREFIX_App::loadMovieFile( const fs::path &moviePath )
{
	
	try {
		m_movie = qtime::MovieGl( moviePath );
        m_movie_valid = m_movie.checkPlayable ();
        
        if (m_movie_valid)
        {
            console() << "Dimensions:" <<m_movie.getWidth() << " x " <<m_movie.getHeight() << std::endl;
            console() << "Duration:  " <<m_movie.getDuration() << " seconds" << std::endl;
            console() << "Frames:    " <<m_movie.getNumFrames() << std::endl;
            console() << "Framerate: " <<m_movie.getFramerate() << std::endl;
                        
            //   m_movie.setLoop( true, false );
            //   m_movie.play();
            m_fc = m_movie.getNumFrames ();
        }
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		return;
	}
	
}


void _TBOX_PREFIX_App::loadCSVFile( const fs::path &csv_file, std::vector<vector<float> >& datas)
{
    
    std::ifstream istream (csv_file.string());
    csv::rows_type rows = csv::to_rows (istream);
    datas.resize (0);    

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

    if (datas.size())
    {
        vector< vector<float> > columns;
        columns.resize(4);
        
        for (uint i = 0; i < datas.size (); i++)
        {
            const vector<float>& vc = datas[i];
            for (uint cc=0; cc<4;cc++)
              columns[cc].push_back(vc[cc]);
        }

        gui2->addWidgetDown(new ciUILabel("MOVING PLOT (USING FPS)", CI_UI_FONT_SMALL),CI_UI_ALIGN_BOTTOM);    
        mvg = (ciUIMovingGraph *) gui1->addWidgetDown(new ciUIMovingGraph(menuWidth, 30, columns[1], 128, 0, 120, "MOVING GRAPH"),CI_UI_ALIGN_BOTTOM); 
        m_result_valid = true;
    }
    
}



void _TBOX_PREFIX_App::fileDrop( FileDropEvent event )
{
	for( size_t s = 0; s < event.getNumFiles(); ++s )
		loadMovieFile( event.getFile( s ) );
}



CINDER_APP_BASIC( _TBOX_PREFIX_App, RendererGl )