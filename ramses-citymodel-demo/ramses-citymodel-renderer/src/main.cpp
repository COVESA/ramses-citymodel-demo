//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/CitymodelUtils.h"
#include "CitymodelRendererArguments.h"
#include "DisplayManager.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"

int main(int argc, char* argv[])
{
    CitymodelRendererArguments arguments;
    if (!arguments.parse(argc, argv))
    {
        return 0;
    }

    ramses::RamsesFramework framework(*CitymodelUtils::CreateFrameworkConfig(arguments));

    ramses::RendererConfig rendererConfig;
    rendererConfig.enableSystemCompositorControl();
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    renderer.setSkippingOfUnmodifiedBuffers(false);

    Citymodel client(arguments, framework);
    framework.connect();

    DisplayManager displayManager(renderer, framework, false, &client);

    ramses::DisplayConfig displayConfig;
    const float aspect = static_cast<float>(arguments.m_windowWidth) / static_cast<float>(arguments.m_windowHeight);
    displayConfig.setPerspectiveProjection(client.getFovy(), aspect, 0.1f, 1500.0f);
    displayConfig.setWindowRectangle(0, 0, arguments.m_windowWidth, arguments.m_windowHeight);
    displayConfig.setWaylandIviSurfaceID(arguments.m_waylandIviSurfaceID);
    displayConfig.setWaylandIviLayerID(arguments.m_waylandIviLayerID);
    displayConfig.setWindowIviVisible();

    const ramses::displayId_t displayId = displayManager.createDisplay(displayConfig);

    ramses::sceneId_t sceneId = client.getSceneId();

    constexpr uint32_t renderOrder = 0;
    displayManager.showSceneOnDisplay(sceneId, displayId, renderOrder);

    Timer frameTime;
    while (!client.shouldExit() && displayManager.isRunning())
    {
        const float dt = frameTime.getTime();
        frameTime.reset();
        client.doFrame(dt);
        renderer.doOneLoop();
        displayManager.dispatchAndFlush();
    }

    renderer.hideScene(sceneId);
    renderer.unmapScene(sceneId);
    renderer.destroyDisplay(displayId);
    renderer.flush();

    return 0;
}
