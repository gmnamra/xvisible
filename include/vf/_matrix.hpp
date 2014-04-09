/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.6  2004/01/29 18:10:42  arman
 *added float overload to matrix multiply
 *
 *Revision 1.5  2003/08/18 21:32:15  arman
 *fixed an overloading bug (inverse and one of the ctor forms)
 *
 *Revision 1.4  2003/08/18 17:45:50  arman
 *added a new ctor and output operator
 *
 *Revision 1.3  2003/04/08 19:18:30  sami
 *Inlining changes
 *
 *Revision 1.2  2003/03/04 18:39:36  arman
 *added inverse
 *
 *Revision 1.1  2003/03/04 15:55:55  arman
 *Matrix class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcMATRIX_H
#define __rcMATRIX_H

#include <rc_pair.h>
#include <ostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <rc_vector2d.h>

namespace ublas = ublas;

using namespace boost;
#define DIM 2

#define e11 element(0,0)
#define e12 element(0,1)
#define e13 element(0,2)
#define e21 element(1,0)
#define e22 element(1,1)
#define e23 element(1,2)
#define e31 element(2,0)
#define e32 element(2,1)
#define e33 element(2,2)


  // A General Templated Matrix Class (pieced together from various open sources)
  // Note on notation:
  //   1) In accessors, matrix elements have indices starting at
  // zero.
  //   2) Indices are ordered row first, column second

  class rcMatrix_2d : public boost::numeric::ublas::c_matrix< double, 2, 2 >
  /* Named degrees of freedom:
   *    There are various ways of specifying the 4 degrees of freedom represented
   * by a 2x2 matrix.  Two sets of four "named" degrees of freedom are
   * implemented for constructors and accessors:
   *   1) xRot, yRot, xScale, yScale     (Rx, Ry, Sx, Sy)
   *   2) scale, aspect, shear, rotation  (s, a, K, R)
   *
   * A 2x2 matrix is defined as follows in terms of these variables:
   *   |e11 e12|   |Sx(cosRx)  -Sy(sinRy)|   |s(cosR)  as(-sinR - cosR tanK)|
   *   |e21 e22| = |Sx(sinRx)   Sy(cosRy)| = |s(sinR)  as( cosR - sinR tanK)|
   *
   * The composition order is:
   *   |Sx(cosRx)  -Sy(sinRy)|   |cosRx  -sinRy| |Sx  0 |
   *   |Sx(sinRx)   Sy(cosRy)| = |sinRx   cosRy| |0   Sy|
   *
   * Or:
   *   |s(cosR)  as(-sinR - cosR tanK)|       |cosR  -sinR| |1  -tanK| |1  0|
   *   |s(sinR)  as( cosR - sinR tanK)| = s * |sinR   cosR| |0    1  | |0  a|
   *
   * The named degrees of freedom are extracted from a matrix as follows:
   *   Rx = atan2(e21, e11)           s = sqrt(e11*e11 + e21*e21)
   *   Ry = atan2(-e12, e22)          a = det / (e11*e11 + e21*e21)
   *   Sx = sqrt(e11*e11 + e21*e21)   K = -atan((e11*e12 + e21*e22) / det)
   *   Sy = sqrt(e12*e12 + e22*e22)   R = atan2(e21, e11)
   *                                  Note: det = a * (s^2)
   * Note that none of these is meaningful if the matrix is singular, even though
   * some values might be well-defined by the above formulas.
   *
   * Also note that the extraction formulas define the canonical ranges for the
   * variables (for a non-singular matrix), as follows:
   *   Rx [-PI, PI]                     s (0, +Inf)
   *   Ry [-PI, PI]                     R [-PI, PI]
   *   Sx (0, +Inf)                     a (-Inf, +Inf), a != 0
   *   Sy (0, +Inf)                     K (-PI/2, PI/2)
   *
   * Be careful if you use both the (Rx, Ry, Sx, Sy) style of
   * decomposition and the (s, a, K, R) style.  There are some
   * non-obvious interactions between the two.  For example, when shear
   * is present, aspect != yScale/xScale.
   */
  {
  public:

    typedef boost::numeric::ublas::c_matrix< double, 2, 2 > matrix_type;

    // default copy ctor, assign, dtor OK
    // Constructors. Default is the identity matrix
    rcMatrix_2d() : mDt(0)
    {
      matrix_type (2,2);
    }


    rcMatrix_2d(double f11, double f12, double f21, double f22) : mDt (0)
    {
      matrix_type (2,2);
      matrix_type::insert_element (0,0,f11);
      matrix_type::insert_element (0,1,f12);
      matrix_type::insert_element (1,0,f21);
      matrix_type::insert_element (1,1,f22);
      setdt();
    }

    rcMatrix_2d(const rcDPair& rot, const rcDPair& scale)
    {
      matrix_type(2,2);

		matrix_type::insert_element (0,0,scale.x() * std::cos(rot.x()));
		matrix_type::insert_element (0,0,-scale.y() * std::sin(rot.y()));
		matrix_type::insert_element (0,0,scale.x() * std::sin(rot.x()));
		matrix_type::insert_element (0,0,scale.y() * std::cos(rot.y()));
      setdt();
    }

    rcMatrix_2d(const rcRadian& rot, const rcDPair& scale)
    {
      matrix_type (2,2);
      double cosR = cos(rot);
      double sinR = sin(rot);

      matrix_type::insert_element (0,0, scale.x() * cosR);
      matrix_type::insert_element (0,0,  -scale.y() * sinR);
      matrix_type::insert_element (0,0,  scale.x() * sinR);
      matrix_type::insert_element (0,0,  scale.y() * cosR);
    }

    rcMatrix_2d(double scale, double aspect,
             const double& shear, const rcRadian& rotation)
    {
      matrix_type (2,2);
      double cosR = cos(rotation);
      double sinR = sin(rotation);
		double tanK = std::tan(shear);
      matrix_type::insert_element (0,0, scale * cosR);
      matrix_type::insert_element (0,0, aspect * scale * (- sinR - cosR * tanK ));
      matrix_type::insert_element (0,0,  scale * sinR);
      matrix_type::insert_element (0,0,  aspect * scale * (  cosR - sinR * tanK ));
      setdt();
    }

    double determinant() const { return mDt;}

    rcMatrix_2d inverse() const
    {
      if (isSingular()) throw general_exception ("is Singular");
      double idt = 1. / mDt; // do division once
      return rcMatrix_2d( e22 * idt, -e12 * idt,
                          -e21 * idt,  e11 * idt);
    }


    bool isSingular() const
    {return (mDt == 0.); }

    bool isIdentity() const
    {return (e11 == 1. && e12 == 0. &&
             e21 == 0. && e22 == 1.);
    }

    rcMatrix_2d transpose() const
    { return rcMatrix_2d(e11, e21, e12, e22); }


    double element(int32 row,int32 column) const
    {
      return matrix_type::operator() (row,  column);
    }

    void   element(int32 row,int32 column, double value)
    {
      matrix_type::insert_element (row, column, value);
    }

    /*
     * effect   get/set element value
     * requires row and column 0 or 1
     * note     The setter recalculates the determinant
     */
    // Decomposition methods
    // All check for singularity throw rcMathError::Singular if so

    // Note xRot,yRot,xScale,yScale are a group.
    double xRot() const
    {
      if (isSingular()) throw general_exception ("Singular");
      // Full angle so use arctan(y,x) = atan2(y,x)
      return atan2(e21, e11);
    }

    double yRot() const
    {
      if (isSingular()) throw general_exception ("Singular");
      // Full angle so use arctan(y,x) = atan2(y,x)
      return atan2(-e12, e22);
    }

    double xScale() const
    {
      if (isSingular()) throw general_exception ("Singular");
      return sqrt(rmSquare(e11) + rmSquare(e21));
    }

    double yScale() const
    {
      if (isSingular()) throw general_exception ("Singular");
      return sqrt(rmSquare(e12) + rmSquare(e22));
    }

    // Note scale,aspect,shear,rotation are a group.
    double scale() const
    {
      if (isSingular()) throw general_exception ("Singular");
      return sqrt(rmSquare(e11) + rmSquare(e21));
    }

    double aspect() const
    {
      if (isSingular()) throw general_exception ("Singular");
      return determinant() / (rmSquare(e11) + rmSquare(e21));
    }

    double shear() const
    {
      if (isSingular()) throw general_exception ("Singular");
      // Shear is limited to (-PI/2, PI/2) so use arctan(x) = atan(x)
      return ( -atan((e11 * e12 + e21 * e22) / determinant()) );
    }

    rcRadian rotation() const
    {
      if (isSingular()) throw general_exception ("Singular");
      return arctan (e21, e11);
    }


    // Decomposition methods


    /*
     * effect scale, aspect, shear, rotation are another way.
     * throws general_exception::Singular if matrix is singular
     */
    // Identity matrix.
    static const rcMatrix_2d I;
    //	static boost::numeric::ublas::identity_matrix<double,2,2> Ib;

    inline rcMatrix_2d  operator+(const rcMatrix_2d&) const;
    inline rcMatrix_2d& operator+=(const rcMatrix_2d&);

    inline rcMatrix_2d  operator-() const;
    inline rcMatrix_2d  operator-(const rcMatrix_2d&) const;
    inline rcMatrix_2d& operator-=(const rcMatrix_2d&);

    inline rcMatrix_2d  operator*(double) const;
    inline rcMatrix_2d& operator*=(double);

    inline rcMatrix_2d  operator*(const rcMatrix_2d& other) const
	  {
		  return (rcMatrix_2d) (*this * other);
	  }
    inline rcMatrix_2d& operator*=(const rcMatrix_2d&);

    inline rcMatrix_2d  operator/(double) const;
    inline rcMatrix_2d& operator/=(double);

    inline rcMatrix_2d  operator/(const rcMatrix_2d&) const;
    inline rcMatrix_2d& operator/=(const rcMatrix_2d&);
    /*
     * note   m1 / m2 = m1 * m2.inverse()
     * throws general_exception::Singular if the matrix used as a divisor
     *        is singular
     */
    inline bool operator==(const rcMatrix_2d& other) const
	  {
		  return *this == other;
	  }

    inline bool operator!=(const rcMatrix_2d& other) const
	  {
		  return *this != other;
	  }

		  // Vector operations.
	template<typename V>
    rc2dVector<V> operator*(const rc2dVector<V>& v) const
	  {
		  return rc2dVector<V> ((V) (e11 * v.x() + e12 * v.y()) , (V) (e21 * v.x() + e22 * v.y()) );
	  }

    friend ostream& operator<< (ostream& ous, const rcMatrix_2d& dis)
    {
      ous << "[" << dis.element (0,0) << " " << dis.element(0,1) << endl << dis.element(1,0) << " " << dis.element(1,1) << "]" << endl;
      return ous;
    }

  private:
    double mDt;	    // Determinant
    void setdt()
    { mDt = e11 * e22 - e21 * e12;}
  };


#undef DIM
#undef e11 element(0,0)
#undef e12 element(0,1)
#undef e13 element(0,2)
#undef e21 element(1,0)
#undef e22 element(1,1)
#undef e23 element(1,2)
#undef e31 element(2,0)
#undef e32 element(2,1)
#undef e33 element(2,2)


#endif /* __rcMATRIX_H */
