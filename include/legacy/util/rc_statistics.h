#ifndef rcSTATISTICS_H
#define rcSTATISTICS_H


template< class TSize >
TSize rfFloorLog(TSize size) ;

template< class TValue >
TValue rfMedianOfThree(const TValue a, const TValue b, const TValue c) ;

template< class TSample >
void rfFindSampleBound(const TSample* sample,
                     typename TSample::ConstIterator begin,
                     typename TSample::ConstIterator end,
                     typename TSample::MeasurementVectorType &min,
                     typename TSample::MeasurementVectorType &max) ;
  
template< class TSubsample >
void rfFindSampleBoundAndMean(const TSubsample* sample,
                            int beginIndex,
                            int endIndex,
                            typename TSubsample::MeasurementVectorType &min,
                            typename TSubsample::MeasurementVectorType &max,
                            typename TSubsample::MeasurementVectorType &mean) ;

template< class TSubsample >
int rfPartition(TSubsample* sample,
              unsigned int activeDimension,
              int beginIndex, int endIndex,
              const typename TSubsample::MeasurementType partitionValue) ;

template< class TSubsample >
typename TSubsample::MeasurementType 
rfQuickSelect(TSubsample* sample,
            unsigned int activeDimension,
            int beginIndex, int endIndex,
            int kth,
            typename TSubsample::MeasurementType medianGuess) ;

template< class TSubsample >
typename TSubsample::MeasurementType 
rfQuickSelect(TSubsample* sample,
            unsigned int activeDimension,
            int beginIndex, int endIndex,
            int kth) ;

template< class TSubsample >
void rfInsertSort(TSubsample* sample, 
                unsigned int activeDimension,
                int beginIndex, int endIndex) ;

template< class TSubsample >
void rfDownHeap(TSubsample* sample,
              unsigned int activeDimension,
              int beginIndex, int endIndex, int node) ;

template< class TSubsample >
void rfHeapSort(TSubsample* sample, 
                unsigned int activeDimension,
                int beginIndex, int endIndex) ;


template< class TSubsample >
void rfIntrospectiveSortLoop(TSubsample* sample, 
                                  unsigned int activeDimension,
                                  int beginIndex,
                                  int endIndex,
                                  int depthLimit, 
                                  int sizeThreshold) ;

template< class TSubsample >
void rfIntrospectiveSort(TSubsample* sample,
                       unsigned int activeDimension,
                       int beginIndex, int endIndex,
                       int sizeThreshold) ;

#include "rc_statisticsalgorithm.txx"

#endif
