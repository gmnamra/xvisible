/*
 *  rc_engineimpl_modeltracker.cpp
 *  visible
 *
 *  Created by arman on 6/27/09.
 *  Copyright 2009Reify Corporation . All rights reserved.
 *
 */

#include "rc_engineimpl.h"
#include <textio.hpp>
#include <fstream>
#include <iostreamio.hpp>
#include <file_system.hpp>
#include <rc_find.hpp>


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
	model2use ();
    double statusUpdateInterval = 1.5;
        // Create a grabber to read the image vector
    rcVectorGrabber imageGrabber( focusImages, cacheSz );

    if ( ! imageGrabber.isValid() )
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
    const rcRadian zr (0.0);
    rc2Dvector translation (0.0, 0.0);
    rcMatrix_2d mtt (zr, rcDPair (1.0,1.0) );
    rc2Xform mx (mtt, translation);


    vector<rcWindow>::const_iterator imgItr = focusImages.begin ();

    focus->setTrackStartTimes( imgItr->timestamp ().secs() );
    rcWindow corr_space;
    rcWindow model2use (_rect2use.width(), _rect2use.height () );
    rcWindow model_win (*imgItr, _rect2use.origin().x(),_rect2use.origin().y(), _rect2use.width(), _rect2use.height () );
    model2use.copyPixelsFromWindow (model_win);

    float horz_trim = _rect2use.width () / 2.0f;
    float vert_trim = _rect2use.height () / 2.0f;

    rcWindow sampled_model (model2use.width(), model2use.height (), model2use.depth());
    rfGaussianConv (model2use, sampled_model, 9);
    rcVisualSegmentCollection visualModel;
    rfImageGradientToVisualSegment (sampled_model, visualModel);


        // Locate the target in the first image
        // @todo Run on the first image @todo remove assumption that the first image has to contain valid
    cv::Mat fixed (model2use.width(), model2use.height(), CV_8UC1, model2use.rowPointer(0));
    rcFRect fixedroi (0, 0,  model2use.width(), model2use.height() );
    cv::Mat moving ( imgItr->height(),imgItr->width(),CV_8UC1, (uint8*)(imgItr->rowPointer(0)), imgItr->rowUpdate());
    rcFRect movingroi (_rect2use.ul().x(), _rect2use.ul().y(), _rect2use.width(), _rect2use.height());
    movingroi = movingroi.trim(-3);
    rcLocation anch = rcFind (fixed, fixedroi , moving, movingroi);
    
    rcStatistics xstats, rstats;


    // Add this data point
    xstats.add (anch.x());
    rstats.add (anch.score());

    if (! anch.haveCondition(rcLocation::eUnknown) )
    {
            // Setup a restricted window for future runs +/- modelwidth
        std::cout << "[" << _rect2use.ul().x() << "," << _rect2use.ul().y() << "]" << "??" << anch.x() << "," << anch.y() << std::endl;
        rcFRect pos (anch.x(), anch.y(), model2use.width(), model2use.height());
        pos.trim (-horz_trim, -horz_trim, -vert_trim, -vert_trim);
	    rc2Fvector startp = anch.pos ();

            // Setup Writer
            // Write the track out
        rcScalarWriter* sWriter = focus->tipDistanceWriter();
        rcWriter* speedwriter = sWriter->down_cast();
        std::string name = speedwriter->getName();
        std::string nameWithOptions = std::string("Instant Speed");
        speedwriter->setName( nameWithOptions.c_str() );


        for (float pct=0.0f; imgItr != focusImages.end(); imgItr++, pct+= 100.0f)
        {
                // After 6 data points turn on stats monitoring
                //            if (!enableMonitor && xstats.n() > 6 && rstats.n() > 6) enableMonitor = true;

            if ( progressIndicator.progress( pct /(focusImages.size())))
            {
                    // Abort Update elapsed time to refresh display
                _observer->notifyStatus( "Analysis stopped" );
                return 0;
            }

            cv::Mat moving ( imgItr->height(),imgItr->width(),CV_8UC1, (uint8*)(imgItr->rowPointer(0)), imgItr->rowUpdate());
            rcFRect movingroi (_rect2use.ul().x(), _rect2use.ul().y(), _rect2use.width(), _rect2use.height());
            movingroi.trim(-3);
            rcLocation loc = rcFind (fixed, fixedroi , moving, movingroi);
            rc2Fvector newp = mx.mapPoint (loc);
                // Add this data point
            xstats.add (anch.x());
            rstats.add (anch.score());

				// Validate the new point
            int x_error = (xstats.n() > 1) ? std::abs (100 * ((anch.x() - xstats.mean() ) / xstats.stdDev ())) : 0;

				// No error if corr values go over mean.
            int r_error = (rstats.n() > 1) ? (100 * ((loc.score() - rstats.mean() ) / rstats.stdDev ())) : 0;
            r_error = r_error >= 0 ? 0 : -r_error;


            float distance (0.0f);
                //     bool found = ! (enableMonitor &&  r_error > 270);
            distance = newp.distance (startp);
            if ( true )
			{
				startp = newp;
			}


            std::cerr << imgItr->timestamp ().secs() << "," << startp.x() << "," << startp.y() << "," <<  loc.score() << "," << distance;
            std::cerr << "e: " << x_error << " " << r_error << " ";	std::cerr << std::endl;

            sWriter->writeValue(  imgItr->frameBuf()->timestamp(), focus->focusRect(), distance);
            postTrackingResults( focus, loc, true, focusRect, mx, imgItr->frameBuf()->timestamp(), visualModel);

        }
    }
    _observer->notifyTime( getElapsedTime() );

    return focusImages.size();
}



    // Produce cell-specific results
void rcEngineImpl::postTrackingResults( rcEngineFocusData* focus,
                                       rcLocation& loc, bool found,
                                       const rcRect& focusRect, const rc2Xform& mx,
                                       const rcTimestamp& curTimeStamp, const rcVisualSegmentCollection& vism)

{
    rmUnused(focus);
    const rc2Fvector focusOffset( static_cast<int>(focusRect.x()),
                                 static_cast<int>(focusRect.y()) ); // Origin offset

   // rcRect boundingBox = focusRect;  // Same as analysis area for now
    rc2Fvector  lr ((float) _rect2use.width(), (float) _rect2use.height());
    rc2Fvector  center = lr / 2.0f;

    rc2Fvector interp = loc.pos ();
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


