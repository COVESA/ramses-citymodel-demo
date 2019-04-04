//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELUTILS_H
#define RAMSES_CITYMODEL_CITYMODELUTILS_H

#include "ramses-citymodel/CitymodelArguments.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"

#include "memory"

/// Citymodel utility class.
class CitymodelUtils
{
public:
    static std::unique_ptr<ramses::RamsesFrameworkConfig> CreateFrameworkConfig(CitymodelArguments& arguments);
};

#endif
