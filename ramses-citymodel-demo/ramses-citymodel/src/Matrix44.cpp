//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Matrix44.h"
#include "ramses-citymodel/Math.h"

#include "stdint.h"
#include "math.h"

Matrix44::Matrix44()
{
    m_m11 = 1.0f;
    m_m21 = 0.0f;
    m_m31 = 0.0f;
    m_m41 = 0.0f;

    m_m12 = 0.0f;
    m_m22 = 1.0f;
    m_m32 = 0.0f;
    m_m42 = 0.0f;

    m_m13 = 0.0f;
    m_m23 = 0.0f;
    m_m33 = 1.0f;
    m_m43 = 0.0f;

    m_m14 = 0.0f;
    m_m24 = 0.0f;
    m_m34 = 0.0f;
    m_m44 = 1.0f;
}

Matrix44::Matrix44(float m11, float m12, float m13, float m14,
                   float m21, float m22, float m23, float m24,
                   float m31, float m32, float m33, float m34,
                   float m41, float m42, float m43, float m44)
{
    m_m11 = m11;
    m_m21 = m21;
    m_m31 = m31;
    m_m41 = m41;

    m_m12 = m12;
    m_m22 = m22;
    m_m32 = m32;
    m_m42 = m42;

    m_m13 = m13;
    m_m23 = m23;
    m_m33 = m33;
    m_m43 = m43;

    m_m14 = m14;
    m_m24 = m24;
    m_m34 = m34;
    m_m44 = m44;
}

Vector3 Matrix44::operator*(const Vector3& v) const
{
    return Vector3(m_m11 * v.getX() + m_m12 * v.getY() + m_m13 * v.getZ() + m_m14,
                   m_m21 * v.getX() + m_m22 * v.getY() + m_m23 * v.getZ() + m_m24,
                   m_m31 * v.getX() + m_m32 * v.getY() + m_m33 * v.getZ() + m_m34);
}

Vector4 Matrix44::operator*(const Vector4& v) const
{
    return Vector4(m_m11 * v.getX() + m_m12 * v.getY() + m_m13 * v.getZ() + m_m14 * v.getW(),
                   m_m21 * v.getX() + m_m22 * v.getY() + m_m23 * v.getZ() + m_m24 * v.getW(),
                   m_m31 * v.getX() + m_m32 * v.getY() + m_m33 * v.getZ() + m_m34 * v.getW(),
                   m_m41 * v.getX() + m_m42 * v.getY() + m_m43 * v.getZ() + m_m44 * v.getW());
}

Matrix44 Matrix44::operator*(const Matrix44& mat) const
{
    return Matrix44(  m_m11 * mat.m_m11 + m_m12 * mat.m_m21 + m_m13 * mat.m_m31 + m_m14 * mat.m_m41, m_m11 * mat.m_m12 + m_m12 * mat.m_m22 + m_m13 * mat.m_m32 + m_m14 * mat.m_m42, m_m11 * mat.m_m13 + m_m12 * mat.m_m23 + m_m13 * mat.m_m33 + m_m14 * mat.m_m43, m_m11 * mat.m_m14 + m_m12 * mat.m_m24 + m_m13 * mat.m_m34 + m_m14 * mat.m_m44
                    , m_m21 * mat.m_m11 + m_m22 * mat.m_m21 + m_m23 * mat.m_m31 + m_m24 * mat.m_m41, m_m21 * mat.m_m12 + m_m22 * mat.m_m22 + m_m23 * mat.m_m32 + m_m24 * mat.m_m42, m_m21 * mat.m_m13 + m_m22 * mat.m_m23 + m_m23 * mat.m_m33 + m_m24 * mat.m_m43, m_m21 * mat.m_m14 + m_m22 * mat.m_m24 + m_m23 * mat.m_m34 + m_m24 * mat.m_m44
                    , m_m31 * mat.m_m11 + m_m32 * mat.m_m21 + m_m33 * mat.m_m31 + m_m34 * mat.m_m41, m_m31 * mat.m_m12 + m_m32 * mat.m_m22 + m_m33 * mat.m_m32 + m_m34 * mat.m_m42, m_m31 * mat.m_m13 + m_m32 * mat.m_m23 + m_m33 * mat.m_m33 + m_m34 * mat.m_m43, m_m31 * mat.m_m14 + m_m32 * mat.m_m24 + m_m33 * mat.m_m34 + m_m34 * mat.m_m44
                    , m_m41 * mat.m_m11 + m_m42 * mat.m_m21 + m_m43 * mat.m_m31 + m_m44 * mat.m_m41, m_m41 * mat.m_m12 + m_m42 * mat.m_m22 + m_m43 * mat.m_m32 + m_m44 * mat.m_m42, m_m41 * mat.m_m13 + m_m42 * mat.m_m23 + m_m43 * mat.m_m33 + m_m44 * mat.m_m43, m_m41 * mat.m_m14 + m_m42 * mat.m_m24 + m_m43 * mat.m_m34 + m_m44 * mat.m_m44);
}

Matrix44 Matrix44::transpose() const
{
    return Matrix44(m_m11,  m_m21,  m_m31,  m_m41,
                    m_m12,  m_m22,  m_m32,  m_m42,
                    m_m13,  m_m23,  m_m33,  m_m43,
                    m_m14,  m_m24,  m_m34,  m_m44);
}

Matrix44 Matrix44::RotationEulerXYZ(const Vector3& rotationXYZ)
{
    const float rotX = Math::Deg2Rad(rotationXYZ.getX());
    const float rotY = Math::Deg2Rad(rotationXYZ.getY());
    const float rotZ = Math::Deg2Rad(rotationXYZ.getZ());

    const float sinX = sin(rotX);
    const float cosX = cos(rotX);
    const float sinY = sin(rotY);
    const float cosY = cos(rotY);
    const float sinZ = sin(rotZ);
    const float cosZ = cos(rotZ);

    return Matrix44(
        cosY * cosZ,                      cosY * sinZ,                      -sinY      , 0.0f,
        sinX * sinY * cosZ - cosX * sinZ, cosX * cosZ + sinX * sinY * sinZ, sinX * cosY, 0.0f,
        sinX * sinZ + cosX * sinY * cosZ, cosX * sinY * sinZ - sinX * cosZ, cosX * cosY ,0.0f,
        0.0f,                             0.0f,                             0.0f,        1.0f);
}

bool Matrix44::toRotationEulerZYX(Vector3& xyz) const
{
    if (m_m31 < 1.0f)
    {
        if (m_m31 > -1.0f)
        {
            xyz = Vector3(-Math::Rad2Deg(atan2(m_m32, m_m33)),
                          -Math::Rad2Deg(asin(-m_m31)),
                          -Math::Rad2Deg(atan2(m_m21, m_m11)));
            return true;
        }
        else
        {
            xyz = Vector3(0.0f, -Math::Rad2Deg(Math::Pi() / 2.0f), Math::Rad2Deg(atan2(-m_m23, m_m22)));
            return false;
        }
    }
    else
    {
        xyz = Vector3(0.0f, Math::Rad2Deg(Math::Pi() / 2.0f), -Math::Rad2Deg(atan2(-m_m23, m_m22)));
        return false;
    }
}

float Matrix44::determinant() const
{
    const float m13m21 = m_m13 * m_m21;
    const float m13m22 = m_m13 * m_m22;
    const float m13m24 = m_m13 * m_m24;
    const float m11m22 = m_m11 * m_m22;
    const float m11m23 = m_m11 * m_m23;
    const float m11m24 = m_m11 * m_m24;
    const float m12m21 = m_m12 * m_m21;
    const float m14m21 = m_m12 * m_m21;
    const float m12m23 = m_m12 * m_m23;
    const float m14m22 = m_m14 * m_m22;
    const float m12m24 = m_m12 * m_m24;
    const float m14m23 = m_m14 * m_m23;
    const float m33m44 = m_m33 * m_m44;
    const float m34m43 = m_m34 * m_m43;
    const float m32m44 = m_m32 * m_m44;
    const float m33m42 = m_m33 * m_m42;
    const float m34m42 = m_m34 * m_m42;
    const float m31m44 = m_m31 * m_m44;
    const float m34m41 = m_m34 * m_m41;
    const float m32m43 = m_m32 * m_m43;
    const float m31m43 = m_m31 * m_m43;
    const float m33m41 = m_m33 * m_m41;
    const float m31m42 = m_m31 * m_m42;
    const float m32m41 = m_m32 * m_m41;

    return m11m22 * m33m44 + m11m23 * m34m42 + m11m24 * m32m43 + m12m21 * m34m43 + m12m23 * m31m44 + m12m24 * m33m41 +
           m13m21 * m32m44 + m13m22 * m34m41 + m13m24 * m31m42 + m14m21 * m33m42 + m14m22 * m31m43 + m14m23 * m32m41 -
           m11m22 * m34m43 - m11m23 * m32m44 - m11m24 * m33m42 - m12m21 * m33m44 - m12m23 * m34m41 - m12m24 * m31m43 -
           m13m21 * m34m42 - m13m22 * m31m44 - m13m24 * m32m41 - m14m21 * m32m43 - m14m22 * m33m41 - m14m23 * m31m42;
}

Matrix44 Matrix44::inverse() const
{
    const float det = determinant();

    if (det == 0.0f)
    {
        return Matrix44(0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f);
    }

    const float invDet = 1.0f / det;

    return Matrix44(
        (m_m22 * m_m44 * m_m33 - m_m22 * m_m43 * m_m34 - m_m44 * m_m32 * m_m23 + m_m43 * m_m32 * m_m24 - m_m42 * m_m24 * m_m33 + m_m42 * m_m23 * m_m34) *
            invDet,
        -(m_m43 * m_m32 * m_m14 - m_m43 * m_m34 * m_m12 + m_m42 * m_m13 * m_m34 - m_m42 * m_m14 * m_m33 - m_m44 * m_m32 * m_m13 + m_m44 * m_m33 * m_m12) *
            invDet,
        (-m_m22 * m_m44 * m_m13 + m_m22 * m_m43 * m_m14 - m_m43 * m_m12 * m_m24 + m_m42 * m_m24 * m_m13 + m_m44 * m_m12 * m_m23 - m_m42 * m_m23 * m_m14) *
            invDet,
        -(m_m13 * m_m32 * m_m24 + m_m12 * m_m23 * m_m34 + m_m22 * m_m14 * m_m33 - m_m12 * m_m24 * m_m33 - m_m22 * m_m13 * m_m34 - m_m14 * m_m32 * m_m23) *
            invDet,
        -(m_m21 * m_m44 * m_m33 - m_m21 * m_m43 * m_m34 + m_m41 * m_m23 * m_m34 - m_m44 * m_m31 * m_m23 - m_m41 * m_m24 * m_m33 + m_m24 * m_m43 * m_m31) *
            invDet,
        (m_m11 * m_m44 * m_m33 - m_m11 * m_m43 * m_m34 - m_m44 * m_m31 * m_m13 + m_m41 * m_m13 * m_m34 - m_m41 * m_m14 * m_m33 + m_m43 * m_m31 * m_m14) *
            invDet,
        (-m_m11 * m_m44 * m_m23 + m_m11 * m_m43 * m_m24 - m_m21 * m_m14 * m_m43 + m_m44 * m_m21 * m_m13 + m_m41 * m_m14 * m_m23 - m_m41 * m_m13 * m_m24) *
            invDet,
        (m_m11 * m_m23 * m_m34 - m_m11 * m_m24 * m_m33 + m_m21 * m_m14 * m_m33 - m_m23 * m_m31 * m_m14 - m_m21 * m_m13 * m_m34 + m_m24 * m_m31 * m_m13) *
            invDet,
        -(m_m31 * m_m22 * m_m44 - m_m42 * m_m24 * m_m31 - m_m41 * m_m34 * m_m22 - m_m44 * m_m32 * m_m21 + m_m41 * m_m32 * m_m24 + m_m42 * m_m21 * m_m34) *
            invDet,
        (-m_m42 * m_m31 * m_m14 + m_m31 * m_m44 * m_m12 + m_m14 * m_m41 * m_m32 - m_m12 * m_m41 * m_m34 + m_m11 * m_m42 * m_m34 - m_m32 * m_m11 * m_m44) *
            invDet,
        -(m_m22 * m_m41 * m_m14 - m_m11 * m_m22 * m_m44 - m_m42 * m_m21 * m_m14 + m_m44 * m_m21 * m_m12 - m_m41 * m_m12 * m_m24 + m_m11 * m_m42 * m_m24) *
            invDet,
        (-m_m34 * m_m11 * m_m22 + m_m34 * m_m21 * m_m12 + m_m31 * m_m14 * m_m22 + m_m32 * m_m11 * m_m24 - m_m32 * m_m21 * m_m14 - m_m31 * m_m12 * m_m24) *
            invDet,
        (-m_m22 * m_m41 * m_m33 + m_m22 * m_m43 * m_m31 + m_m41 * m_m32 * m_m23 - m_m43 * m_m32 * m_m21 + m_m42 * m_m21 * m_m33 - m_m42 * m_m23 * m_m31) *
            invDet,
        -(m_m11 * m_m42 * m_m33 - m_m11 * m_m43 * m_m32 - m_m42 * m_m31 * m_m13 + m_m41 * m_m13 * m_m32 - m_m41 * m_m12 * m_m33 + m_m43 * m_m31 * m_m12) *
            invDet,
        -(m_m43 * m_m11 * m_m22 - m_m43 * m_m21 * m_m12 - m_m41 * m_m13 * m_m22 - m_m42 * m_m11 * m_m23 + m_m42 * m_m21 * m_m13 + m_m41 * m_m12 * m_m23) *
            invDet,
        -(-m_m33 * m_m11 * m_m22 + m_m33 * m_m21 * m_m12 + m_m31 * m_m13 * m_m22 + m_m32 * m_m11 * m_m23 - m_m32 * m_m21 * m_m13 - m_m31 * m_m12 * m_m23) *
            invDet);
}

Vector3 Matrix44::getTranslationVector() const
{
    return Vector3(m_m14, m_m24, m_m34);
}
