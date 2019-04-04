//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Vector3.h"
#include "ramses-citymodel/Vector4.h"

#include "assert.h"
#include "math.h"

Vector3::Vector3(float x, float y, float z) : m_x(x), m_y(y), m_z(z)
{
}

Vector3::Vector3(const Vector4& v): m_x(v.getX()), m_y(v.getY()), m_z(v.getZ())
{
}

void Vector3::set(uint32_t i, float v)
{
    switch (i)
    {
    case 0:
        m_x = v;
        break;
    case 1:
        m_y = v;
        break;
    case 2:
        m_z = v;
        break;
    default:
        assert(false);
    }
}

void Vector3::setX(float x)
{
    m_x = x;
}

void Vector3::setY(float y)
{
    m_y = y;
}

void Vector3::setZ(float z)
{
    m_z = z;
}

float Vector3::getX() const
{
    return m_x;
}

float Vector3::getY() const
{
    return m_y;
}

float Vector3::getZ() const
{
    return m_z;
}

float Vector3::get(uint32_t i) const
{
    switch (i)
    {
      case 0:
        return m_x;
      case 1:
        return m_y;
      case 2:
        return m_z;
      default:
          assert(false);
          return 0.0f;
    }
}

Vector3 Vector3::operator+(const Vector3& v) const
{
    return Vector3(m_x + v.m_x, m_y + v.m_y, m_z + v.m_z);
}

Vector3 Vector3::operator-(const Vector3& v) const
{
    return Vector3(m_x - v.m_x, m_y - v.m_y, m_z - v.m_z);
}

Vector3 Vector3::operator-() const
{
    return Vector3(-m_x, -m_y, -m_z);
}

void Vector3::operator-=(const Vector3& v)
{
    m_x -= v.getX();
    m_y -= v.getY();
    m_z -= v.getZ();
}

void Vector3::operator+=(const Vector3& v)
{
    m_x += v.getX();
    m_y += v.getY();
    m_z += v.getZ();
}

Vector3 Vector3::operator*(float f) const
{
    return Vector3(m_x * f, m_y * f, m_z * f);
}

Vector3 Vector3::cross(const Vector3& v) const
{
    return Vector3(m_y * v.m_z - m_z * v.m_y, m_z * v.m_x - m_x * v.m_z, m_x * v.m_y - m_y * v.m_x);
}

float Vector3::dot(const Vector3& v) const
{
    return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z;
}

Vector3 Vector3::normalize() const
{
    const float len = length();
    return Vector3(m_x / len, m_y / len, m_z / len);
}

float Vector3::length() const
{
    return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
}
