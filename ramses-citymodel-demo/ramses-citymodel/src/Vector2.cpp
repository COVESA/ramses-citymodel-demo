//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Vector2.h"

#include "math.h"

Vector2::Vector2(float x, float y)
    : m_x(x)
    , m_y(y)
{
}

void Vector2::setX(float x)
{
    m_x = x;
}
void Vector2::setY(float y)
{
    m_y = y;
}

float Vector2::getX() const
{
    return m_x;
}

float Vector2::getY() const
{
    return m_y;
}

float Vector2::get(uint32_t i) const
{
    switch (i)
    {
    case 0:
        return m_x;
    case 1:
        return m_y;
    default:
        return 0.0f;
    }
}

Vector2 Vector2::operator+(const Vector2& v) const
{
    return Vector2(m_x + v.m_x, m_y + v.m_y);
}

Vector2 Vector2::operator-(const Vector2& v) const
{
    return Vector2(m_x - v.m_x, m_y - v.m_y);
}

Vector2 Vector2::operator-() const
{
    return Vector2(-m_x, -m_y);
}

Vector2 Vector2::normalize() const
{
    const float len = length();
    return Vector2(m_x / len, m_y / len);
}

float Vector2::length() const
{
    return sqrt(m_x * m_x + m_y * m_y);
}

bool Vector2::operator==(const Vector2& v) const
{
    return m_x == v.m_x && m_y == v.m_y;
}
