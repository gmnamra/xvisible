/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      ut_generator.h
 *   Creation Date  09/09/2003
 *   Author         Peter Roberts
 *
 ***************************************************************************/

#ifndef __UT_GENERATOR_H
#define __UT_GENERATOR_H


#include <rc_unittest.h>
#include <rc_def_generators.h>
#include <rc_lin_generators.h>
#include <rc_cubic_generators.h>

class UT_generator : public rcUnitTest {
 public:

  UT_generator(std::string nearestName, std::string bilinName, std::string bicubeName);
  ~UT_generator();

  virtual uint32 run();

 private:
  void def();
  void bilinear();
  void cubic();
  void rotate();
  void genImages();
  void project ();

  std::string _nearestName, _bilinName, _bicubeName;
};

#endif /* __UT_GENERATOR_H */
