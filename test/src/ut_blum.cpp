/*
 *  ut_blum.cpp
 *  
 *
 *  Created by Arman Garakani on Tue Aug 20 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_time.h>
#include "ut_blum.h"
#include <iomanip>


#define rmPrintImage(a){		    \
  for (uint32 i = 0; i < (a).height(); i++)	\
    {						\
      cout << endl;					\
      for (uint32 j = 0; j < (a).width(); j++)	\
	{						\
	  if ((a).depth() != 8)				\
	    cout <<  setw(5) << (int32) ((a).getPixel (j, i));	\
	  else cout <<  setprecision(3) << setw (5) << (a).getDoublePixel (j, i); \
	}						\
      cout << endl;					\
    }}


UT_Blum::UT_Blum()
{
}

UT_Blum::~UT_Blum()
{
        printSuccessMessage( "rcBlum test", mErrors );
}

uint32
UT_Blum::run() {
  // Test different sizes
    {        
      //      testCBL ();
      testBlum ();
      testSeedp ();
    }
    return mErrors;
}

void UT_Blum::testSeedp()
{
  int *image1, *image2;
  int size[2];

  size[0]=size[1]=1024;
  image1 = (int*) calloc ( size[0]*size[1], sizeof(int));
  image2 = (int*) calloc ( size[0]*size[1], sizeof(int));

  draw(image1,size);
  draw(image2,size);
  distmap_saito2d(image1,size);
  distmap_4ssedp(image2,size);  
  rcUTCheck (diff(image1,image2,size) == 0);
  free(image1); free(image2);

  printSuccessMessage( "test Seedp unit test" , mErrors);   
}

void UT_Blum::testCBL()
{
  vector<rc2Fvector>vec (32);

  rcRadian po8 (rk2PI/8.);

  randomSeed ();
  for (uint32 i = 0; i < vec.size()/4; i++)
    {
      vec[i] = rc2Fvector (float(10.0), i * po8) +
	rc2Fvector (float (rand()) / float (RAND_MAX+1) + float (10.0), 
		    float (rand()) / float (RAND_MAX+1) + float (10.0));
    }

  randomSeed ();
  for (uint32 i = vec.size()/4; i < vec.size()/2; i++)
    {
      vec[i] = rc2Fvector (float(20.0), i * po8) + 
	rc2Fvector (float (rand()) / float (RAND_MAX+1) + float (20.0), 
		    float (rand()) / float (RAND_MAX+1) + float (20.0));

    }

  // at another location
  randomSeed ();
  for (uint32 i = vec.size()/2; i < (3*vec.size()/4); i++)
    {
      vec[i] = rc2Fvector (float(20.0), i * po8) + 
	rc2Fvector (float (rand()) / float (RAND_MAX+1) + float (52.0), 
		    float (rand()) / float (RAND_MAX+1) + float (47.0));

    }

  // a line at another location
  randomSeed ();
  for (uint32 i = (3*vec.size()/4); i < vec.size(); i++)
    {
      vec[i] = rc2Fvector (float (90), float (i)) + 
	rc2Fvector (float (rand()) / float (RAND_MAX+1), 
		    float (rand()) / float (RAND_MAX+1));

    }


  vector<float> labels = rfCBLDistance (vec);

  for (uint32 i = 0; i < vec.size(); i++)
    cout << labels[i] << endl;


   printSuccessMessage( "testCBL unit test" , mErrors);   
}



void UT_Blum::testBlum()
{
  {
    vector<rc2Dvector> vec(4);

    vec[0] = rc2Dvector (0.3, 4.4);
    vec[1] = rc2Dvector (2.2, 3.1);
    vec[2] = rc2Dvector (4.0, 2.9);
    vec[3] = rc2Dvector (4.2, 0.7);

    // Find max x and y distance
    rc2Dvector maxi;

    vector<rc2Dvector>::iterator fpos = vec.begin();
    for (; fpos < vec.end(); fpos++)
      {
	maxi.x(rmMax (maxi.x(), fpos->x()));
	maxi.y(rmMax (maxi.y(), fpos->y()));
      }

    rcWindow feat (uint32 (maxi.x() + 1),
		   uint32 (maxi.y() + 1), rcPixel32);
    rcWindow dist (uint32 (maxi.x() + 1),
		   uint32 (maxi.y() + 1), rcPixelDouble);

    rfGrassFire (vec, feat, dist);

    rcUTCheck (feat.width() == 5);
    rcUTCheck (feat.height() == 5);
    rcUTCheck (dist.width() == 5);
    rcUTCheck (dist.height() == 5);

    rcUTCheck (feat.getPixel(0,0) == 1);
    rcUTCheck (feat.getPixel(1,0) == 1);
    rcUTCheck (feat.getPixel(0,1) == 1);

    rcUTCheck (feat.getPixel(0,4) == 0);
    rcUTCheck (feat.getPixel(1,4) == 0);
    rcUTCheck (feat.getPixel(0,3) == 0);

    rcUTCheck (feat.getPixel(4, 0) == 3);
    rcUTCheck (feat.getPixel(4, 1) == 3);
    rcUTCheck (feat.getPixel(3, 0) == 3);

    rcUTCheck (feat.getPixel(4, 4) == 2);
    rcUTCheck (feat.getPixel(3, 3) == 2);
    rcUTCheck (feat.getPixel(3, 4) == 2);


  }

  vector<rc2Dvector>mvec (32);
  rc2Dvector maxi (0.0, 0.0);

  rcRadian po8 (rk2PI/8.);
  randomSeed ();

  for (uint32 i = 0; i < mvec.size(); i++)
    {
      mvec[i] = rc2Dvector (float(10.0), i * po8) +
	rc2Dvector (rand() / double (RAND_MAX+1) + 10.0,
		    rand() / double (RAND_MAX+1) + 10.0);
    }



  // Find max x and y distance
  vector<rc2Dvector>::iterator fpos = mvec.begin();
  for (; fpos < mvec.end(); fpos++)
    {
      maxi.x(rmMax (maxi.x(), fpos->x()));
      maxi.y(rmMax (maxi.y(), fpos->y()));
    }

  rcWindow feature (uint32 (maxi.x() + 1),
		    uint32 (maxi.y() + 1), rcPixel32);
  rcWindow distance (uint32 (maxi.x() + 1),
		     uint32 (maxi.y() + 1), rcPixelDouble);

  rfGrassFire (mvec, feature, distance);

  rcUTCheck (distance.getDoublePixel (9, 9) > 9.5);
  rcUTCheck (feature.getPixel (9, 9) >= 30);

  printSuccessMessage( "testBlum unit test" , mErrors);   
}


void UT_Blum::draw(int *im, int size[2])
{
  int i,j;
  int maxval;
  float x,y;
  float rx, ry;
  
  rx = ( size[0] * 0.5 ) ;
  ry = ( size[1] * 0.5 ) ;
  maxval = size[0]*size[0]+size[1]*size[1]+1;

  for(j=0;j<size[1];j++)
    for(i=0;i<size[0];i++)
      {
	x = ( i - size[0]/2 ) / rx;
	y = ( j - size[1]/2 ) / ry;
	
	if( ( ( x*x + y*y ) > 1 ) || ( ( x*x + y*y ) < 0.9 ) )
	  im[i+j*size[0]] = maxval;
	else 
	  im[i+j*size[0]] = 0;
      }
}

int UT_Blum::diff(int *im1, int *im2, int size[2])
{
  int i,j,pt;
  int count=0;

  pt=0;
  for(j=0;j<size[1];j++)
    for(i=0;i<size[0];i++,pt++)
      if( im1[pt] != im2[pt] ) 
	count++;

  return(count);
}
