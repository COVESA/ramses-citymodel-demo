//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES Vector2f

#ifndef RAMSES_CITYMODEL_VECTOR2_H
#define RAMSES_CITYMODEL_VECTOR2_H

#include "stdint.h"

/// Vector2 class.
class Vector2
{
public:
    Vector2(float x = 0.0f, float y = 0.0f);
    void    setX(float x);
    void    setY(float y);
    float   getX() const;
    float   getY() const;
    float   get(uint32_t i) const;
    Vector2 operator+(const Vector2& v) const;
    Vector2 operator-(const Vector2& v) const;
    Vector2 operator-() const;
    Vector2 normalize() const;
    float   length() const;
    bool    operator==(const Vector2& v) const;

private:
    float m_x;
    float m_y;
};

#endif
