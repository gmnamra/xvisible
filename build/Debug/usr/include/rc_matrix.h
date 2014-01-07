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
#include <rc_vector2d.h>
#include <ostream>

// A General Templated Matrix Class (pieced together from various open sources)
// Note on notation:
//   1) In accessors, matrix elements have indices starting at
// zero.  
//   2) Indices are ordered row first, column second

template <int32 D> class rcMatrix;  /* D is the dimension */

typedef rcMatrix<2> rcMatrix_2d;

#define e11 element(0,0)
#define e12 element(0,1)
#define e13 element(0,2)
#define e21 element(1,0)
#define e22 element(1,1)
#define e23 element(1,2)
#define e31 element(2,0)
#define e32 element(2,1)
#define e33 element(2,2)

template <>
class rcMatrix<2>
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

  // default copy ctor, assign, dtor OK
  // Constructors. Default is the identity matrix
  rcMatrix() { me[0][0] = 1.; me[0][1] = 0.;
               me[1][0] = 0.; me[1][1] = 1.; mDt = 1.;}

  rcMatrix(double m11, double m12, double m21, double m22)
  { 
    me[0][0] = m11; me[0][1] = m12;
    me[1][0] = m21; me[1][1] = m22; setdt();
  }

  rcMatrix(const rcDPair& rot, const rcDPair& scale)
    {

      me[0][0] = scale.x() * std::cos(rot.x());
      me[1][0] = scale.y() * std::sin(rot.y());
      me[0][1] = scale.x() * std::sin(rot.x());
      me[1][1] = scale.y() * std::cos(rot.y());
      setdt();
    }

  rcMatrix(const rcRadian& rot, const rcDPair& scale)
    {
      double cosR = cos(rot);
      double sinR = sin(rot);

      me[0][0] =  scale.x() * cosR;
      me[1][0] = -scale.y() * sinR;
      me[0][1] = scale.x() * sinR;
      me[1][1] = scale.y() * cosR;
      setdt ();
    }


  rcMatrix(double scale, double aspect,
           const double& shear, const rcRadian& rotation);

  double determinant() const { return mDt;}

  rcMatrix<2> inverse() const
  {
    if (isSingular()) throw general_exception ("is Singular");
    double idt = 1. / mDt; // do division once
    return rcMatrix<2> ( e22 * idt, -e12 * idt,
		       -e21 * idt,  e11 * idt);
   }

  /*
   * throws general_exception::Singular if matrix is singular
   */
  rcMatrix<2> transpose() const;
  bool isSingular() const;
  bool isIdentity() const;

  inline double element(int32 row,int32 column) const;
  void   element(int32 row,int32 column, double value);
  /*
   * effect   get/set element value
   * requires row and column 0 or 1
   * note     The setter recalculates the determinant
   */


  // Decomposition methods
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

  /*
   * effect xRot, yRot, xScale, yScale are one way to dissect a transform.
   * throws general_exception::Singular if matrix is singular
   */

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

  /*
   * effect scale, aspect, shear, rotation are another way.
   * throws general_exception::Singular if matrix is singular
   */
  // Identity matrix.
  static const rcMatrix<2> I;

  inline rcMatrix<2>  operator+(const rcMatrix<2>&) const;
  inline rcMatrix<2>& operator+=(const rcMatrix<2>&);

  inline rcMatrix<2>  operator-() const;
  inline rcMatrix<2>  operator-(const rcMatrix<2>&) const;
  inline rcMatrix<2>& operator-=(const rcMatrix<2>&);

  inline rcMatrix<2>  operator*(double) const;
  inline rcMatrix<2>& operator*=(double);

  inline rcMatrix<2>  operator*(const rcMatrix<2>&) const;
  inline rcMatrix<2>& operator*=(const rcMatrix<2>&);

  inline rcMatrix<2>  operator/(double) const;
  inline rcMatrix<2>& operator/=(double);

  inline rcMatrix<2>  operator/(const rcMatrix<2>&) const;
  inline rcMatrix<2>& operator/=(const rcMatrix<2>&);
  /*
   * note   m1 / m2 = m1 * m2.inverse()
   * throws general_exception::Singular if the matrix used as a divisor
   *        is singular
   */
  inline bool operator==(const rcMatrix<2>&) const;
  inline bool operator!=(const rcMatrix<2>&) const;

  // Vector operations.
  inline rc2Dvector operator*(const rc2Dvector&) const;
  inline friend rc2Dvector operator*(const rc2Dvector&, const rcMatrix<2>&);

  inline rc2Fvector operator*(const rc2Fvector&) const;
  inline friend rc2Fvector operator*(const rc2Fvector&, const rcMatrix<2>&);

  friend ostream& operator<< (ostream& ous, const rcMatrix<2>& dis)
  {
    ous << "[" << dis.element (0,0) << " " << dis.element(0,1) << endl << dis.element(1,0) << " " << dis.element(1,1) << "]" << endl;
    return ous;
  }

private:
  double me[2][2];  // Matrix elements.
  double mDt;	    // Determinant
  void setdt() { mDt = me[0][0] * me[1][1] - me[0][1] * me[1][0];}
};

inline double rcMatrix<2>::element(int32 row,int32 column) const
{ assert((row == 0 || row == 1) && (column == 0 || column == 1));
  return me[row][column];
}

inline bool rcMatrix<2>::isSingular() const
 {return (mDt == 0.); }

inline bool rcMatrix<2>::isIdentity() const
{return (me[0][0] == 1. && me[0][1] == 0. &&
	 me[1][0] == 0. && me[1][1] == 1.);
}

inline rcMatrix<2> rcMatrix<2>::transpose() const
{ return rcMatrix<2>(me[0][0], me[1][0], me[0][1], me[1][1]); }

inline rcMatrix<2>& rcMatrix<2>::operator+=(const rcMatrix<2>& m)
{ return *this = *this + m; }

inline rcMatrix<2> rcMatrix<2>::operator-() const
{ return rcMatrix<2>(-me[0][0], -me[0][1],
		     -me[1][0], -me[1][1]);
}

inline rcMatrix<2>& rcMatrix<2>::operator-=(const rcMatrix<2>& m)
{ return *this = *this - m; }

inline rcMatrix<2>& rcMatrix<2>::operator*=(double s)
{ return *this = *this * s; }

inline rcMatrix<2>& rcMatrix<2>::operator*=(const rcMatrix<2>& m)
{ return *this = *this * m; }

inline rcMatrix<2> rcMatrix<2>::operator/(const rcMatrix<2>& m) const
{ return (*this) * m.inverse(); }

inline rcMatrix<2>& rcMatrix<2>::operator/=(const rcMatrix<2>& m)
{ return *this *= m.inverse(); }

inline rcMatrix<2> rcMatrix<2>::operator/(double s) const
{ return *this * (1. / s); }  // Do division once

inline rcMatrix<2>& rcMatrix<2>::operator/=(double s)
{ return *this = *this / s; }

inline rcMatrix<2> operator*(double s, const rcMatrix<2>& m)
{return m * s;}

inline rcMatrix<2> operator/(double s, const rcMatrix<2>& m)
{ return m.inverse() * s;}

inline rc2Dvector& operator*=(rc2Dvector& v, const rcMatrix<2>& m)
{ return v = v * m;}

inline rc2Dvector operator/(const rc2Dvector& v, const rcMatrix<2>& m)
{ return v * m.inverse();}

inline rc2Dvector& operator/=(rc2Dvector& v, const rcMatrix<2>& m)
{ return v = v * m.inverse();}

/*	************************
	*                      *
	*     Arithmetic       *
	*                      *
	************************
*/

inline bool rcMatrix<2>::operator == (const rcMatrix<2>& m) const
{ return (me[0][0] == m.me[0][0] && me[0][1] == m.me[0][1] &&
	  me[1][0] == m.me[1][0] && me[1][1] == m.me[1][1]);
}

inline bool rcMatrix<2>::operator != (const rcMatrix<2>& m) const
{ return (!(*this==m)); }

inline rcMatrix<2> rcMatrix<2>::operator + (const rcMatrix<2>& m) const
{ return rcMatrix<2>(me[0][0] + m.me[0][0], me[0][1] + m.me[0][1],
                     me[1][0] + m.me[1][0], me[1][1] + m.me[1][1]);
}

inline rcMatrix<2> rcMatrix<2>::operator - (const rcMatrix<2>& m) const
{ return rcMatrix<2>(me[0][0] - m.me[0][0], me[0][1] - m.me[0][1],
                     me[1][0] - m.me[1][0], me[1][1] - m.me[1][1]);
}

inline rcMatrix<2> rcMatrix<2>::operator * (double s) const
{ return rcMatrix<2>(me[0][0] * s, me[0][1] * s, me[1][0] * s, me[1][1] * s); }

inline rcMatrix<2> rcMatrix<2>::operator * (const rcMatrix<2>& m) const
{ return rcMatrix<2>(me[0][0]*m.me[0][0] + me[0][1]*m.me[1][0],
		     me[0][0]*m.me[0][1] + me[0][1]*m.me[1][1],
		     me[1][0]*m.me[0][0] + me[1][1]*m.me[1][0],
		     me[1][0]*m.me[0][1] + me[1][1]*m.me[1][1]);
}

inline rc2Dvector rcMatrix<2>::operator * (const rc2Dvector& v) const
{ return rc2Dvector(me[0][0] * v.x() + me[0][1] * v.y(),
		     me[1][0] * v.x() + me[1][1] * v.y());
}

inline rc2Dvector operator * (const rc2Dvector& v, const rcMatrix<2>& m)
{ return rc2Dvector(m.me[0][0] * v.x() + m.me[1][0] * v.y(),
		     m.me[0][1] * v.x() + m.me[1][1] * v.y());
}

inline rc2Fvector rcMatrix<2>::operator * (const rc2Fvector& v) const
{ return rc2Fvector((float) (me[0][0] * v.x() + me[0][1] * v.y()),
		    (float) (me[1][0] * v.x() + me[1][1] * v.y()));
}

inline rc2Fvector operator * (const rc2Fvector& v, const rcMatrix<2>& m)
{ return rc2Fvector((float) (m.me[0][0] * v.x() + m.me[1][0] * v.y()),
		    (float) (m.me[0][1] * v.x() + m.me[1][1] * v.y()));
}

#endif /* __rcMATRIX_H */
