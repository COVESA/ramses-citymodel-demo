//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELRENDERERARGUMENTS_H
#define RAMSES_CITYMODEL_CITYMODELRENDERERARGUMENTS_H

#include "ramses-citymodel/CitymodelArguments.h"

/// Citymodel renderer command line arguments
class CitymodelRendererArguments : public CitymodelArguments
{
public:
    virtual void createOptions(cxxopts::Options& options)
    {
        CitymodelArguments::createOptions(options);
        options.add_options("Wayland")(
            "sid", "Wayland surface id", cxxopts::value<uint32_t>(m_waylandIviSurfaceID)->default_value("100"))(
            "lid", "Wayland layer id", cxxopts::value<uint32_t>(m_waylandIviLayerID)->default_value("1"));
    }

    uint32_t m_waylandIviSurfaceID;
    uint32_t m_waylandIviLayerID;
};

#endif
