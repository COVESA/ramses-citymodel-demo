//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES Vector3f

#ifndef RAMSES_CITYMODEL_VECTOR3_H
#define RAMSES_CITYMODEL_VECTOR3_H

#include "stdint.h"

class Vector4;

/// Vector3 class.
class Vector3
{
public:
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    Vector3(const Vector4& v);
    void  set(uint32_t i, float v);
    float get(uint32_t i) const;
    void  setX(float x);
    void  setY(float y);
    void  setZ(float z);
    float getX() const;
    float getY() const;
    float getZ() const;

    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator-() const;
    void    operator+=(const Vector3& v);
    void    operator-=(const Vector3& v);
    Vector3 operator*(float f) const;
    Vector3 cross(const Vector3& v) const;
    float   dot(const Vector3& v) const;
    Vector3 normalize() const;
    float   length() const;

private:
    float m_x;
    float m_y;
    float m_z;
};

#endif
