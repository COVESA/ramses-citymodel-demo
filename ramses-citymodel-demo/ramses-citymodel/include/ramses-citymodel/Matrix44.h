//  -------------------------------------------------------------------------
//  Copyright (C) 2019 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES Matrix44f

#ifndef RAMSES_CITYMODELMATRIX44_H
#define RAMSES_CITYMODELMATRIX44_H

#include "Vector3.h"
#include "Vector4.h"

/// Matrix44 class.
class Matrix44
{
public:
    Matrix44();
    Matrix44(float m11, float m12, float m13, float m14,
             float m21, float m22, float m23, float m24,
             float m31, float m32, float m33, float m34,
             float m41, float m42, float m43, float m44);

    Vector3 operator*(const Vector3& v) const;
    Vector4 operator*(const Vector4& v) const;
    Matrix44 operator*(const Matrix44& mat) const;
    Matrix44 transpose() const;
    float determinant() const;
    Matrix44 inverse() const;
    bool toRotationEulerZYX(Vector3& xyz) const;

    Vector3 getTranslationVector() const;
    static Matrix44 RotationEulerXYZ(const Vector3& rotationXYZ);

private:
    float m_m11;
    float m_m21;
    float m_m31;
    float m_m41;
    float m_m12;
    float m_m22;
    float m_m32;
    float m_m42;
    float m_m13;
    float m_m23;
    float m_m33;
    float m_m43;
    float m_m14;
    float m_m24;
    float m_m34;
    float m_m44;
};

#endif
