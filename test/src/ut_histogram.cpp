/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2004/10/14 19:48:59  arman
 *added some commented out histogram code for testing
 *
 *Revision 1.2  2004/10/07 18:18:00  arman
 *added listhistogram test
 *
 *Revision 1.1  2004/08/20 01:31:20  arman
 **** empty log message ***
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_histogram.h"


UT_histogram::UT_histogram ()
{
}

uint32 UT_histogram::run ()
{
  testGeneralHistogram ();
  testListHistogram ();
  return mErrors;
}

UT_histogram::~UT_histogram ()
{
    printSuccessMessage( "Histogram test", mErrors );  
}



void UT_histogram::testGeneralHistogram ()
{
  bool pass = true;
  std::string whereFail = "" ;
  
  typedef float MeasurementType ;

  // creats a histogram with 3 components measurement vectors
  typedef rcHistogram< MeasurementType, 3 > HistogramType ;
  HistogramType::Pointer histogram = HistogramType::New() ;

  // initializes a 64 x 64 x 64 histogram with equal size interval
  HistogramType::SizeType size ;
  size.Fill(64) ;
  unsigned long totalSize = size[0] * size[1] * size[2] ;
  HistogramType::MeasurementVectorType lowerBound ;
  HistogramType::MeasurementVectorType upperBound ;
  lowerBound.Fill(0.0) ;
  upperBound.Fill(1024.0) ;
  histogram->Initialize(size, lowerBound, upperBound ) ;
  histogram->SetToZero();
  HistogramType::MeasurementType interval = 
    (upperBound[0] - lowerBound[0]) / 
    static_cast< HistogramType::MeasurementType >(size[0]) ;

  // tests begin
  HistogramType::MeasurementVectorType measurements ;
  measurements.Fill(512.0) ;
  HistogramType::IndexType index ;
  HistogramType::IndexType ind;
  index.Fill(32) ;
  if(histogram->GetIndex(measurements,ind))
    {
    if(index != ind)
      {
      pass = false ;
      whereFail = "GetIndex(MeasurementVectorType&)";
      }
    }
  else
    {
    pass = false ;
    whereFail = "GetIndex(MeasurementVectorType&)";
    }
  
  HistogramType::InstanceIdentifier id = 
    histogram->GetInstanceIdentifier(index);
  if (index != histogram->GetIndex(id))
    {
    pass = false ;
    whereFail = "GetIndex(InstanceIdentifier&)" ;
    }

  index.Fill(100) ;
  
  if (!histogram->IsIndexOutOfBounds(index))
    {
    pass = false ;
    whereFail = "IsIndexOutOfBound(IndexType)" ;
    }

  if (totalSize != histogram->Size())
    {
    pass = false ;
    whereFail = "Size()" ;
    }

  if (size != histogram->GetSize())
    {
    pass = false ;
    whereFail = "GetSize()" ;
    }

  if ((lowerBound[0] + interval * 31) != histogram->GetBinMin(0,31))
    {
    pass = false ;
    whereFail = "GetBinMin(Dimension, nthBin)" ;
    }

  if ((lowerBound[0] + interval * 32) != histogram->GetBinMax(0,31))
    {
    pass = false ;
    whereFail = "GetBinMax(Dimension, nthBin)" ;
    }

  for (id = 0 ; 
       id < static_cast< HistogramType::InstanceIdentifier >(totalSize) ;
       id++)
    {
    histogram->SetFrequency(id, 1) ;
    histogram->IncreaseFrequency(id, 1) ;
    if (histogram->GetFrequency(id) != 2)
      {
      pass = false ;
      whereFail = 
        "SetFrequency(InstanceIdentifier, 1) + IncreaseFrequency(InstanceIdentifier, 1) + GetFrequency(InstanceIdentifier)" ;
      }
    }

  if (histogram->Quantile(0, 0.5) != 512.0)
    {
    pass = false ;
    whereFail = "Quantile(Dimension, percent)" ;
    }

  rcUTCheck (pass == true);
  if( !pass )
    {
    std::cout << "Test failed in " << whereFail << "." << std::endl;
    }

#if 0
  {

  // creats a histogram with 3 components measurement vectors
    typedef rcHistogram< MeasurementType, 1 > HistogramType ;
  HistogramType::Pointer histogram = HistogramType::New() ;

  // initializes a 64 x 64 x 64 histogram with equal size interval
  HistogramType::SizeType size ;
  size.Fill(64) ;
  unsigned long totalSize = size[0];
  HistogramType::MeasurementVectorType lowerBound ;
  HistogramType::MeasurementVectorType upperBound ;
  lowerBound.Fill(-1.0) ;
  upperBound.Fill(1.0) ;
  histogram->Initialize(size, lowerBound, upperBound ) ;
  histogram->SetToZero();
  HistogramType::MeasurementType interval = 
    (upperBound[0] - lowerBound[0]) / 
    static_cast< HistogramType::MeasurementType >(size[0]) ;

  // tests begin
  HistogramType::MeasurementVectorType measurements ;
  measurements.Fill(-0.5) ;
  HistogramType::IndexType index ;
  HistogramType::IndexType ind;
  

  histogram->GetIndex (measurements, index);

  MeasurementType bmx = histogram->GetBinMax (0, 0);
  MeasurementType bmi = histogram->GetBinMin (0, 0);

  cerr << "i: " << index[0] << " " << (float) bmx << "gg" << (float) bmi << "  " << 
    (float) histogram->GetFrequency (index) << endl;

  histogram->IncreaseFrequency (measurements, 1.0);
  measurements.Fill(0.5);
  histogram->IncreaseFrequency (measurements, 1.0);  
  measurements.Fill(0.0);
  histogram->IncreaseFrequency (measurements, 1.0);  

  cerr << (float) histogram->GetBinMax (0, 0) << "gg" << 
    (float) histogram->GetBinMin (0, 0) << 
    (float) histogram->GetFrequency (index) << "Total: " << 
    (float) histogram->GetTotalFrequency () << endl;

  cerr << histogram->Quantile (0, 0.01) << 
    " -- > " << histogram->Quantile (0, 0.99) << endl;
  }
#endif
  
}


void UT_histogram::testListHistogram ()
{
  bool pass = true;
  std::string whereFail = "" ;

  typedef int MeasurementType ;
  typedef rcFixedArray<MeasurementType,2> MeasurementVectorType ;
  typedef rcListSample< MeasurementVectorType > ListSampleType ;

  ListSampleType::Pointer sample = ListSampleType::New() ;

  MeasurementVectorType mv ;
  for ( unsigned int i = 1 ; i < 6 ; i++ )
    {
      for (unsigned int j = 0 ; j < 2 ; j++ )
        {
          mv[j] = ( MeasurementType ) i ;
        }
      for ( unsigned int j = 0 ; j < i ; j++ )
        {
          sample->PushBack(mv) ;
        }
    }

  typedef float HistogramMeasurementType ;
  typedef rcListSampleToHistogramGenerator< ListSampleType, HistogramMeasurementType > GeneratorType ;
  
  GeneratorType::Pointer generator = GeneratorType::New() ;

  GeneratorType::HistogramType::SizeType size ;
  size.Fill(5) ;

  generator->SetListSample( sample ) ;
  generator->SetNumberOfBins(size) ;
  generator->Update() ;

  GeneratorType::HistogramType::ConstPointer histogram = generator->GetOutput() ;

  GeneratorType::HistogramType::IndexType index ;
  
  index.Fill(0) ;

  for ( unsigned int i = 0 ; i < 5 ; i++ )
    {
      index[0] = i ;
      for ( unsigned int j = 0 ; j < 5 ; j++ )
        {
          index[1] = j ;
          
          if ( i == j && histogram->GetFrequency(index) != (i + 1.0) )
            {
              pass = false ;
              whereFail = " this class" ;
            }
        }
    }

  rcUTCheck (pass == true);
  if( !pass )
    {
    std::cout << "Test failed in " << whereFail << "." << std::endl;
    }

}
