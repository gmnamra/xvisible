/*! \file persistence1d_driver.cpp
 * Use this program to run Persistence1D on data in text files. 
 *
 * This file contains a sample code for using Persistence1D on data in text files, and 
 * can be used to directly run Persistence1D on data in a single text file.
 *
 *  Command line: persistence1d_driver.exe	\<filename\> [threshold] [-MATLAB]
 *			- filename is the path to a data text file.
 *			  Data is assumed to be formatted as a single float-compatible value per row. 
 *			- [Optional] threshold is a floating point value. Acceptable threshold value >= 0
 *			- [Optional] -MATLAB - output indices match Matlab 1-indexing convention.
 *  Output:	- Indices of extrema, written to a text file, one value per row.
			  Indices of paired extrema are written in following rows. 
 *			  Indices are ordered according to their persistence, from most to least persistence. 
 *			  Even rows contain indices of minima.
 *			  Odd rows contain indices of maxima.
 *			  Global minimum is not paired and is not written to file.
 *			  Output filename: \<filename\>_res.txt
 *
 */


#include "persistence1d.hpp"
#include "linlsq.h"
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

#define MATLAB "-MATLAB"

using namespace std;
using namespace p1d;




struct norm_scaler
{
    void operator()(const std::vector<float>& src, std::vector<float>& dst, float pw)
    {
        vector<float>::const_iterator bot = std::min_element (src.begin (), src.end() );
        vector<float>::const_iterator top = std::max_element (src.begin (), src.end() );
        
        float scaleBy = *top - *bot;
        dst.resize (src.size ());
        for (int ii = 0; ii < src.size (); ii++)
        {
            float dd = (src[ii] - *bot) / scaleBy;
            dst[ii] = pow (dd, 1.0 / pw);
        }
    }
};



struct detrender
{
    struct lsqres
    {
        float r;
        double angle_in_radian;
        double constant;
        rc2Fvector mean_point;
    };
    void operator()(const std::vector<float>& src, lsqres& res)
    {
        LLSQ lsqr;
        double dsize = (double) src.size ();        
        for (uint ii = 0; ii < src.size (); ii++)
        {
            lsqr.add(ii / dsize, src[ii]);
        }
        res.angle_in_radian = lsqr.vector_fit_angle().Double();
        res.r = lsqr.pearson();
        res.constant = lsqr.c(lsqr.m());
        res.mean_point = lsqr.mean_point();

    }

};



int fpad_main ();

/*!
	Tries to open the input file and read its contents to a float vector.
	
	Input is assumed to be formatted as one number per line, in float compatible notation.
	
	Ignores any lines which do not conform to this assumption.

	Number of data entries is assumed to be smaller than vector's class maximum size - this is not checked!

	@param[in] filename		Name of input file with float data.
	@param[out] data		Data is written to this vector.
*/
bool ReadFileToVector (char * filename, vector<float> & data);
/*!
	Writes indices of extrema features to file, sorted according to their persistence. 

	If no features were found, writes an empty file.

	Overwrites any existing file with the same name.

	@param[in] filename		Name of output file.
	@param[out] pairs		Data to write.
	
*/
void WriteMinMaxPairsToFile (char * filename, vector<TPairedExtrema> pairs);
bool WriteVectorToFile (char * filename, vector<float> singles);

/*!
	Parses user command line.
	Checks if the user set a threshold value or wants MATLAB indexing.
*/
bool ParseCmdLine(int argc, char* argv[], float &threshold, bool & matlabIndexing);

/*!
	Main function - reads a file specified as a command line argument. runs persistence, 
	writes the indices of extrema to a file called inputfilename_res.txt. 
	
	Overwrites files with the same name.
	
	Input file name is assumed to end with a three letter extension. 
*/
int main(int argc, char* argv[])
{
	vector<float> data;
	vector<int> indices; 
	float threshold;
	vector <TPairedExtrema> pairs;
	bool matlabIndexing;
	Persistence1D p;

	if (argc < 2) 
	{
		cout << "No filename" << endl;
		cout << "Usage: " << argv[0] << " <filename> [threshold] [-MATLAB]" << endl;

		cout << "Running cpad demo " << endl;        
        fpad_main();
        
		return false;
	}

	//filename processing, easier done here.
    std::string ffullname (argv[1]);
    std::string fname = ffullname.substr(0, ffullname.length()-4);
    std::string outfilename = fname + "_res.txt";
    std::string nfname = fname + "_norm.txt";
    std::string d2fname = fname + "_d2.txt";    
    
	if (!ParseCmdLine(argc, argv, threshold, matlabIndexing))
	{
		cout << "Usage: " << argv[0] << " <filename> [threshold] [-MATLAB]" << endl;
		return -1; 
	}
	
	if(!ReadFileToVector(((char*)ffullname.c_str()), data))
	{
		cout << "Error reading data to file." << endl; 
		return -2;
	}
	
    vector<float> normed (data.size ());
    std::copy_n (data.begin(), data.size (), normed.begin () );
    norm_scaler ns;
    ns.operator()(normed, data, 1.0f);
    
    detrender dt;
    detrender::lsqres lsres;
    dt.operator()(data, lsres);
    
	if(!WriteVectorToFile(((char*)nfname.c_str()), normed))
	{
		cout << "Error reading data to file." << endl; 
		return -2;
	}
    
	p.RunPersistence(data);
	p.GetPairedExtrema(pairs, threshold , matlabIndexing);
    WriteMinMaxPairsToFile(((char*)outfilename.c_str()), pairs);
	if (! WriteVectorToFile (((char*)d2fname.c_str()), p.secondDerivative () ) )
    {
		cout << "Error reading data to file ." << endl; 
		return -2;
	}

		
	return 0;
}

bool ReadFileToVector (char * filename, vector<float> & data)
{
	ifstream datafile;
	
	//check the datadfile actually exists
	datafile.open(filename, ifstream::in);


	if(!datafile) 
	{
		cout << "Cannot open file " << filename << " for reading" << endl;
		return false;
	}

	float currdata;

	while(datafile >> currdata)
	{
		data.push_back(currdata);
	}
		
	datafile.close();
	return true;
}
void WriteMinMaxPairsToFile (char * filename, vector<TPairedExtrema> pairs)
{
	ofstream datafile; 
	datafile.open(filename);
	
	if (!datafile)
	{
		cout << "Cannot open file " << filename << " for writing." << endl;
		return;
	}
	
	for (vector<TPairedExtrema>::iterator p = pairs.begin(); p != pairs.end(); p++)
	{
		datafile << to_string((long long)(*p).MinIndex) << endl;
		datafile << to_string((long long)(*p).MaxIndex) << endl;
	}

	datafile.close();
}

bool WriteVectorToFile (char * filename, vector<float> singles)
{
	ofstream datafile;
	datafile.open(filename);

	if (!datafile)
	{
		cout << "Cannot open file " << filename << " for writing." << endl;
		return false;
	}

	for (vector<float>::iterator p = singles.begin(); p != singles.end(); p++)
	{
		datafile << to_string((float)(*p)) << endl;
    }

	datafile.close();
    return true;
}

bool ParseCmdLine(int argc, char* argv[], float &threshold, bool & matlabIndexing)
{	
	bool noErrors = true;
		
	threshold = 0.0;
	matlabIndexing = false;
	
	//now let's find out if anyone wants MATLAB indexing or threshold values
	for (int counter = 2; counter < argc ; counter ++)
	{
		if (argv[counter][0]=='-' && matlabIndexing == false)
		{
			if (strcmp(argv[counter],"-MATLAB") == 0 || 
				strcmp(argv[counter],"-Matlab") == 0 || 
				strcmp(argv[counter],"-matlab") == 0 )
			{				
				//turn on matlab indexing
				matlabIndexing = true;
			}
			else
			{
				cout << "Possibly misspelled Matlab flag or negative values for threshold." << endl;
				noErrors = false;
			}
		}
		else if ( argv[counter][0] == '0') //different from nullptr
		{
			//this doesn't throw exceptions, AKAIK
			threshold = (float)atof(argv[counter]);

			//string begins with 0, 
			//so it's ok that atof returns 0
			//not much can be done about other errors, will set threshold to 0
			
			//check that value is positive - this really should not happen
			if (threshold < 0)
			{
				cout << "Error. Threshold value should be >= 0. Rerun with valid threshold value or leave out to get all features.\n";
				noErrors = false;
			}		
		}
		else
		{
			//this doesn't throw exceptions, AKAIK
			threshold = (float)atof(argv[counter]);

			//the string does not include a 0, but atof returns 0. 
			//string cannot be converted to threshold value
			if (threshold == 0.0)
			{ 
				cout << "Cannot convert threshold value to number.\n" << endl;
				noErrors = false;
			}
			else if (threshold < 0)
			{
				cout << "Error. Threshold value should be >= 0. Rerun with valid threshold value or leave out to get all features.\n";
				noErrors = false;
			}			
		}
				
	}
	return noErrors;
}




int fpad_main() 
{ 
    fpad x (0.123, 1.0); // Identitaet für x=0.123 
    // 1.0 ist die Ableitung der Identitaet 
    fpad y = x*x - 3*sin(x) + 2; 
    std::cout << "f(x) = x*x - 3*sin(x) + 2\n" 
    "fuer x=0.123 ist f (x) = " << y.value() << "\n" 
    "             ist f'(x) = " << y.derivative() << "\n"; 
    return 0;
}

