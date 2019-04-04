//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_IINPUTRECEIVER_H
#define RAMSES_CITYMODEL_IINPUTRECEIVER_H

#include "ramses-renderer-api/Types.h"

class IInputReceiver
{
public:
    virtual ~IInputReceiver() {}
    virtual void mouseEvent(ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY)             = 0;
};

#endif
