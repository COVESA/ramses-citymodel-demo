//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES DisplayManager

#ifndef RAMSES_CITYMODELRENDERER_DISPLAYMANAGER_H
#define RAMSES_CITYMODELRENDERER_DISPLAYMANAGER_H

#include "DisplayManager.h"
#include "ramses-citymodel/IInputReceiver.h"

#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include "memory"
#include "mutex"
#include "map"
#include "set"

namespace ramses
{
    class RamsesFramework;
    class RamsesRenderer;
    class DisplayConfig;
}

class DisplayManager : public ramses::RendererEventHandlerEmpty
{
public:
    DisplayManager(ramses::RamsesRenderer& renderer, ramses::RamsesFramework& framework, bool autoShow, IInputReceiver* inputReceiver);
    virtual ~DisplayManager();

    void showSceneOnDisplay(ramses::sceneId_t   sceneId,
                            ramses::displayId_t displayId,
                            int32_t             sceneRenderOrder = 0u,
                            std::string         confirmationText = "");
    void unsubscribeScene(ramses::sceneId_t sceneId);
    void unmapScene(ramses::sceneId_t sceneId);
    void hideScene(ramses::sceneId_t sceneId);
    void linkData(ramses::sceneId_t        providerSceneId,
                  ramses::dataProviderId_t providerId,
                  ramses::sceneId_t        consumerSceneId,
                  ramses::dataConsumerId_t consumerId);
    void processConfirmationEchoCommand(const char* text);

    void dispatchAndFlush();

    ramses::displayId_t createDisplay(const ramses::DisplayConfig& config);
    void                destroyDisplay(ramses::displayId_t displayId);

    bool isRunning() const;
    bool isSceneShown(ramses::sceneId_t sceneId) const;
    bool isDisplayCreated(ramses::displayId_t display) const;

private:
    // IRendererEventHandler methods for scene state transition
    virtual void scenePublished(ramses::sceneId_t sceneId) override final;
    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override final;
    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
    virtual void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override final;
    virtual void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override final;

    virtual void mouseEvent(ramses::displayId_t displayId,
                            ramses::EMouseEvent eventType,
                            int32_t             mousePosX,
                            int32_t             mousePosY) override;
    virtual void windowClosed(ramses::displayId_t displayId) override final;

    struct MappingInfo
    {
        ramses::displayId_t display;
        int32_t             renderOrder;
        std::string         confirmationText;

        inline bool operator==(const MappingInfo& other)
        {
            return display == other.display && renderOrder == other.renderOrder;
        }
    };

    struct SceneStateEventCommandParams
    {
        MappingInfo mappingInfo;
    };

    enum ESceneState
    {
        ESceneState_Unpublished = 0,
        ESceneState_Published,
        ESceneState_Subscribed,
        ESceneState_Mapped,
        ESceneState_Rendered,

        ESceneState_GoingToPublished,
        ESceneState_GoingToSubscribed,
        ESceneState_GoingToMapped,
        ESceneState_GoingToRendered
    };

    ESceneState getCurrentSceneState(ramses::sceneId_t sceneId);
    ESceneState getTargetSceneState(ramses::sceneId_t sceneId);
    bool        isInTargetState(ramses::sceneId_t sceneId);
    void        goToTargetState(ramses::sceneId_t sceneId);
    static const std::string& GetSceneStateString(ESceneState sceneState);


    void handleShowCommand(ramses::sceneId_t sceneId, MappingInfo mappingInfo);
    void handleHideCommand(ramses::sceneId_t sceneId);
    void handleUnmapCommand(ramses::sceneId_t sceneId);
    void handleSubscribeCommand(ramses::sceneId_t sceneId);
    void handleUnsubscribeCommand(ramses::sceneId_t sceneId);

    ramses::RamsesRenderer& m_ramsesRenderer;

    const bool                               m_autoShow;
    std::map<ramses::sceneId_t, MappingInfo> m_scenesMappingInfo;
    std::map<ramses::sceneId_t, ESceneState> m_currentSceneStates;
    std::map<ramses::sceneId_t, ESceneState> m_targetSceneStates;

    std::set<ramses::displayId_t> m_createdDisplays;

    bool                                               m_isRunning;

    mutable std::recursive_mutex m_lock;

    IInputReceiver* m_inputReceiver = nullptr;
};

#endif
