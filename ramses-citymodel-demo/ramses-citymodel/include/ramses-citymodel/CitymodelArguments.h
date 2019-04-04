//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELARGUMENTS_H
#define RAMSES_CITYMODEL_CITYMODELARGUMENTS_H

#include "cxxopts.hpp"
#include "getopt.h"
#include "stdint.h"
#include "cstdlib"
#include "string"

#include "ramses-citymodel/Vector2.h"

/// Citymodel command line arguments
class CitymodelArguments
{
public:
    bool parse(int argc, char* argv[])
    {
        try
        {
            cxxopts::Options options(argv[0], "");

            createOptions(options);

            auto result = options.parse(argc, argv);

            if (m_help)
            {
                printf("%s\n", options.help({"","Wayland"}).c_str());
                return false;
            }
        } catch (const cxxopts::OptionException& e)
        {
            printf("Error parsing options: %s. Use --help to show command line options.\n", e.what());
            return false;
        }
        return true;
    }

    virtual void createOptions(cxxopts::Options& options)
    {
        options.add_options()
            ("help", "Print help", cxxopts::value<bool>(m_help))
            ("sceneId", "RAMSES Scene Id", cxxopts::value<uint32_t>(m_sceneId)->default_value("100"))
            ("disableRoute", "Disable route rendering", cxxopts::value<bool>(m_disableRoute))
            ("disableNaming", "Disable naming", cxxopts::value<bool>(m_disableNaming))
            ("staticFrame", "Render only a given static frame instead animating", cxxopts::value<int32_t>(m_staticFrame))
            ("showPerformanceValues", "Show fps/cpu usage performance values", cxxopts::value<bool>(m_showPerformanceValues))
            ("rounds", "Limit number of rounds to drive", cxxopts::value<uint32_t>(m_roundsToDrive))
            ("filePath", "Path to the database file", cxxopts::value<std::string>(m_filePath)->default_value("./res"))
            ("resPath", "Path to the resource files", cxxopts::value<std::string>(m_resPath)->default_value("./res"))
            ("fovy", "Field of view in degrees", cxxopts::value<float>(m_fovy)->default_value("19.0"))
            ("w,width", "Window width", cxxopts::value<uint32_t>(m_windowWidth)->default_value("1280"))
            ("h,height", "Window height", cxxopts::value<uint32_t>(m_windowHeight)->default_value("480"))
            ("l,loglevel", "Loglevel", cxxopts::value<uint32_t>(m_logLevel)->default_value("3"))
            ("myip", "My ip address", cxxopts::value<std::string>(m_myIp)->default_value("127.0.0.1"))
            ("ip", "Daemon ip address", cxxopts::value<std::string>(m_daemonIp)->default_value("127.0.0.1"))
            ;
    }

    bool        m_help = false;
    uint32_t    m_sceneId;
    bool        m_disableRoute          = false;
    bool        m_disableNaming         = false;
    int32_t     m_staticFrame           = -1;
    bool        m_showPerformanceValues = false;
    uint32_t    m_roundsToDrive         = 0;
    std::string m_filePath;
    std::string m_resPath;
    float       m_fovy;
    uint32_t    m_windowWidth;
    uint32_t    m_windowHeight;
    uint32_t    m_logLevel;
    std::string m_myIp;
    std::string m_daemonIp;
};

#endif
