//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/BoundingBox.h"

#include "limits"
#include "algorithm"

BoundingBox::BoundingBox()
    : m_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , m_max(std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest())
{
}

BoundingBox::BoundingBox(const Vector3& min, const Vector3& max)
    : m_min(min)
    , m_max(max)
{
}

void BoundingBox::set(const Vector3& min, const Vector3& max)
{
    m_min = min;
    m_max = max;
}

void BoundingBox::add(const Vector3& p)
{
    for (int i = 0; i < 3; i++)
    {
        m_min.set(i, std::min(m_min.get(i), p.get(i)));
        m_max.set(i, std::max(m_max.get(i), p.get(i)));
    }
}

void BoundingBox::add(const BoundingBox& box)
{
    for (int i = 0; i < 3; i++)
    {
        m_min.set(i, std::min(m_min.get(i), box.getMinimumBoxCorner().get(i)));
        m_max.set(i, std::max(m_max.get(i), box.getMaximumBoxCorner().get(i)));
    }
}

const Vector3& BoundingBox::getMinimumBoxCorner() const
{
    return m_min;
}

const Vector3& BoundingBox::getMaximumBoxCorner() const
{
    return m_max;
}

void BoundingBox::reset()
{
    m_min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_max = Vector3(std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest());
}

bool BoundingBox::checkOverlap(const BoundingBox& other) const
{
    for (uint32_t i = 0; i < 3; i++)
    {
        if ((m_min.get(i) > other.m_max.get(i)) || (m_max.get(i) < other.m_min.get(i)))
        {
            return false;
        }
    }
    return true;
}

Vector3 BoundingBox::getPoint(uint32_t index) const
{
    return Vector3(
        (index & 1) ? m_max.getX() : m_min.getX(), (index & 2) ? m_max.getY() : m_min.getY(), (index & 4) ? m_max.getZ() : m_min.getZ());
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const
{
    Vector3 min;
    Vector3 max;
    for (int i = 0; i < 3; i++)
    {
        min.set(i, std::max(m_min.get(i), other.getMinimumBoxCorner().get(i)));
        max.set(i, std::min(m_max.get(i), other.getMaximumBoxCorner().get(i)));
    }

    return BoundingBox(min, max);
}

bool BoundingBox::isEmpty() const
{
    return (m_min.getX() > m_max.getX()) || (m_min.getY() > m_max.getY()) || (m_min.getZ() > m_max.getZ());
}
