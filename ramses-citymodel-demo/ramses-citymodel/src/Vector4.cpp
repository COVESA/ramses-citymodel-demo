//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Vector4.h"
#include "ramses-citymodel/Vector3.h"

Vector4::Vector4(float x, float y, float z, float w)
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_w(w)
{
}

Vector4::Vector4(const Vector3& v, float w)
    : m_x(v.getX())
    , m_y(v.getY())
    , m_z(v.getZ())
    , m_w(w)
{
}

float Vector4::getX() const
{
    return m_x;
}

float Vector4::getY() const
{
    return m_y;
}

float Vector4::getZ() const
{
    return m_z;
}

float Vector4::getW() const
{
    return m_w;
}

float Vector4::get(uint32_t i) const
{
    switch (i)
    {
    case 0:
        return m_x;
    case 1:
        return m_y;
    case 2:
        return m_z;
    case 3:
        return m_w;
    default:
        return 0.0f;
    }
}
