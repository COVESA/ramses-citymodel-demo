//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Math.h"
#include "math.h"

float Math::Deg2Rad(float degrees)
{
    constexpr float f = 4.0 * atan(1.0) / 180.0;
    return degrees * f;
}

float Math::Rad2Deg(float rad)
{
    constexpr float f = 180.0 / (4.0 * atan(1.0));
    return rad * f;
}

float Math::Pi()
{
    constexpr float pi = atan(1.0f)*4.0f;
    return pi;
}
