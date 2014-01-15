/*
 * 	$Id: ut_motion.cpp 4768 2006-11-17 18:44:14Z armanmg $	
 *      Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 *       *$Log$
 *       *Revision 1.52  2006/01/01 21:30:58  arman
 *       *kinetoscope ut
 *       *
 *       *Revision 1.51  2005/12/27 15:48:38  arman
 *       *incremental in ut for kinetoscope
 *       *
 *       *Revision 1.50  2005/12/27 01:01:36  arman
 *       *incremental towards kineut
 *       *
 *       *Revision 1.49  2005/12/03 00:35:36  arman
 *       *tiff plus motion compensation
 *       *
 *       *Revision 1.48  2005/11/26 22:29:30  arman
 *       *changed synthetic movie to squares moving at sub-pixel demonstrating
 *       *aprture issue.
 *       *
 *       *Revision 1.47  2005/10/19 22:04:26  arman
 *       *updated test wrt visualFunction states
 *       *
 *       *Revision 1.46  2005/08/30 21:08:50  arman
 *       *Cell Lineage
 *       *
 *       *Revision 1.46  2005/08/22 15:50:14  arman
 *       *added visualFunction tests
 *       *
 *       *Revision 1.45  2004/12/10 03:52:11  arman
 *       *removed motion center
 *       *
 *       *Revision 1.44  2004/11/19 22:10:22  arman
 *       *added a new movie test
 *       *
 *       *Revision 1.43  2004/08/24 21:37:57  arman
 *       **** empty log message ***
 *       *
 *       *Revision 1.42  2004/07/12 19:43:55  arman
 *       *updated api
 *       *
 *       *Revision 1.41  2004/02/16 21:49:00  arman
 *       *updated according to latest changes in rcFeature
 *       *
 *       *Revision 1.40  2004/02/04 17:33:19  sami
 *       *Commented unused code
 *       *
 *       *Revision 1.39  2004/01/24 02:18:36  arman
 *       *incremental checkin
 *       *
 *       *Revision 1.38  2004/01/21 23:50:25  sami
 *       *Removed unused static function makeTmpName()
 *       *
 *       *Revision 1.37  2004/01/15 01:36:23  arman
 *       *removed unnecessary include
 *       *
 *       *Revision 1.36  2004/01/14 21:33:38  arman
 *       *in progress
 *       *
 *       *Revision 1.35  2003/11/10 01:21:23  arman
 *       *removed ACF -- no longer used.
 *       *
 *       *
 */

#include <rc_utdrawutils.h>
#include <rc_moments.h>
#include <rc_gen_movie_file.h>
#include <rc_mathmodel.h>
#include <rc_ip.h>


using namespace std;

#include "ut_motion.h"


extern void segment (rcWindow& image, rcWindow& connect, float dt, float lambda, int32 times);

static std::string fileName;

UT_motion::UT_motion(std::string movieFileName)
{

}

UT_motion::~UT_motion()
{
    printSuccessMessage( "rcmotion test", mErrors );
}

uint32
UT_motion::run()
{
    {        
       // testBasic ();
        testfile ();
        testVisualFunction ();
    }
    return mErrors;
}


void UT_motion::testVisualFunction()
{
    rcSharedKinetoscope sharedKin = boost::shared_ptr<rcKinetoscope> (new rcKinetoscope ());
    rcUTCheck (!sharedKin->canStep());
    sharedKin->visualFunctionInfo ( rcOrganismInfo::eFluorescenceCluster, 
                                    rcOrganismInfo::eProtein);

    // Create a cell and test the basics
    rcPolygon poly (100.0f, 8);
    rcIRect big (500, 500, 450, 380);
    rc2Fvector motionCtr;
    poly.translate (rc2Dvector (big.ul().x(), big.ul().y()));
    poly.centerOf (motionCtr);
    rcVisualFunction vf (sharedKin, motionCtr, poly);  
    rcUTCheck (vf.id() == 0); // id is not set by the ctor
    rcPolygon back = vf.polygons().back();
    rcUTCheck (back == poly);
    rcUTCheck (vf.position() == motionCtr);
    rcFRect fr;
    poly.orthogonalEnclosingRect(fr);
    rcFRect snap = vf.rectSnaps().back ();
    rcUTCheck (snap == fr);
    rcUTCheck (vf.isState (rcVisualFunction::eIsInitialized));
    rcUTCheck (!rfCellNotViable (vf));

    // Make a copy and check
    rcVisualFunction foo (vf);
    rcUTCheck (foo.id() == 0); // id is not set by the ctor
    rcPolygon back2 = foo.polygons().back();
    rcUTCheck (back2 == poly);
    rcUTCheck (foo.position() == motionCtr);
    rcFRect fsnap = foo.rectSnaps().back ();
    rcUTCheck (fsnap == fr);
    rcUTCheck (foo.isState (rcVisualFunction::eIsInitialized));
    rcUTCheck (!rfCellNotViable (foo));

}




void UT_motion::testBasic()
{

    {
        rcKinetoscope rk;
        rcUTCheck (!rk.canStep());
    }

    // Test Ctor with FileGrabber 
    // Default and specific rect specification

    for (int32 depth = 1; depth < 3; depth++)
    {
        buildImages (192, 192, (rcPixel) depth);
        rcVectorGrabber vg (images ());
        rcUTCheck (vg.isValid());
  
        const rcIRect rect;
        rcKinetoscope watch (vg, rect);
        rcUTCheck (watch.canStep() == true); // construction worked
        rcUTCheck (watch.iframe().size() == rcIPair (64, 64));
        rcUTCheck (watch.elapsedTime() == 0);
        rcUTCheck (watch.deltaTime() == 0);
        rcUTCheck(watch.startingAbsoluteTime().secs() == 0);
        rcUTCheck (watch.minMobSizeInPixels () == 25); // 25 is the default
        rcUTCheck (watch.organism() == rcOrganismInfo::eCell);
        rcUTCheck (watch.organismName() == rcOrganismInfo::eUnknown);
        rcUTCheck (watch.isOfOrganismType(rcOrganismInfo::eCell));
        rcUTCheck (watch.isOfOrganismName(rcOrganismInfo::eUnknown));
        rcUTCheck (!watch.isCardiacCell());
        rcUTCheck (!watch.isLabelProtein());
        rcUTCheck (watch.frames () == 9);    
        rcUTCheck (watch.sample () == 1);  
        rcUTCheck (watch.phase () == 0);  
        rcUTCheck (real_equal (watch.laminarSuppression (), 0.8, 0.00001));

        // Before calling step the first time, ctor has fetched one frame
        rcUTCheck (watch.count() == 1);
        rcUTCheck (watch.use4fixed().isBound());
        rcUTCheck (!watch.use4moving().isBound());
        rcUTCheck (watch.fixed().contentCompare (images()[0]));

        // Check connection mapping and transformation
        rc2Dvector lowres; // ctor'd to 0,0
						   //    rc2Dvector mappedhighres = watch.connectionXform().mapPoint (lowres);
						   //      rcUTCheck (mappedhighres.x() == 9.5);
						   //     rcUTCheck (mappedhighres.y() == 9.5);      

        rc2Dvector highres (9.5,9.5); 
			//    rc2Dvector mappedlowres = watch.connectionXform().invMapPoint (highres);
			//      rcUTCheck (mappedlowres.x() == 0.0);
			//      rcUTCheck (mappedlowres.y() == 0.0);      
      
        rc2Dvector midp (16.0, 16.0);
			//   rc2Dvector mappedmidp = watch.connectionXform().mapPoint (midp);
			//      rcUTCheck (mappedmidp.x() == 41.5);
			//rcUTCheck (mappedmidp.y() == 41.5);      

    
        // Step Through and Check
        // Check fixed and moving images
        // Check time stamps for results and timers

        for (uint32 i = 1; i < images().size(); i++)
	{
            rcUTCheck (watch.step());
            rcUTCheck (watch.use4fixed().isBound());
            rcUTCheck (watch.use4moving().isBound());
            rcUTCheck (watch.fixed().contentCompare (images()[i-1]));
            rcUTCheck (watch.moving().contentCompare (images()[i]));
            rcUTCheck (real_equal (watch.deltaTime(), 33.0, 0.001));
            rcUTCheck (real_equal (watch.elapsedTime(), 33.0 * i , 0.001));
            rcWindow tmp; 
            watch.fieldMap (tmp);
            watch.advance ();
	}
    }
}


void UT_motion::buildImages (int32 width, int32 height, rcPixel depth)
{
    rc2Dvector ctr ((double) width, (double) height);
    ctr /= 2.0;
    int32 dia = rmMin (width, height) / 5;
    rcWindow tmp (width, height, depth);
    rcWindow tmp2 (width, height, depth);
    double time = 0.0;

    mImages.clear ();

    for (int32 j = -1; j < 2; j++)
        for (int32 i = -1; i < 2; i++)
        {
            tmp.setAllPixels (128);

            rcWindow r (tmp, i + int32 (ctr.x() - dia / 2.0) , j + int32 (ctr.y() - dia / 2.0), dia, dia);
            r.setAllPixels ((uint32) 32);
            rfGaussianConv (tmp, tmp2, 17);
            rcWindow celld = rfPixelSample (tmp2, 3, 3);
            celld.frameBuf()->setTimestamp (rcTimestamp (time));
            time += 0.033;
            mImages.push_back (celld);
        }
}
  
void UT_motion::testfile()
{

    for (int32 depth = 1; depth < 3; depth++)
    {
        rcVideoCache* videoCacheP = 0;
        std::string cellName = makeTmpName(0, "cellut.rfymov");
        const char* creator = "UT_generator";
        const movieFormatRev rev = movieFormatRevLatest;
        const movieOriginType origin = movieOriginSynthetic;
  
        rcGenMovieFile cellMovie(cellName, origin,  rcSelectAll, creator,
                                 rev, true, 1.0f);

        buildImages (192, 192, (rcPixel) depth);

        vector<rcWindow>::const_iterator imageItr = images().begin ();
        for (; imageItr < images().end (); imageItr++)
	{
            cellMovie.addFrame (*imageItr);
	}
        cellMovie.flush();

        rcUNITTEST_ASSERT( cellMovie.valid());

        // Test Ctor with VideoCache
        // Default and specific rect specification
        {
            videoCacheP = rcVideoCache::rcVideoCacheCtor(cellName, 0, true);
            const rcIRect rect;
            rcKinetoscope watch (*videoCacheP, rect);
            rcUTCheck (watch.canStep() == true); // construction worked
            rcUTCheck (watch.sample() == 1);
            rcUTCheck (watch.phase() == 0);
            rcUTCheck (watch.iframe().size() == rcIPair (64, 64));
            rcUTCheck (watch.elapsedTime() == 0);
            rcUTCheck (watch.deltaTime() == 0);
            rcUTCheck(watch.startingAbsoluteTime().secs() == 0);
            rcUTCheck (watch.minMobSizeInPixels () == 25); // 25 is the default
            rcUTCheck (watch.organism() == rcOrganismInfo::eCell);
            rcUTCheck (watch.organismName() == rcOrganismInfo::eUnknown);
            rcUTCheck (watch.isOfOrganismType(rcOrganismInfo::eCell));
            rcUTCheck (watch.isOfOrganismName(rcOrganismInfo::eUnknown));
            rcUTCheck (!watch.isCardiacCell());
            rcUTCheck (!watch.isLabelProtein());
            rcUTCheck (watch.frames () == 9);    
            rcUTCheck (watch.sample () == 1);  
            rcUTCheck (watch.phase () == 0);  
            rcUTCheck (real_equal (watch.laminarSuppression (), 0.8, 0.00001));

            // Before calling step the first time, ctor has fetched one frame
            rcUTCheck (watch.count() == 1);
            rcUTCheck (watch.use4fixed().isBound());
            rcUTCheck (!watch.use4moving().isBound());
            rcUTCheck (watch.fixed().contentCompare (images()[0]));


            for (uint32 i = 1; i < images().size(); i++)
            {
                rcUTCheck (watch.step());
                rcUTCheck (watch.use4fixed().isBound());
                rcUTCheck (watch.use4moving().isBound());
                rcUTCheck (watch.fixed().contentCompare (images()[i-1]));
                rcUTCheck (watch.moving().contentCompare (images()[i]));
                rcUTCheck (real_equal (watch.deltaTime(), 33.0, 0.001));
                rcUTCheck (real_equal (watch.elapsedTime(), 33.0 * i , 0.001));
                watch.advance ();
            }
        }
 
        if ( videoCacheP )
            rcVideoCache::rcVideoCacheDtor( videoCacheP );
    }

}




#if 0

// Test as above but this time, initiate a defined-by-ends cardiomyocyte
  {
      rcVectorGrabber vg (images);
      rcUTCheck (vg.isValid());
  
      const rcIRect rect;
      rcKinetoscope watch (vg, rect, 0, 1, 
                           rcKinetoscope::eFine, 
                           rcMovieFileOrgExt (), 
                           rcOrganismInfo::eCell, rcOrganismInfo::eCardioMyocyteSelected, 25);
      rcUTCheck (watch.canStep() == true); // construction worked
      rcUTCheck (watch.iframe().size() == rcIPair (64, 64));
      rcUTCheck (watch.elapsedTime() == 0);
      rcUTCheck (watch.deltaTime() == 0);
      rcUTCheck(watch.startingAbsoluteTime().secs() == 0);
      rcUTCheck (watch.minMobSizeInPixels () == 25); // 25 is the default
      rcUTCheck (watch.organism() == rcOrganismInfo::eCell);
      rcUTCheck (watch.isOfOrganismType(rcOrganismInfo::eCell));
      rcUTCheck (watch.isOfOrganismName(rcOrganismInfo::eCardioMyocyteSelected));
      rcUTCheck (watch.isCardiacCell());
      rcUTCheck (!watch.isLabelProtein());
      rcUTCheck (watch.frames () == 9);    
      rcUTCheck (watch.sample () == 1);  
      rcUTCheck (watch.phase () == 0);  
      rcUTCheck (real_equal (watch.laminarSuppression (), 0.8, 0.00001));

      // Before calling step the first time, ctor has fetched one frame
      rcUTCheck (watch.count() == 1);
      rcUTCheck (watch.use4fixed().isBound());
      rcUTCheck (!watch.use4moving().isBound());
      rcUTCheck (watch.fixed().contentCompare (images[0]));

      // Step Through and Check
      // Check fixed and moving images
      // Check time stamps for results and timers
      vector< vector< rc2Fvector> > bodies;
      vector<rc2Fvector> bends;
      bends.push_back (rc2Fvector ((float) (ctr.x() - dia / 2.0f) / 3.0f, (float) ctr.y() / 3.0f));
      bends.push_back (rc2Fvector ((float) (ctr.x() + dia / 2.0f) / 3.0f, (float) ctr.y() / 3.0f));
      bodies.push_back (bends);
      vector<float> rads (1, dia / (3.0f * 2.0f));
      watch.preSegmentedBodies (bodies, rads);

      for (uint32 i = 1; i < images.size(); i++)
      {
          rcUTCheck (watch.step());
          rcUTCheck (watch.use4fixed().isBound());
          rcUTCheck (watch.use4moving().isBound());
          rcUTCheck (watch.fixed().contentCompare (images[i-1]));
          rcUTCheck (watch.moving().contentCompare (images[i]));
          rcUTCheck (real_equal (watch.deltaTime(), 33.0, 0.001));
          rcUTCheck (real_equal (watch.elapsedTime(), 33.0 * i , 0.001));
#if 0
          const list<rcVisualFunction>& visualBodies =watch.visualBodies();
          list<rcVisualFunction>::const_iterator cell;
          for( cell = visualBodies.begin(); cell != visualBodies.end(); ++cell )
              cerr << *cell << endl;
#endif
          watch.advance ();
      }

  }

// Now add checks for testing polygons and other analysis mode 
// Now add checks for testing visual graphics output

#endif
