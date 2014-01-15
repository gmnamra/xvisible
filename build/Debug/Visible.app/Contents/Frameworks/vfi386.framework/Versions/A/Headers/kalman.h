
#ifndef __RC_KALMAN_H__
#define __RC_KALMAN_H__

#include <rc_matrix_defs.h>
#include <rc_matrix_ops.h>
#include <rc_vector_funcs.h>
#include <rc_matrix_funcs.h>


/*
 * Generic Kalman filter implementation
 *
 * The Kalman filter is the optimal state estimator
 * for the following state (x) evolution and observation (z) equations
 *
 * x_1 = Ax_0 + q, q ~ N(r; 0, Q)
 * z = Hx + r, r ~ N(r; 0, R)
 * 
 * i.e. the state dynamics is linear with gaussian distributed noise (with
 * covariance Q); the observation is also linear (with noise covariance R).
 *
 * The state is preserved as mean x and covariance P. 
 * The state is update as follows:
 *
 * First dynamics update (using only previous state and dynamics)
 * x_1^ = Ax_0
 * P_1^ = A * P_0 * A' + Q
 *
 * Then observation update (incorporate dynamics)
 * K = P_1^ * H' / (H * P_1^ * H' + R);
 * x_1 = x_1^ + K * (z_1 - H * x_1^)
 * P_1 = (eye(size(K, 1)) - K * H) * P_1^;
 *
 * The effect is that if P_0 is small, then the gain matrix K would also be 
 * small, and so the state would depend more of dynamics than on the 
 * observation.
 */
     
template<int DX, int DZ>
class rcKalmanFilter
{
public:
  rcKalmanFilter();
  ~rcKalmanFilter();
  
  // Getters for system matrices
  const rc_Matrix<double, DX, DX>&
                    A() const;
  
  void A(rc_Matrix<double, DX, DX>& a)
  {
    _AA = a;
  }

  const rc_Matrix<double, DX, DX>&
                    Q() const;

  void Q(rc_Matrix<double, DX, DX>& q)
  {
    _QQ = q;
  }

  const rc_Matrix<double, DZ, DX>&
                    H() const;

  void H(rc_Matrix<double, DZ, DX>& h)
  {
    _H = h;
  }

  const rc_Matrix<double, DZ, DZ>&
                    R() const;

  void R(rc_Matrix<double, DZ, DZ>& r)
  {
    _RR = r;
  }

  
  // Getters for the current state
  const rcVector<double, DX>&
                    x() const;
  const rc_Matrix<double, DX, DX>&
                    P() const;

  const rc_Matrix<double, DX, DZ>&
                    K() const;
  

  // Initializer
  void              init(const rcVector<double, DX>& x0, 
                          const rc_Matrix<double, DX, DX>& p0);
  // Updates
  void              update(const rcVector<double, DZ>& z);
  void              update(const rcVector<double, DZ>& z, 
                            const rc_Matrix<double, DZ, DZ>& zR);
  // The second form allows for variable R
  
protected:
  
  void              _setupDerivedMatrices();

  rcVector<double, DX>
                    _x;
  rc_Matrix<double, DX, DX>
    _AA;
  rc_Matrix<double, DX, DX>
    _AT;
  rc_Matrix<double, DX, DX>
    _QQ;
  rc_Matrix<double, DX, DX>
    _PP;
  rc_Matrix<double, DX, DX>
    _II;
  rc_Matrix<double, DZ, DX>
                    _H;
  rc_Matrix<double, DX, DZ>
                    _HT;
  rc_Matrix<double, DZ, DZ>
                    _RR;
  rc_Matrix<double, DX, DZ>
                    _K;

};

// Inline functions
template<int DX, int DZ>
inline rcKalmanFilter<DX, DZ>::rcKalmanFilter()
{
}

template<int DX, int DZ>
inline rcKalmanFilter<DX, DZ>::~rcKalmanFilter()
{
}
  
template<int DX, int DZ>
inline const rc_Matrix<double, DX, DX>& rcKalmanFilter<DX, DZ>::A() const
{
  return _AA;
};

template<int DX, int DZ>
inline const rc_Matrix<double, DX, DX>& rcKalmanFilter<DX, DZ>::Q() const
{
  return _QQ;
};

template<int DX, int DZ>
inline const rc_Matrix<double, DZ, DX>& rcKalmanFilter<DX, DZ>::H() const
{
  return _HH;
};

template<int DX, int DZ>
inline const rc_Matrix<double, DZ, DZ>& rcKalmanFilter<DX, DZ>::R() const
{
  return _RR;
};

template<int DX, int DZ>
inline const rcVector<double, DX>& rcKalmanFilter<DX, DZ>::x() const
{
  return _x;
};

template<int DX, int DZ>
inline const rc_Matrix<double, DX, DX>& rcKalmanFilter<DX, DZ>::P() const
{
  return _PP;
};

template<int DX, int DZ>
inline const rc_Matrix<double, DX, DZ>& rcKalmanFilter<DX, DZ>::K() const
{
  return _K;
};
  

template<int DX, int DZ>
inline void rcKalmanFilter<DX, DZ>::init(const rcVector<double, DX>& x0, 
                          const rc_Matrix<double, DX, DX>& p0)
{
  _setupDerivedMatrices ();
  _x = x0;
  _PP = p0;
}

template<int DX, int DZ>
inline void rcKalmanFilter<DX, DZ>::update(const rcVector<double, DZ>& z)
{
  update(z, _RR);
};

template<int DX, int DZ>
inline void rcKalmanFilter<DX, DZ>::update(const rcVector<double, DZ>& z, 
                            const rc_Matrix<double, DZ, DZ>& zR)
{
  rcVector<double, DX>
                    x1_;
  rc_Matrix<double, DX, DX>
                    P1_;
  
  x1_ = _AA * _x;
  P1_ = _AA * _PP * _AT + _QQ;
  
  _K = (P1_ * _HT) * rfMInvert(_H * P1_ * _HT + zR);
  _x = x1_ + _K * (z - _H * x1_);
  _PP = (_II - _K * _H) * P1_;
}
  // The second form allows for variable R
  
template<int DX, int DZ>
inline void rcKalmanFilter<DX, DZ>::_setupDerivedMatrices()
{
  _AT = rfMTranspose(_AA);
  _HT = rfMTranspose(_H);
  _II = 0;
  rfMTrace(_II) = 1;
}


/*
 * An instantiation of the kalman filter for the first order systems 
 * (that is other than the observation "state", the state also contains the 
 * velocity)
 *
 */
template<int D>
class rcKalmanFilterFirstOrder : public rcKalmanFilter<D * 2, D>
{
public:
  rcKalmanFilterFirstOrder();
  ~rcKalmanFilterFirstOrder();
  
  void              setup(const rc_Matrix<double, D * 2, D * 2>& Q_,
                              const rc_Matrix<double, D, D>& R_);
  
};

template<int D>
inline rcKalmanFilterFirstOrder<D>::rcKalmanFilterFirstOrder()
  :
  rcKalmanFilter<D * 2, D>()
{
};

template<int D>
inline rcKalmanFilterFirstOrder<D>::~rcKalmanFilterFirstOrder()
{
};

template<int D>
inline void rcKalmanFilterFirstOrder<D>::setup(
                              const rc_Matrix<double, D * 2, D * 2>& Q_,
                              const rc_Matrix<double, D, D>& R_)
{
  _AA = 0;
  rfMTrace(_AA) = 1;
  rcMatrixRange<double>
    velMod = _AA.range(0, D, D, D);
  rfMTrace(velMod) = 1;
  
  _HH = 0;
  rcMatrixRange<double>
    proj = _HH.range(0, 0, D, D);
  rfMTrace(proj) = 1;

  _RR = R_;
  _QQ = Q_;
  _setupDerivedMatrices();
};

#endif
