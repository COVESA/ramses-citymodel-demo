//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/CitymodelUtils.h"

int main(int argc, char* argv[])
{
    CitymodelArguments arguments;
    if (!arguments.parse(argc, argv))
    {
        return 0;
    }

    ramses::RamsesFramework framework(*CitymodelUtils::CreateFrameworkConfig(arguments));
    Citymodel               citymodel(arguments, framework);

    framework.connect();

    uint32_t frame = 0;
    Timer    timer;

    while (!citymodel.shouldExit())
    {
        constexpr float frameTime = 1.0f / 60.0f;
        citymodel.doFrame(frameTime);
        frame++;

        float sleepTime = (frameTime * frame - timer.getTime());
        if (sleepTime >= 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }
    }

    return 0;
}
