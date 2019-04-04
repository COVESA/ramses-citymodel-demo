//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES Vector4f

#ifndef RAMSES_CITYMODEL_VECTOR4_H
#define RAMSES_CITYMODEL_VECTOR4_H

#include "stdint.h"

class Vector3;

/// Vector4 class.
class Vector4
{
public:
    Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    Vector4(const Vector3& v, float w = 1.0);
    float getX() const;
    float getY() const;
    float getZ() const;
    float getW() const;
    float get(uint32_t i) const;

private:
    float m_x;
    float m_y;
    float m_z;
    float m_w;
};

#endif
