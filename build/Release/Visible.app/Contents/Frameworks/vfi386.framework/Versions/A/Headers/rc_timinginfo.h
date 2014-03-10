// Copyright 2002 Reify, Inc.

#ifndef _rcTIMINGINFO_H_
#define _rcTIMINGINFO_H_

#include <rc_types.h>
#include <rc_timestamp.h>
#include <vector>
#include <string>

class rcTimingInfo
{
 public:
  
  /* Create an object that will store timing info for "locations"
   *  number of locations and will remember the last "passes" number
   *  of passes.
   */
  rcTimingInfo(uint32 locations, uint32 passes);

  /* Mark a touch at "location". "Location" must be less than the
   * value of "locations" passed in the ctor. Note that only the 1st
   * touch of a location is recorded, until the next pass starts.
   */
  void touch(uint32 location);
  void touch(uint32 location, std::string label);

  /* Indicate the start of a new pass through the execution stream. If
   * "reset" is true all existing pass information is forgotten.
   */
  void nextPass(bool reset);

  /* Display in a neat, tablular form the time between touches. Up to
   * "passes" number of passes is displayed. The limiting factor is
   * the number of valid passes currently available for printing.
   *
   * The "labels" version will print the strings in the labels
   * vector to be printeed alongside the timing info.
   */
  void printInfo(uint32 passes, vector<std::string>& labels);
  void printInfo(uint32 passes);

  static bool turnOffPrint () { mPrintOff = true; return mPrintOff; }
  static bool turnOnPrint () { mPrintOff = false; return mPrintOff;}
  static bool mPrintOff;
  
 private:
  vector<rcTimestamp>               _times;
  vector<std::string>                       _labels;
  rcTimestamp                       _resetTime;
  vector<vector<double> >           _deltas;
  vector<vector<double> >::iterator _firstValidPass;
  vector<vector<double> >::iterator _curPass;
  uint32                          _curPassLoc;
 };

#endif // _rcTIMINGINFO_H_
