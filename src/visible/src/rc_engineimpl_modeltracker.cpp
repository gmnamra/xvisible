/*
 *  rc_engineimpl_modeltracker.cpp
 *  visible
 *
 *  Created by arman on 6/27/09.
 *  Copyright 2009Reify Corporation . All rights reserved.
 *
 */

#include "rc_engineimpl.h"
#include <rc_visualtarget.h>
#include <textio.hpp>
#include <fstream>
#include <iostreamio.hpp>
#include <file_system.hpp>

inline uint32 rfRgbf( float r, float g, float b )
{ return (0xff << 24) | ((((uint32) (r * 255)) & 0xff) << 16) | ((((uint32) (g * 255)) & 0xff) << 8) | (((uint32) (b * 255)) & 0xff); }

static const rcVisualStyle sRed ( rfRgbf(1.0f, 0.0f, 0.0f ), 2, rc2Fvector( 0.5f, 0.5f ) );
static const rcVisualStyle sGray ( rfRgbf(0.8f, 0.8f, 0.8f ), 2, rc2Fvector( 0.5f, 0.5f ) );
static const rcVisualStyle sAffy ( rfRgbf(0.1f, 1.0f, 0.2f ), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle sBlue ( rfRgbf(0.1f, 0.0f, 1.0f ), 1, rc2Fvector( 0.5f, 0.5f ) );

// Perform motion tracking analysis on a focus area
uint32 rcEngineImpl::modelTracker( rcEngineFocusData* focus,
                                     const vector<rcWindow>& focusImages,
                                     int32 cacheSz )
{
	
    double statusUpdateInterval = 1.5;
    // Create template if you need to
    model2use ();
    
    rcWindow sampled_model (_model2use.width(), _model2use.height (), _model2use.depth());
    rfGaussianConv (_model2use, sampled_model, 9);
    rcVisualSegmentCollection visualModel;
    rfImageGradientToVisualSegment (sampled_model, visualModel);
    
    // Create a grabber to read the image vector
    rcVectorGrabber imageGrabber( focusImages, cacheSz );
        
    if ( ! imageGrabber.isValid()  || ! _model2use.isBound () ) 
    {
        strstream s;
        std::string cause = " Does not exist ";
        s << imageGrabber.getInputSourceName() << " Pre Defined Template " << cause << std::ends;
        _observer->notifyError( s.str() );
        s.freeze( false );
        cerr << s.str() << endl;
    }
	
    bool displayMotionVectors = getSettingValue( cAnalysisMotionTrackingMotionVectorDisplaySettingId );
    bool displayBodies = getSettingValue( cAnalysisMotionTrackingBodyVectorDisplaySettingId );
    createGraphicsWriters( displayMotionVectors, displayBodies, false, false );
	
    rcEngineProgress progressIndicator( this, _observer, "Target Tracking...%.2f%% complete", statusUpdateInterval );
    progressIndicator.progress( 0.0 );

    rcRect focusRect = focus->focusRect();	
    const rcIPair pres (3,1); // pyramid resolution
    const rcRadian zr (0.0);
    rc2Dvector translation (0.0, 0.0);
    rcMatrix_2d mtt (zr, rcDPair ((double) pres.x(), (double) pres.y()));
    rc2Xform mx (mtt, translation);

	
    vector<rcWindow>::const_iterator imgItr = focusImages.begin ();

    focus->setTrackStartTimes( imgItr->timestamp ().secs() );
    rcWindow corr_space;	
    rcWindow model2use, image2use, tmp2use;
    int32 smoothKernelWidth = rmMax (pres.x() , pres.y()) * 2 - 1; // same as 2*res - 1. 	
    if (pres.x() == 1 && pres.y() == 1)
    {
        image2use = *imgItr;
        model2use = _model2use;
    }
    else
    {
        // Generate Low resolution images and process the first image
        rcWindow tmp (imgItr->width(), imgItr->height(), imgItr->depth());
        tmp2use = tmp;
        rfGaussianConv (*imgItr, tmp2use, smoothKernelWidth);
        image2use= rfPixelSample (tmp2use, pres.x(), pres.y());
        rcWindow sampled_model (_model2use.width(), _model2use.height (), _model2use.depth());
        rfGaussianConv (_model2use, sampled_model, smoothKernelWidth);
        model2use = rfPixelSample (sampled_model, pres.x(), pres.y());
    }
    
    // Locate the target in the first image
    // @todo Run on the first image @todo remove assumption that the first image has to contain valid
    rcFindList firstFind;
    rfImageFind (image2use, model2use, firstFind, corr_space);
    rc2Fvector startp = mx.mapPoint (firstFind.begin()->interpolated ());
	
	// Setup a restricted window for future runs +/- modelwidth	
		rcIPair limitTopLeft ((int32) (firstFind.begin()->interpolated().x() - model2use.width() / 2), model2use.height());
		rcIPair limitSize (model2use.width()*2, image2use.height() - model2use.height());
		rc2Fvector limitTL ((float) limitTopLeft.x(), (float) limitTopLeft.y());
		mx.trans (mx.mapPoint (limitTL));
	
    std::cout << "[ @ " << imgItr->timestamp ().secs() << " + " << startp.x() << 
      " , " << startp.y() << " ]: " <<  firstFind.begin()->score() << 
		" limit " << limitTopLeft.x() << " , " << limitTopLeft.y() << " -- " << 
		limitSize.x() << " , " << limitSize.y() << 	std::endl;
	
    // Assume we are tracking one object
    rc2Fvector anch = startp;
    rcFRect box (anch.x(), anch.y(), (float) _model2use.width(), (float) _model2use.height());

    // Setup Writer
    // Write the track out
    rcScalarWriter* sWriter = focus->tipDistanceWriter();
    rcWriter* speedwriter = sWriter->down_cast();
    std::string name = speedwriter->getName();
    std::string nameWithOptions = std::string("Instant Speed");
    speedwriter->setName( nameWithOptions.c_str() );

    rcStatistics xstats, rstats;
    bool enableMonitor = false;
    
    for (float pct=0.0f; imgItr != focusImages.end(); imgItr++, pct+= 100.0f)
    {
        // After 6 data points turn on stats monitoring
        if (!enableMonitor && xstats.n() > 6 && rstats.n() > 6) enableMonitor = true;
        
        if ( progressIndicator.progress( pct /(focusImages.size())))
        {
            // Abort Update elapsed time to refresh display
            _observer->notifyStatus( "Analysis stopped" );
            return 0;
        }
			
        rcFindList finds;
        if (pres.x() == 1 && pres.y() == 1) image2use = *imgItr;
        else
        {
            rmAssert (tmp2use.isBound());
            rfGaussianConv (*imgItr, tmp2use, smoothKernelWidth);
            rfPixelSample (tmp2use, image2use, pres.x(), pres.y());
        }
			
				rcWindow window2use (image2use, limitTopLeft.x(), limitTopLeft.y(), limitSize.x(), limitSize.y());	
        rfImageFind (window2use, model2use, finds, corr_space);
        rc2Fvector newp = mx.mapPoint (finds.begin()->interpolated());  
      
				// Validate the new point
        int x_error = std::abs (100 * ((startp.x() - xstats.mean() ) / xstats.stdDev ()));
			
				// No error if corr values go over mean. 
	int r_error = (100 * ((finds.begin()->score() - rstats.mean() ) / rstats.stdDev ()));
				r_error = r_error >= 0 ? 0 : -r_error;

				// Add this data point after we have read the stats so far
        xstats.add (newp.x());
        rstats.add (finds.begin()->score());

				float distance (0.0f);
				bool found = ! (enableMonitor &&  r_error > 270);
				distance = newp.distance (startp);				
				if ( found )
					{
						startp = newp;
					}

			
				std::cerr << imgItr->timestamp ().secs() << "," << startp.x() << "," << startp.y() << "," <<  finds.begin()->score() << "," << distance; 
        if (enableMonitor) std::cerr << "e: " << x_error << " " << r_error << " ";				
				std::cerr << std::endl;
			
        sWriter->writeValue(  imgItr->frameBuf()->timestamp(), focus->focusRect(), distance);        
        postTrackingResults( focus, finds, found, focusRect, mx, imgItr->frameBuf()->timestamp(), visualModel);

    }

    _observer->notifyTime( getElapsedTime() );
	
    return focusImages.size();
}



// Produce cell-specific results
void rcEngineImpl::postTrackingResults( rcEngineFocusData* focus,
                                        rcFindList& locs, bool found,
                                        const rcRect& focusRect, const rc2Xform& mx, 
                                        const rcTimestamp& curTimeStamp, const rcVisualSegmentCollection& vism)
                                      
{
    const rc2Fvector focusOffset( static_cast<int>(focusRect.x()),
                                  static_cast<int>(focusRect.y()) ); // Origin offset
	
    rcRect boundingBox = focusRect;  // Same as analysis area for now
    rc2Fvector  lr ((float) _model2use.width(), (float) _model2use.height());    
    rc2Fvector  center = lr / 2.0f;
    
    rcFindList::const_iterator loc;
    loc = locs.begin();    
    rc2Fvector interp = loc->interpolated ();
    interp = mx.mapPoint (interp);

    // Move the model to visually overlap the image accordring to the registration results
    rcVisualSegmentCollection null_segs;        
    rcVisualSegmentCollection segs = (! found ) ? null_segs : vism;
    rcVisualSegmentCollection::iterator sItr = segs.begin();
    for (; sItr != segs.end(); sItr++) *sItr += interp;

    segs.push_back (sAffy);
    segs.push_back (rcVisualRect (interp, interp+lr));
		segs.push_back (found ? sRed : sBlue);
    segs.push_back (rcVisualEllipse (interp+center, center));

    if (_bodyVectorWriter != 0)
    {
        rcVisualGraphicsCollection graphics (rcStyle (), segs);
        const rc2Fvector PixelOffset((float)0.5, (float)0.5);
        offsetGraphics( graphics, focusOffset + PixelOffset);
        _bodyVectorWriter->writeValue( curTimeStamp, focusRect, graphics );
    }

}


