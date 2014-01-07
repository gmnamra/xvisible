/*
 *
 *$Id $
 *$Log$
 *Revision 1.67  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.66  2006/01/08 19:20:40  arman
 *added caliper display to debug graphics output
 *
 *Revision 1.65  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.64  2005/09/09 20:45:21  arman
 *2.0 Pre
 *
 *Revision 1.63  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.72  2005/08/22 23:54:35  arman
 *Cell Lineage II
 *
 *Revision 1.71  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.70  2005/08/16 03:30:40  arman
 *Cell Lineage II
 *
 *Revision 1.69  2005/08/12 20:37:53  arman
 *inc cell lineage plus
 *
 *Revision 1.68  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.67  2005/08/04 20:43:16  arman
 *added back intantTransformation to bring polys in to frame coordinate
 *system
 *
 *Revision 1.66  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.65  2005/07/27 23:40:37  arman
 *Moved rc_mathmodel.h to include dir
 *
 *Revision 1.64  2005/07/25 22:39:54  arman
 *removed unneeded code
 *
 *Revision 1.63  2005/07/21 22:08:26  arman
 *incremental
 *
 *Revision 1.62  1970/01/01 17:36:02  arman
 *incremental
 *
 *Revision 1.61  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.72  2005/06/09 21:53:18  arman
 *incremental
 *
 *Revision 1.71  2005/06/06 23:07:06  arman
 *added drawing initial cell children
 *
 *Revision 1.70  2005/06/05 06:58:38  arman
 *updaed wrt recent changes in template implementation of cardiac
 *function
 *
 *Revision 1.69  2005/06/03 20:09:51  arman
 *removed eCorrespondence check (no longer relevant)
 *
 *Revision 1.68  2005/05/28 16:48:01  arman
 *updated wrt rcShapeRef
 *
 *Revision 1.67  2005/05/23 23:52:27  arman
 *updated wrt to changes in rc_visualcell.cpp
 *
 *Revision 1.66  2005/05/23 12:08:30  arman
 *incremental
 *
 *Revision 1.65  2005/05/21 23:09:20  arman
 *streamlined and added new end tracking debug graphics
 *
 *Revision 1.64  2005/05/20 21:56:31  arman
 *updated visualFunction changes
 *
 *Revision 1.63  2005/05/18 21:09:35  arman
 *added support for debug drawing cell-end templates
 *
 *Revision 1.62  2005/05/16 10:05:22  arman
 *added polygon to debug graphics
 *
 *Revision 1.61  2005/05/15 00:14:28  arman
 *updated wrt rcVisualFunction changes
 *
 *Revision 1.60  2005/05/10 01:03:37  arman
 *adopted to new rcVisualFunction
 *
 *Revision 1.59  2005/05/02 21:00:10  arman
 *converted to the latest rcvisualcell changes
 *
 *Revision 1.58  2005/04/25 01:58:25  arman
 *addition of an intial plot of the per cell ACI
 *
 *Revision 1.57  2005/03/03 14:48:48  arman
 *added calcium
 *
 *Revision 1.56  2005/02/28 22:53:49  arman
 *Dividing cells are in blue
 *
 *Revision 1.55  2004/12/16 22:04:29  arman
 *graphics are now transformed to client space
 *
 *Revision 1.54  2004/12/07 22:10:38  arman
 *removed bounding rect and enabled polygon drawing
 *
 *Revision 1.53  2004/11/22 01:54:48  arman
 *updated wrt changes in rcVisualFunction
 *
 *Revision 1.52  2004/11/19 22:19:13  arman
 *corrected motion vector display
 *
 *Revision 1.51  2004/09/16 21:46:50  arman
 *added but commented out color wheel support
 *
 *Revision 1.50  2004/06/15 21:57:16  arman
 *incremental
 *
 *Revision 1.49  2004/06/14 12:18:19  arman
 *added per cell plot of Length and Time
 *Modified graphics definition of muscle end
 *
 *Revision 1.48  2004/06/04 14:54:48  arman
 *removed cause for a warning
 *
 *Revision 1.47  2004/04/22 23:10:48  arman
 *added identification of the two ends
 *
 *Revision 1.46  2004/04/18 18:45:07  arman
 *fixed side selection
 *
 *Revision 1.45  2004/04/05 14:03:26  arman
 *added drawing of the ends
 *
 *Revision 1.44  2004/04/01 23:50:45  arman
 *changed api to affine rectangle to new one
 *
 *Revision 1.43  2004/03/30 20:56:01  arman
 *added 1d plotting of profiles
 *
 *Revision 1.42  2004/03/19 12:37:44  arman
 *adding contractile motion source
 *
 *Revision 1.41  2004/02/26 23:57:49  arman
 *removed warnings
 *
 *Revision 1.40  2004/02/16 21:52:29  arman
 *on-going changes in how mvectos look
 *
 *Revision 1.39  2004/02/11 22:45:55  arman
 *inc checkin
 *
 *Revision 1.38  2004/02/04 10:35:29  arman
 *removed references to RLEs
 *
 *Revision 1.37  2004/02/03 19:58:36  arman
 *removed debugging display
 *
 *Revision 1.36  2004/02/03 15:03:05  arman
 *incremental
 *
 *Revision 1.35  2004/01/24 03:09:40  arman
 *corrected polygon position drawing error
 *
 *Revision 1.34  2004/01/18 00:44:19  arman
 *removed clutter. Motion vector shows length
 *
 *Revision 1.33  2004/01/14 20:43:02  arman
 *current
 *
 *Revision 1.32  2003/12/15 20:38:44  sami
 *Silence compiler warning
 *
 *Revision 1.31  2003/12/15 20:32:16  sami
 *Added new kinetoscope debug data methods debug() and visualDebugGraphics()
 *
 *Revision 1.30  2003/12/05 20:09:30  arman
 *added circle drawing for beads
 *
 *Revision 1.29  2003/11/25 01:57:41  arman
 *motion vectors are normalized. Structures are outlined
 *
 *Revision 1.28  2003/10/24 15:31:58  sami
 *Disable color sweep
 *
 *Revision 1.27  2003/09/03 22:26:40  sami
 *Refactoring, cleanup
 *
 *Revision 1.26  2003/08/28 22:02:09  sami
 *Make motion vector color a bit more orange (better contrast against white)
 *
 *Revision 1.25  2003/08/26 20:39:19  sami
 *Added rcVisualStyle argument to visualGraphics, visualFunctionMap and
 *visualFunctionHistory graphics methods
 *
 *Revision 1.24  2003/08/26 10:04:22  arman
 *fixed a warning
 *
 *Revision 1.23  2003/08/26 01:27:46  arman
 *corrected angular color display. It is still not perfect.
 *
 *Revision 1.22  2003/08/22 19:11:42  arman
 *removed cosmetic magnification of motion vector lengths. turned on angluar sweep
 *
 *Revision 1.21  2003/08/21 19:35:06  sami
 *Persistence improvements, graphics persistence
 *
 *Revision 1.20  2003/08/05 19:27:24  sami
 *Added conditionalized code to demonstrate rcVisualStyle usage
 *
 *Revision 1.19  2003/07/15 22:10:08  sami
 *Graphics color tweaks (use less saturated colors)
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_kinetoscope.h>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <rc_platform.h>
#include <rc_polygon.h>
#include <rc_draw.h>


inline uint32 rfRgbf( float r, float g, float b )
{ return (0xff << 24) | ((((uint32) (r * 255)) & 0xff) << 16) | ((((uint32) (g * 255)) & 0xff) << 8) | (((uint32) (b * 255)) & 0xff); }

static const rcVisualStyle affy ( rfRgbf(0.1f, 1.0f, 0.2f ), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle caliper ( rfRgbf(0.82f, 0.26f, 0.0f), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle caliper2 ( rfRgbf(0.0f, 0.26f, 0.82f), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle graph ( rfRgbf(1.0f, 160.0f/255.0f, 80.0f/255.0f), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle affylow ( rfRgbf(0.05f, 0.5f, 0.1f ), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle fnd ( rfRgbf(1.0f, 0.26f, 0.02f), 2, rc2Fvector( 0.0f, 0.0f ) );    
static const rcVisualStyle green ( rfRgbf(0.58f, 0.82f, 0.18f ), 2, rc2Fvector( 0.0f, 0.0f ) );
static const rcVisualStyle yel ( rfRgbf(0.8f, 1.0f, 0.60f ), 2, rc2Fvector( 0.0f, 0.0f ));
static const rcVisualStyle red ( rfRgbf(0.9f, 0.0f, 0.0f ), 2, rc2Fvector( 0.0f, 0.0f ));

// #define ARROW_COLOR_SWEEP 1

void rcKinetoscope::visualGraphics (rcVisualSegmentCollection& segments,
                                    const rcVisualStyle& baseStyle)
{
  rmUnused( baseStyle );
  rmUnused(segments );
}




void rcKinetoscope::visualFunctionMap (rcVisualSegmentCollection& segments,
                                       const rcVisualStyle& baseStyle )
{
  rmUnused( baseStyle );
  // Reserve space for one graphics token per motion vector
  segments.reserve( 1 + mPathMap.size() );


  // If there are motion vectors draw them.
  rcMotionPathMap::iterator pos;
  for (pos = mPathMap.begin(); pos != mPathMap.end(); pos++)
    {

      rc2Fvector p2(pos->second.anchor().x(), 
		    pos->second.anchor().y());
      rc2Fvector p1 (pos->second.position().x(), 
		    pos->second.position().y());
      rc2Fvector p21 (p1 - p2);
      if (p21.isNull())
          continue;

      p1 = xform().mapPoint (p1);
      p2 = xform().mapPoint (p2);
      rcVisualArrow arrow( p2, p1);
      segments.push_back( arrow );

    }


  
  list<rcVisualFunction>::const_iterator cell;
  for( cell = mFunctionMap.begin(); cell != mFunctionMap.end(); ++cell )
    {
      if (cell->isCardiacCell ())
	{
	  rcAffineRectangle ar = cell->affys().back();
	  segments.push_back (affy);
	  rfAffineRectangleToSegmentsCollection (ar, segments);

	  // if we have dimensios, draw projections
	  if (cell->have(rcVisualFunction::eDimensions))
	    {
	      // Draw Length Calipers
	      rcFPair maj = cell->majorEnds();
	      bool side = cell->isState (rcVisualFunction::eIsAffineXdirected);
	      int32 d = side ? ar.cannonicalSize().x() : ar.cannonicalSize().y();
	      rcFPair size ((double) d, (double) d);
	      maj = maj / size;

	      if (side)
		{
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.x(), 0.33)), 
						    ar.affineToImage(rc2Dvector(maj.x(), 0.67))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.y(), 0.33)), 
						    ar.affineToImage(rc2Dvector(maj.y(), 0.67))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.x(), 0.5)), 
						    ar.affineToImage(rc2Dvector(maj.y(), 0.5))));

		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.0, 0.5)), 
						    ar.affineToImage(rc2Dvector(0.0, 0.5))));


		}
	      else
		{
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.33, maj.x())), 
						    ar.affineToImage(rc2Dvector(0.67, maj.x()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.33, maj.y())), 
						    ar.affineToImage(rc2Dvector(0.67, maj.y()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.5, maj.x())), 
						    ar.affineToImage(rc2Dvector(0.5, maj.y()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.5, 0.0)), 
						    ar.affineToImage(rc2Dvector(0.5, 0.0))));

		}

	      // Draw the last framerPerPeriod in the affine Rectangle
	      vector<float> lengthHistory ((uint32) cell->framesPerPeriod());
	      const deque<rcFPair>& imin = cell->interpolatedMin ();

	      deque<rcFPair>::const_iterator start = (imin.size() < lengthHistory.size()) ? imin.begin() :
		imin.end() - lengthHistory.size();
	      deque<rcFPair>::const_iterator finish = imin.end();
	      vector<float>::iterator lIter;
	      vector<float>::iterator historyStart = lengthHistory.begin() + (imin.size() < lengthHistory.size() ?
									      lengthHistory.size() - imin.size() : 0);

	      for (lIter = historyStart; lIter < lengthHistory.end() && start < finish; lIter++, start++)
		*lIter = start->y();

	      // Place a rectangle at cell position
	      rcFRect ctr (cell->position().x() - cell->framesPerPeriod() / 2,
			   cell->position().y() - cell->framesPerPeriod() / 2,
			   cell->framesPerPeriod(), cell->framesPerPeriod());
	      segments.push_back (graph);
	      rfPlot1Dsignal (historyStart, lengthHistory.end(), ctr, segments, false);
	    }
	}
      else
	{
	  rcPolygon pog = cell->polygons().back();

	  // @note: add other visual codes for cell function 
	  if (cell->isState (rcVisualFunction::eIsMoving))
	    {
	      segments.push_back (affy);
	    }

	  // @note: add other visual codes for cell function 
	  if (cell->isState (rcVisualFunction::eIsDivided))
	    {
	      segments.push_back (affylow);
	    }

	  if (cell->isState (rcVisualFunction::eIsDividing))
	    {
	      segments.push_back (caliper2);
	    }
	  
	  rfPolygonToSegmentsCollection (pog, segments);

	}
    }
}


void rcKinetoscope::visualFunctionHistory (rcVisualSegmentCollection& segments,
                                           const rcVisualStyle& baseStyle)
{
    rmUnused( baseStyle );
        
    // Show cell bounding box
    list<rcVisualFunction>::const_iterator cell;
    for( cell = mFunctionMap.begin(); cell != mFunctionMap.end(); ++cell ) {
      rcIRect r = cell->roundRectangle (cell->rectSnaps().back());
        rcVisualRect rect( r.origin().x(), r.origin().y(), r.width(), r.height() );
        segments.push_back( rect );
    }
}

void rcKinetoscope::visualGraphics (rcVisualGraphicsCollection& graphics)
{
    // Collect line segments
    rcVisualSegmentCollection& segments = graphics.segments();
    // Green color, line width of 1 display pixel, 0.0 pixel origin
    rcVisualStyle style( rfRgb( 10, 255, 20 ), 1, rc2Fvector( 0.0f, 0.0f ) );
    // Set drawing style
    segments.push_back( style );
    visualGraphics( segments, style );

    rcVisualStyle style2 ( rfRgb( 10, 20, 255 ), 1, rc2Fvector( 0.0f, 0.0f ) );
    // Set drawing style
    segments.push_back( style2 );
    // Show cell bounding box

}

void rcKinetoscope::visualDebugGraphics(rcVisualSegmentCollection& segments,
                                        const rcVisualStyle& baseStyle)
{
    rmUnused( baseStyle );

    // The first polygon is for fixed. But we want results for moving so start from the second
    vector<rcPolygon>::const_iterator vc = movingPolygons ().begin();
    for (;vc != movingPolygons ().end() && vc->isValid(); vc++)
      {
	segments.push_back (fnd);
	rfPolygonToSegmentsCollection (*vc, segments);
      }

    vc = fixedPolygons ().begin();
    for (;vc != fixedPolygons ().end() && vc->isValid(); vc++)
      {
	segments.push_back (graph);
	rfPolygonToSegmentsCollection (*vc, segments);
      }

    vc = mMergedPOLYs.begin();
    for (;vc != mMergedPOLYs.end() && vc->isValid(); vc++)
      {
	segments.push_back (red);
	rfPolygonToSegmentsCollection (*vc, segments);
      }

    // Show cell bounding box
    list<rcVisualFunction>::const_iterator cell;
    for( cell = mFunctionMap.begin(); cell != mFunctionMap.end(); ++cell ) {

      if (rfCellNotViable (*cell)) continue;

      rcPolygon pog = cell->polygons().back();
      segments.push_back (caliper2);
      rfPolygonToSegmentsCollection (pog, segments);

      if (cell->isCardiacCell ())
	  {
	    rcAffineRectangle ar = cell->affys().back();
	    segments.push_back (affy);
	    rfAffineRectangleToSegmentsCollection (ar, segments);
	    const vector<float>& profile = cell->shape()->majorProfile();
	    rfPlot1Dsignal (profile.begin(), profile.end(), ar, segments);

    // Draw Length Calipers
	      rcFPair maj = cell->majorEnds();
	      bool side = cell->isState (rcVisualFunction::eIsAffineXdirected);
	      int32 d = side ? ar.cannonicalSize().x() : ar.cannonicalSize().y();
	      rcFPair size ((double) d, (double) d);
	      maj = maj / size;

	      if (side)
		{
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.x(), 0.33)), 
						    ar.affineToImage(rc2Dvector(maj.x(), 0.67))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.y(), 0.33)), 
						    ar.affineToImage(rc2Dvector(maj.y(), 0.67))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(maj.x(), 0.5)), 
						    ar.affineToImage(rc2Dvector(maj.y(), 0.5))));

		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.0, 0.5)), 
						    ar.affineToImage(rc2Dvector(0.0, 0.5))));


		}
	      else
		{
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.33, maj.x())), 
						    ar.affineToImage(rc2Dvector(0.67, maj.x()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.33, maj.y())), 
						    ar.affineToImage(rc2Dvector(0.67, maj.y()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.5, maj.x())), 
						    ar.affineToImage(rc2Dvector(0.5, maj.y()))));
		  segments.push_back (caliper);
		  segments.push_back (rcVisualLine (ar.affineToImage(rc2Dvector(0.5, 0.0)), 
						    ar.affineToImage(rc2Dvector(0.5, 0.0))));

		}

	    for (uint32 i = 0; i < cell->rectTemplates().size(); i++)
	      {
		segments.push_back (fnd);
		rcVisualRect rect(cell->rectTemplates()[i]);
		segments.push_back( rect );
	      }

	    for (uint32 i = 0; i < cell->rectSearches().size(); i++)
	      {
		segments.push_back (caliper2);
		rcVisualRect rect(cell->rectSearches()[i]);
		segments.push_back( rect );
	      }

	    // Place a rectangle at origin of the affine rect coordinate 
	    rc2Dvector ul = ar.affineToImage(rc2Dvector(0.0, 0.0));
	    rc2Dvector lr = ar.affineToImage(rc2Dvector(0.05, 0.05));
	    rcVisualRect rect (rc2Fvector ((float) ul.x(), (float) ul.y()), rc2Fvector ((float) lr.x(), (float) lr.y()));
	    segments.push_back( rect );
	  }
      else if (have (rcKinetoscope::eLineage))
	{
	  segments.push_back (yel);
	  segments.push_back(	  rcVisualRect (cell->similarityArea ()));

	  vector<rcPolygon>::const_iterator vc = movingPolygons ().begin() + cell->label();
	  rcPolygon peg = vc->discBuffer (5.0);
	  rfPolygonToSegmentsCollection (peg, segments);
	}
    }
}

void rcKinetoscope::visualFunctionMap (rcVisualGraphicsCollection& graphics)
{
    // Collect line segments
    rcVisualSegmentCollection& segments = graphics.segments();
    // Blue color, line width of 1 display pixel, 0.0 pixel origin
    rcVisualStyle style( rfRgb( 255, 240, 0 ), 1, rc2Fvector( 0.0f, 0.0f ) );
    // Set drawing style
    segments.push_back( style );
    visualFunctionMap( segments, style );
}


void rcKinetoscope::visualFunctionHistory (rcVisualGraphicsCollection& graphics)
{
    // Collect line segments
    rcVisualSegmentCollection& segments = graphics.segments();
    // Yellow color, line width of 1 display pixel, 0.0 pixel origin
    rcVisualStyle style( rfRgb( 200, 200, 0 ), 1, rc2Fvector( 0.0f, 0.0f ) );
    // Set drawing style
    segments.push_back( style );
    
    visualFunctionHistory( segments, style );
}

void rcKinetoscope::visualDebugGraphics (rcVisualGraphicsCollection& graphics)
{
    // Collect line segments
    rcVisualSegmentCollection& segments = graphics.segments();
    // Yellow color, line width of 1 display pixel, 0.0 pixel origin
    rcVisualStyle style( rfRgb( 200, 200, 0 ), 1, rc2Fvector( 0.0f, 0.0f ) );
    // Set drawing style
    segments.push_back( style );
    
    visualDebugGraphics( segments, style );
}



