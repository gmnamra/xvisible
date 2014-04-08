/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2005/08/30 21:02:22  arman
 **** empty log message ***
 *
 *Revision 1.4  2005/08/24 02:40:38  arman
 *Cell Lineage II
 *
 *Revision 1.3  2005/08/23 23:32:31  arman
 *Cell Lineage II
 *
 *Revision 1.2  2005/08/16 22:23:39  arman
 *Cell Lineage II
 *
 *Revision 1.1  2005/08/16 22:05:16  arman
 *SVD association
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

// #define MATHPRINT

#include <rc_associate.h>
#include <functional>
#include <iomanip>
#include "nr.h"

#if 1
// Association using algorithm developed by Scott & onguet-Higgins. 
// "An algorithm for associating the features of two patterns", 
// 		   Proc. Royal Society London, Vol B244, p 21-26, 1991
// SVD decomposition is applied to a association matrix of pairwise distances. 
static int32 rowMaxX(Mat_DP P, int32 width, int32 row, double& ratio, double& rmax);
static int32 colMaxY(Mat_DP P, int32 height, int32 col, double& ratio, double& cmax);
  
void rfAssociate (vector<rc2Dvector>& cells, vector<rc2Dvector>& regions, 
		vector<int32>& labels, vector<float>& scores)
{
  int32 j,k,l,m,n;

  m = cells.size();
  n = regions.size();
  rmAssert (labels.size() == cells.size());
  
  if (!n || !m) return;

  Vec_DP w(n);
  Mat_DP a(m,n),u(m,n),v(n,n);

  //#define PM(a) pow (2.71828, -(a) / (1.4 * gl))
#define PM(a) (1.0 / (a))

  rc2Dvector cop;
  rc2Fvector lp;

  vector<rc2Dvector>::iterator cell = cells.begin();
  for (k=0;k<m;k++, cell++)
    {
      labels[k] = 0; // clear the labels (no relation to this loop)
      rc2Fvector kp;
      kp.x((float) cell->x()), kp.y((float) cell->y());

      for (l = 0; l < n; l++)
	{
	  lp.x((float) regions[l].x()), lp.y((float) regions[l].y());

	  double pm = PM(kp.distanceSquared (lp) + 1e-10);
	  a[k][l] = pm;
	  u[k][l] = pm;
	}
    }

  // perform decomposition
  NR::svdcmp(u,w,v);

  // Replace the diagonal with 1. And produce U W V^t
  for (k=0;k<m;k++)
    {
      for (l=0;l<n;l++)
	{
	  a[k][l]=0.0;
	  for (j=0;j<n;j++)
	    a[k][l] += u[k][j]*v[l][j];
	}
    }

#ifdef MATHPRINT
  cerr.setf (ios::fixed);
  cerr << setprecision (4);
  cerr << "{";
  for (k=0;k<m;k++)
    {
      cerr << "{";
    for (l=0;l<n;l++)
      {
	  double moo = a[k][l] < 1e-7 ? 0.0 : a[k][l];
	  if (l < n - 1)
	    cerr << setw(5) << moo << ",\t";
	  else
	    cerr << setw(5) << moo << "}" << endl;
      }
    if (k < m - 1)
      cerr << " , ";
    else
      cerr << "};" << endl;
    }
  cerr << endl;
#endif MATHPRINT
 
  // @note: 
  // Look for associations that are max in both x and y
  // For each cell find the best region.
  // Then check that the best region is only best for this cell. 
  cell = cells.begin();
  for (k = 0; k < m; k++, cell++)
    {
      double ratio;
      double rmax (-1.0), cmax (-1.0);
      int32 rowMax = rowMaxX (a, n, k, ratio, rmax);
      if (colMaxY (a, m, rowMax, ratio, cmax) == k)
	{
	  rmAssert (rowMax >= 0);
	  rmAssert (rowMax < n);
	  uint32 index = cell - cells.begin();
	  labels[index] = rowMax;
	  scores[index] = (float) cmax;
	}
    }
}
  


/* Gives ratio between first best and second best */

static int32 rowMaxX(Mat_DP P, int32 width, int32 row, double& ratio, double& rmax)
{
 double sec_max=-9999999.0, max = -9999999.0;
 int32 i,sec_max_i(-1),max_i (-1);

 for (i=0; i<width; i++)
   if (P[row][i]>max)
     {
       sec_max = max;
       sec_max_i = max_i;
       max = P[row][i];
       max_i = i;
     }

 rmax = max;
 if (sec_max != 0.0) ratio = max/sec_max;
 return max_i;

}

/* Gives ratio between first best and second best */  
static int32 colMaxY(Mat_DP P, int32 height, int32 col, double& ratio, double& cmax)
{
 double sec_max=-9999999.0, max = -9999999.0;
 int32 i,sec_max_i(-1),max_i(-1);

 for (i=0; i<height; i++)
   if (P[i][col]>max)
     {
       sec_max = max;
       sec_max_i = max_i;
       max = P[i][col];
       max_i = i;
     }

 cmax = max;
 if (sec_max != 0.0) ratio = max/sec_max;
 ratio = max/sec_max;
 return max_i;
}

#endif

