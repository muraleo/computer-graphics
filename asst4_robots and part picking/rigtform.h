#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <iostream>
#include <cassert>

#include "matrix4.h"
#include "quat.h"

class RigTForm {
  Cvec3 t_; // translation component
  Quat r_;  // rotation component represented as a quaternion

public:
  RigTForm() : t_(0) {
    assert(norm2(Quat(1,0,0,0) - r_) < CS175_EPS2);
  }

  RigTForm(const Cvec3& t, const Quat& r) {
    // ======
	// TODO:
	// ======
    t_ = t;                              //---------------change
    r_ = r;                              //---------------change
  }

  explicit RigTForm(const Cvec3& t) {
    // ======
	// TODO:
	// ======
    t_ = t;                             //---------------change
  }

  explicit RigTForm(const Quat& r) {
    // ======
	// TODO:
	// ======
    r_ = r;                             //---------------change
  }

  Cvec3 getTranslation() const {
    return t_;
  }

  Quat getRotation() const {
    return r_;
  }

  RigTForm& setTranslation(const Cvec3& t) {
    t_ = t;
    return *this;
  }

  RigTForm& setRotation(const Quat& r) {
    r_ = r;
    return *this;
  }

  Cvec4 operator * (const Cvec4& a) const {
    // ======
	// TODO: Implement : A.r * c + A.t in page 70 of text book. * a
	// ======
       Cvec4 temp1(t_, 0.0);                             //---------------change
       Cvec4 temp2 = temp1 + r_ * a;                             //---------------change
       return temp2;                             //---------------change
  }

  RigTForm operator * (const RigTForm& a) const {
    // ======
	// TODO: Implement : new rigid transform with translation t1 + r1t2 and rotation r1r2, a is 2                             //---------------change
	// ======
       Cvec4 temp1(a.t_, 0.0); // t2 
       Cvec4 temp2 = r_ * temp1; // r1 * t2
       Cvec3 t_new(t_(0) + temp2(0), t_(1) + temp2(1), t_(2) + temp2(2) ); // t1 + r1t2
       RigTForm rtf(t_new, r_*a.r_);
       return rtf;
  }
};

inline RigTForm inv(const RigTForm& tform) {
    // ======
	// TODO:new rigid body transform with translation r^−1t and rotation r^−1                             //---------------change
	// ======
      Quat r = tform.getRotation(); // get r
      Cvec4 t(tform.getTranslation(),1.0); // get t
      Quat r_inv = inv(r); // get r^-1
      Cvec4 temp = r_inv *(-t);
      Cvec3 temp2(temp(0), temp(1), temp(2)); // −r^−1t
      RigTForm rtf(temp2, r_inv);
      return rtf;
    }

inline RigTForm transFact(const RigTForm& tform) {
  return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform) {
  return RigTForm(tform.getRotation());
}

inline Matrix4 rigTFormToMatrix(const RigTForm& tform) {
    // ======
	// TODO:                             //---------------change
	// ======
    //return m;
    Matrix4 M = quatToMatrix(tform.getRotation());
    //Matrix4 T(0.0);
    //T(0,3) = tform.t_(0);
    //T(1,3) = tform.t_(1);
    //T(2,3) = tform.t_(2);
    Matrix4 T = Matrix4::makeTranslation(tform.getTranslation());
    return T * M;
}

#endif
