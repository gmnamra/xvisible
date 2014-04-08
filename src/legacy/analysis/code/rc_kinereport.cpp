/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2005/11/18 17:45:21  arman
 *reporting functions
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_kinetoscope.h>

ostream& operator<< (ostream& output, rcKinetoscope& kin)
{

  const list<rcVisualFunction>& visualBodies = kin.visualBodies();

  list<rcVisualFunction>::const_iterator cell;
  for( cell = visualBodies.begin(); cell != visualBodies.end(); ++cell )
    {
      output << "cell" << cell->id() << "minLaTtlX" << (int32) cell->position().x() << "tlY" << (int32) cell->position().y() << " = {";
      for (uint32 i = 0; i < cell->interpolatedMin().size() - 1;i++)
	{
	  if (i) output << ",";
	  output << "{" << cell->interpolatedMin()[i].x() << "," << cell->interpolatedMin()[i].y() << "}";
	}
      output << "};" << endl;
    }

  for( cell = visualBodies.begin(); cell != visualBodies.end(); ++cell )
    {
      output << "cell" << cell->id() << "ceAnglesTlX" << (int32) cell->position().x() << "tlY" << (int32) cell->position().y() << " = {";
      for (uint32 i = 0; i < cell->peakAngles().size() - 1;i++)
	{
	  if (i) output << ",";
	  rcDegree c (cell->peakAngles()[i].x());
	  rcDegree r (cell->peakAngles()[i].y());
	  output << "{" << c.Double() << "," << r.Double() << "}";
	}
      output << "};" << endl;
    }

  return output;
}

void rcKinetoscope::dumpPolys (ostream& output)
{
  output << "Time: " << count () << "Available Polys: " << mPOLYs.size() << endl;
  vector<rcPolygon>::iterator vc = mPOLYs.begin();
  for (; vc < mPOLYs.end(); vc++)
    {
      rc2Fvector cop;
      rcFRect fr;
      vc->centerOf (cop);
      vc->orthogonalEnclosingRect(fr);
      output << "(" << fr << "::" << cop << ")" << endl;
    }
}


ostream& rcKinetoscope::temporalResults (ostream& output1, ostream& output2)
{
  uint32 maxNumberOfFrames (rcUINT32_MAX);
  uint32 maxNumberOfCycles (rcUINT32_MAX);

  const list<rcVisualFunction>& vbs = visualBodies();

  output1 << "Positions in pixels, top to bottom, left to right" << endl;

  // First Line Cell IDs + Positions
  list<rcVisualFunction>::const_iterator cell;
  for( cell = vbs.begin(); cell != vbs.end(); ++cell )
    {
      output1 << "\t" << cell->id() << "\t" << "X" << (int32) cell->position().x()  << "Y" << (int32) cell->position().y() << "\t";
      maxNumberOfFrames = rmMin (maxNumberOfFrames, cell->interpolatedMin().size());
    }

  output1 << endl;

  // 2nd to frame size time and length
  for (uint32 i = 0; i < maxNumberOfFrames - 1;i++)
    {
      for( cell = vbs.begin(); cell != vbs.end(); ++cell )
	{
	  if (i < cell->interpolatedMin().size())
	    {
	      float x (cell->interpolatedMin()[i].x());
	      float y (cell->interpolatedMin()[i].y());
	      output1 << std::fixed << "\t" << x << "\t" << y;
	    }
	}
    }

  output1 << endl << endl;

  // Two different outputs

  for( cell = vbs.begin(); cell != vbs.end(); ++cell )
    {
      output2 << "\t" << cell->id() << "\t" << "X" << (int32) cell->position().x()  << "Y" << (int32) cell->position().y() << "\t";
      maxNumberOfCycles = rmMin (maxNumberOfCycles, cell->peakAngles().size());
    }

  output2 << endl;

  for (uint32 i = 0; i < maxNumberOfCycles - 1;i++)
    {
      for( cell = vbs.begin(); cell != vbs.end(); ++cell )
	{
	  if (i < cell->peakAngles().size())
	    {
	      rcDegree c (cell->peakAngles()[i].x());
	      rcDegree r (cell->peakAngles()[i].y());
	      output2 << std::fixed << "\t" << c.Double() << "\t" << r.Double();
	    }
	}
    }

  return output1;
}


/*	**********************************
	*                                *
	*     Special Output Processing  *
	*                                *
	**********************************
*/

// @note division support
// list<rcVisualFunction>::const_iterator cell;
// for( cell = visualBodies.begin(); cell != visualBodies.end(); ++cell )
//   {
//     output << "Frame: " << analysisCount-1 << " Time: " << curTimeStamp.secs() << endl;

//     output << "cell " << "[" << cell->id() << "," << cell->parentId () << "]" 
// 	   << (int32) cell->position().x() << "tlY" << (int32) cell->position().y() 
// 	   << " age: " << cell-> age ();
		
//     if (cell->state() == rcVisualFunction::eIsMoving) 
//       output << " is Moving ";
//     else if (cell->state() == rcVisualFunction::eIsDividing)
//       output << " is Dividing ";
//     else if (cell->state() == rcVisualFunction::eIsDividing)
//       output << " is Divided  ";
//   }

