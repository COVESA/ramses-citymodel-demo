//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/CitymodelUtils.h"

std::unique_ptr<ramses::RamsesFrameworkConfig> CitymodelUtils::CreateFrameworkConfig(CitymodelArguments& arguments)
{
    std::string logLevelString       = std::to_string(arguments.m_logLevel);
    const char* frameworkArguments[] = {"", "-l", logLevelString.c_str()};

    std::unique_ptr<ramses::RamsesFrameworkConfig> frameworkConfig(new ramses::RamsesFrameworkConfig(3, frameworkArguments));
    frameworkConfig->setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    frameworkConfig->setInterfaceSelectionIPForTCPCommunication(arguments.m_myIp.c_str());
    frameworkConfig->setDaemonIPForTCPCommunication(arguments.m_daemonIp.c_str());
    return frameworkConfig;
}
