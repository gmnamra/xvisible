/*
 *
 *$Id $
 *$Log: $
 *
 * Copyright (c) 2007 Reify Corp. All rights reserved.
 */

#include <stlplus.hpp>
#include <textio.hpp>
#include <fstream>
#include <iostreamio.hpp>
#include <rc_types.h>
#include <rc_fileutils.h>
#include <rc_vector2d.h>
#include <rc_macro.h>
#include <matrix.hpp>

using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) exit (0);
	std::string inf = string (argv[1]);
	std::string filef = string (argv[2]);

  try
    {
      iftext input (inf);
      deque<double> dm;
      restore_from_file (inf, dm, 0);

	  ofstream output_stream(filef.c_str(), ios::trunc);
      // create and initialise the TextIO wrapper device
      oiotext output(output_stream);
      // now use the device
      uint32 i, j;
      deque<double>::iterator ds = dm.begin();
	  for (j = 0; j < cm.size() - 1; j++)
	     output << *ds++ << ",";
	   output << *ds << endl;
      output_stream.flush();
    }
}


