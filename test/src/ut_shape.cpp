/*
 *
 *$Id $
 *$Log$
 *Revision 1.20  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.19  2005/11/04 22:04:25  arman
 *cell lineage iv
 *
 *Revision 1.18  2005/08/30 21:08:50  arman
 *Cell Lineage
 *
 *Revision 1.18  2005/07/28 00:13:17  arman
 *pre-release ci for lineage additions
 *
 *Revision 1.17  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.18  2005/06/28 17:24:24  arman
 *ut_shape (and shape in general ) need a lot of attention to special
 *casesl. For now this is a good enough test
 *
 *Revision 1.17  2005/05/25 01:35:39  arman
 *updated wrt shape changes
 *
 *Revision 1.16  2005/02/08 19:55:54  arman
 *updated wrt to changes in rcShape.
 *
 *Revision 1.15  2005/02/07 21:48:41  arman
 *incremental
 *
 *Revision 1.14  2005/02/07 13:51:31  arman
 *incremental adding ut for ctf
 *
 *Revision 1.13  2005/02/07 02:00:10  arman
 *commented out mask for now
 *
 *Revision 1.12  2004/08/20 19:10:13  arman
 *added mask moment tests. one of the com tests is broken
 *
 *Revision 1.11  2004/08/20 14:17:51  arman
 *added weighted test code
 *
 *Revision 1.10  2004/08/20 13:35:34  arman
 *added ut for frame moment calculation
 *
 *Revision 1.9  2004/07/19 20:28:53  arman
 *updated unit test
 *
 *Revision 1.8  2004/03/31 00:40:16  arman
 **** empty log message ***
 *
 *Revision 1.7  2004/03/30 23:15:27  arman
 *incremental
 *
 *Revision 1.6  2004/03/15 06:16:59  arman
 *initial coverage
 *
 *Revision 1.5  2004/03/12 22:13:00  arman
 *added more test methods
 *
 *Revision 1.4  2004/03/11 04:34:51  arman
 *added new tests
 *
 *Revision 1.3  2004/03/10 04:10:46  arman
 *incremental
 *
 *Revision 1.2  2004/03/08 22:10:06  arman
 *incremental
 *
 *Revision 1.1  2004/03/05 18:01:17  arman
 *initial
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_shape.h"
#include <rc_utdrawutils.h>
#include <rc_ncs.h>

#include <rc_mathmodel.h>
#include <rc_draw.h>
#include <rc_imageprocessing.h>

UT_shape::UT_shape ()
{
}

UT_shape::~UT_shape ()
{
  printSuccessMessage( "Shape Analysis tests" , mErrors);
}

uint32 UT_shape::run ()
{
  testFilter ();
  testMoment ();
  testBasic();
  testAffine ();
}

static double moments (rcWindow& image, rc2Dvector& com)
{
  rc2Dvector cog;
  double mass (0.0);
  for (int32 j = 0; j < image.height(); j++)
    for (int32 i = 0; i < image.width(); i++)
      {
	double val = image.getPixel (i, j);
	mass += val;
	rc2Dvector w (i * val, j * val);
	cog += w;
      }
  com = cog / mass;

#if 0
  double mean = mass / image.pixelCount ();

  double wmass (0.0);
  rc2Dvector cog2;
  for (int32 j = 0; j < image.height(); j++)
    for (int32 i = 0; i < image.width(); i++)
      {
	double val = image.getPixel (i, j);
	val = val / 255.;
	wmass += val;
	rc2Dvector w (i * val, j * val);
	cog2 += w;
      }
#endif
  
  return mass;
}


static double moments (rcWindow& image, rcPolygon& p, rc2Dvector& com)
{
  rc2Dvector cog;
  double mass (0.0);

  rcWindow image2 (image.width(), image.height());
  rcWindow image3 (image.width(), image.height());
  image2.setAllPixels (0);
  rcFillPolygon(p, image2, 0xff);
  rfAndImage (image, image2, image3);

  for (int32 j = 0; j < image.height(); j++)
    for (int32 i = 0; i < image.width(); i++)
      {
	if (image3.getPixel (i, j) == 0) continue;
	double val = image3.getPixel (i, j);
	mass += val;
	rc2Dvector w (i * val, j * val);
	cog += w;
      }
  com = cog / mass;
}

void UT_shape::testMoment()
{
  rcMathematicalImage g (2.5);
  rcWindow gauss (33, 33);
  g.gauss (gauss);

  {
    rc2Dvector cog;
    double mass = moments (gauss, cog);

    rcShapeRef shaper = new rcShape (gauss);
    double smass = shaper->mass ();
    rc2Fvector fcog = shaper->centerOfmass ();
    rc2Dvector dcog ((double) fcog.x(), (double) fcog.y());
    rc2Dvector unit;

    rcUTCheck (mass == smass);
    rcUTCheck (real_equal (dcog.x(), cog.x(), 0.0001));
    rcUTCheck (real_equal (dcog.y(), cog.y(), 0.0001));
    rcUTCheck (dcog != unit);
  }

  {
    rcPolygon p;
    rc2Dvector p0 (5.0,7.0), p1 (15.0, 7.0), p2 (15.0, 27.0), p3 (5.0, 27.0);
    p.pushVertex (p0);
    p.pushVertex (p1);
    p.pushVertex (p2);
    p.pushVertex (p3);
    p.convexHullConvert();

    rc2Dvector cog;
    double mass = moments (gauss, p, cog);

    rcShapeRef shaper = new rcShape (gauss, p);
    //    shaper.setMask ();
    double smass = shaper->mass ();
    rc2Fvector fcog = shaper->centerOfmass ();
    rc2Dvector dcog ((double) fcog.x(), (double) fcog.y());
  }

  
}

void UT_shape::testBasic()
{

  {
    rcShape sh;
    rcUTCheck (sh.isBound() == false);
  }

  char *shape[] =  {"0000000000000000",
		    "0000000000000000",
                    "0000111000000000",
                    "0001111100000000",
                    "0011111110000000",
                    "0011121110000000",
                    "0011111110000000",
                    "0001111100000000",
                    "0000111000000000",
		    "0000000000000000",
		    "0000000000000000",
0};

    rcWindow ti;
    rcWindow tm;
    rfDrawShape (ti, shape);
    rfDrawShape (tm, shape);
    int32 ms (0);

    vector<rcPolygon> polys;
    vector<rcIRect> rects;


    rfComponentPolygons (ti, ms, polys, rects);
   
    rcUNITTEST_ASSERT (polys.size() == 1);
    rcUNITTEST_ASSERT (rects.size() == 1);

    {
      rcShapeRef col = new rcShape (polys[0]);
      rcUTCheck (col->isBound() == true);
      rcUTCheck (col->isBound(rcShape::eAffine) == true);
      rcUTCheck (col->isBound(rcShape::eImage) == false);
      rcUTCheck (real_equal (col->area(), 28.0, 0.1));
      rcUTCheck (real_equal (col->perimeter(), 19.3137, 0.1));
      rc2Fvector com = col->centerOfmass (rcShape::eAffine);
      rcUTCheck (real_equal (com.x(), 5.0f, 0.1f));
      rcUTCheck (real_equal (com.y(), 5.0f, 0.1f));

    }

  }


void UT_shape::testAffine()
{

  {
    rcShape sh;
    rcUTCheck (sh.isBound() == false);
  }

  {
    char *shape[] =
      {
	"000000000000",
	"000000000000",
	"000111100000",
	"000111100000",
	"000111100000",
	"000111100000",
	"000000000000",
	"000000000000", 0};

    char *frame[] =
      {
	"00000000000000000000",
	"00000000000000000000",
	"00015371537100000000",
	"00024462446100000000",
	"00033453345100000000",
	"00042644264100000000",
	"00015371537100000000",
	"00024462446100000000",
	"00033453345100000000",
	"00042644264100000000",
	"00000000000000000000",
	"00000000000000000000", 0};

    rcWindow ti;
    rcWindow tm;
    rfDrawShape (ti, shape);
    rfDrawShape (tm, frame);
    int32 ms (0);

    vector<rcPolygon> polys;
    vector<rcIRect> rects;


    rfComponentPolygons (ti, ms, polys, rects);
   
    rcUNITTEST_ASSERT (polys.size() == 1);
    rcUNITTEST_ASSERT (rects.size() == 1);
    cerr << polys[0] << endl;
    {
      rcShapeRef col = new rcShape (polys[0]);
      rcUTCheck (col->isBound() == true);
      rcUTCheck (col->isBound(rcShape::eAffine) == true);
      rcUTCheck (col->isBound(rcShape::eImage) == false);

      rcUTCheck (col->area() == 9.0f);
      rcUTCheck (col->perimeter() == 12.0f);
      rc2Fvector com = col->centerOfmass (rcShape::eAffine);
      rcUTCheck (com.x() == 4.5f);
      rcUTCheck (com.y() == 3.5f);

    }

    {
      rcShapeRef col = new rcShape (tm, polys[0]);
      rcUTCheck (col->isBound() == true);
      rcUTCheck (col->isBound(rcShape::eAffine) == true);
      rcUTCheck (col->isBound(rcShape::eImage) == true);

      rcUTCheck (col->area() == 9.0f);
      rcUTCheck (col->perimeter() == 12.0f);
      rcUTCheck (col->mass() == 63.0);
      rc2Fvector com = col->centerOfmass (rcShape::eAffine);
      rcUTCheck (com.x() == 4.5f);
      rcUTCheck (com.y() == 3.5f);

      rc2Fvector med = col->median ();
      rcUTCheck (real_equal (med.x(), 3.977f, 0.001f));
      rcUTCheck (real_equal (med.y(), 3.968f, 0.001f));

      com = col->centerOfmass (rcShape::eImage);
      rcUTCheck (real_equal (col->majorDimension (), 3.5f, 0.001f));
      rcUTCheck (real_equal (col->minorDimension (), 3.5f, 0.001f));
    }

    // Test copy and assignment
    {
      rcShapeRef col = new rcShape (tm, polys[0]);

      rcUTCheck (col->isBound() == true);
      rcUTCheck (col->isBound(rcShape::eAffine) == true);
      rcUTCheck (col->isBound(rcShape::eImage) == true);

      rcUTCheck (col->area() == 9.0f);
      rcUTCheck (col->perimeter() == 12.0f);
      rcUTCheck (col->mass() == 63.0f);
      rc2Fvector com = col->centerOfmass (rcShape::eAffine);
      rcUTCheck (com.x() == 4.5f);
      rcUTCheck (com.y() == 3.5f);

      com = col->centerOfmass (rcShape::eImage);
      rcUTCheck (real_equal (col->majorDimension (), 3.5f, 0.001f));
      rcUTCheck (real_equal (col->minorDimension (), 3.5f, 0.001f));

      // Test Ref counted a copy. col2 will be a reference counted pointer to col
      rcShapeRef col2 = col;
      rcUTCheck (col2->isBound() == true);      
      rcUTCheck (&(col2->majorProfile()[0]) ==
		 &(col->majorProfile()[0]));
      rcUTCheck (&(col2->minorProfile()[0]) == 
		 &(col->minorProfile()[0]));
      
    }
  }




  // Test at - 135 or so  angle

   {
     char *shape[] =
       {
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000011110000000",
	 "0000001111000000",
	 "0000000111100000",
	 "0000000011110000",
	 "0000000000000000"
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",0};

     char *frame[] =
       {
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000012340000000",
	 "0000001234000000",
	 "0000000123400000",
	 "0000000012340000",
	 "0000000000000000"
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",0};

     rcWindow ti;
     rcWindow tm;
     rfDrawShape (ti, shape);
     rfDrawShape (tm, frame);
     int32 ms (0);

     vector<rcPolygon> polys;
     vector<rcIRect> rects;


     rfComponentPolygons (ti, ms, polys, rects);
   
     rcUNITTEST_ASSERT (polys.size() == 1);
     rcUNITTEST_ASSERT (rects.size() == 1);
     // Add more tests of the resulting polygon

     {
       rcShapeRef col = new rcShape (polys[0]);
       rcUTCheck (col->isBound() == true);
       rcUTCheck (col->isBound(rcShape::eAffine) == true);
       rcUTCheck (col->isBound(rcShape::eImage) == false);

       rcUTCheck (col->area() == 13.0f);
       rcUTCheck (real_equal ((float) col->perimeter(), 15.6569f, 0.01f));
       rc2Fvector com = col->centerOfmass (rcShape::eAffine);
       rcUTCheck (com.x() == 8.0f);
       rcUTCheck (com.y() == 5.0f);
     }



     {
       rcShapeRef col = new rcShape (tm, polys[0]);
       rcUTCheck (col->isBound() == true);
       rcUTCheck (col->isBound(rcShape::eAffine) == true);
       rcUTCheck (col->isBound(rcShape::eImage) == true);
       rcUTCheck (col->area() == 13.0f);
       rcUTCheck (real_equal ((float) col->perimeter(), 15.6569f, 0.01f));
       rc2Fvector com = col->centerOfmass (rcShape::eImage);
       rcUTCheck (real_equal (com.x(), 9.025f, 0.01f));
       rcUTCheck (real_equal (com.y(), 5.675f, 0.01f));
       rcUTCheck (real_equal (col->majorDimension (), 6.1f, 0.1f));
       rcUTCheck (real_equal (col->minorDimension (), 3.5f, 0.1f));

       rcDegree dg (col->angle());
       rcUTCheck (real_equal (dg.Double(), 36.86, 0.01));

//        cout << dg.Double() << " degrees " << endl;
//        cout << col->majorDimension () << endl;
//        cout << col->minorDimension () << endl;

//        rc2Dvector four[4] = { col->affineRect().affineToImage(rc2Dvector(0.0, 0.0)),
// 			      col->affineRect().affineToImage(rc2Dvector(0.0, 1.0)),
// 			       col->affineRect().affineToImage(rc2Dvector(1.0, 0.0)),
// 			       col->affineRect().affineToImage(rc2Dvector(1.0, 1.0)) };
//        cout << four[0] << "d " << four[3].distance (four[0]) << endl;
//        cout << four[1] << "d " << four[0].distance (four[1]) << endl;
//        cout << four[2] << "d " << four[1].distance (four[2]) << endl;
//        cout << four[3] << "d " << four[2].distance (four[3]) << endl;

       
     }



   }


  // Test at + 135 or so  angle

   {
     char *shape[] =
       {
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000011110000",
	 "0000000111100000",
	 "0000001111000000",
	 "0000011110000000",
	 "0000000000000000"
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",0};

     char *frame[] =
       {
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",
	 "0000000012340000",
	 "0000000123400000",
	 "0000001234000000",
	 "0000012340000000",
	 "0000000000000000"
	 "0000000000000000",
	 "0000000000000000",
	 "0000000000000000",0};

     rcWindow ti;
     rcWindow tm;
     rfDrawShape (ti, shape);
     rfDrawShape (tm, frame);
     int32 ms (0);

     vector<rcPolygon> polys;
     vector<rcIRect> rects;


     rfComponentPolygons (ti, ms, polys, rects);
   
     rcUNITTEST_ASSERT (polys.size() == 1);
     rcUNITTEST_ASSERT (rects.size() == 1);
     // Add more tests of the resulting polygon

     {
       rcShapeRef col = new rcShape (polys[0]);
       rcUTCheck (col->isBound() == true);
       rcUTCheck (col->isBound(rcShape::eAffine) == true);
       rcUTCheck (col->isBound(rcShape::eImage) == false);

       rcUTCheck (real_equal ((float) col->area(), 13.5f, 0.01f));
       rcUTCheck (real_equal ((float) col->perimeter(), 17.1356f, 0.01f));
       rc2Fvector com = col->centerOfmass (rcShape::eAffine);
       rcUTCheck (real_equal (com.x(), 8.14f, 0.1f));
       rcUTCheck (real_equal (com.y(), 5.28f, 0.1f));
     }

     {
       rcShapeRef col = new rcShape (tm, polys[0]);
       rcUTCheck (col->isBound() == true);
       rcUTCheck (col->isBound(rcShape::eAffine) == true);
       rcUTCheck (col->isBound(rcShape::eImage) == true);

       rcUTCheck (real_equal ((float) col->area(), 13.5f, 0.01f));
       rcUTCheck (real_equal ((float) col->perimeter(), 17.1356f, 0.01f));
       rc2Fvector com = col->centerOfmass (rcShape::eAffine);
       rcUTCheck (real_equal (com.x(), 8.14f, 0.1f));
       rcUTCheck (real_equal (com.y(), 5.28f, 0.1f));

       com = col->centerOfmass (rcShape::eImage);
       rcUTCheck (real_equal (com.x(), 7.947f, 0.1f));
       rcUTCheck (real_equal (com.y(), 5.535f, 0.1f));

       rcUTCheck (real_equal (col->majorDimension (), 7.0f, 0.01f));
       rcUTCheck (real_equal (col->minorDimension (), 2.0f, 0.01f));
       rcDegree dg (col->angle());
       rcUTCheck (real_equal (dg.Double(), 135.0, 0.01));

   //     col->printCache ();
//        cout << dg.Double() << " degrees " << endl;
//        cout << col->majorDimension () << endl;
//        cout << col->minorDimension () << endl;

//        rc2Dvector four[4] = { col->affineRect().affineToImage(rc2Dvector(0.0, 0.0)),
// 			      col->affineRect().affineToImage(rc2Dvector(0.0, 1.0)),
// 			       col->affineRect().affineToImage(rc2Dvector(1.0, 0.0)),
// 			       col->affineRect().affineToImage(rc2Dvector(1.0, 1.0)) };
//        cout << four[0] << "d " << four[3].distance (four[0]) << endl;
//        cout << four[1] << "d " << four[0].distance (four[1]) << endl;
//        cout << four[2] << "d " << four[1].distance (four[2]) << endl;
//        cout << four[3] << "d " << four[2].distance (four[3]) << endl;


     }

   }


}


void UT_shape::testFilter()
{
  int32 hw (3);
  int32 size (20);
  float zf (0.0f);
  deque<float> signal (size + (hw + 1) * 2, zf);
  for (uint32 i = signal.size() / 4; i < signal.size() / 2; i++) signal[i] = 1.0f;

  deque<float> output;
  double dummy;
  rfCtfDiff1d (signal, 3, output, dummy, false);

  rcUTCheck (real_equal (output[3], -3.0f, 0.00001f));  
  rcUTCheck (real_equal (output[4], -3.0f, 0.00001f));  
  rcUTCheck (real_equal (output[10], 3.0f, 0.00001f));  
  rcUTCheck (real_equal (output[11], 3.0f, 0.00001f));  

}

