//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// Derrived from RAMSES DisplayManager

#include "DisplayManager.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "assert.h"

DisplayManager::DisplayManager(ramses::RamsesRenderer& renderer, ramses::RamsesFramework& framework, bool autoShow, IInputReceiver* inputReceiver)
    : m_ramsesRenderer(renderer)
    , m_autoShow(autoShow)
    , m_isRunning(true)
    , m_lock()
    , m_inputReceiver(inputReceiver)
{
}

DisplayManager::~DisplayManager() {}

void DisplayManager::showSceneOnDisplay(ramses::sceneId_t   sceneId,
                                        ramses::displayId_t displayId,
                                        int32_t             sceneRenderOrder,
                                        std::string         confirmationText)
{
    std::unique_lock<std::recursive_mutex>  lock(m_lock);
    const MappingInfo mappingInfo = {displayId, sceneRenderOrder, confirmationText};
    handleShowCommand(sceneId, mappingInfo);
}

void DisplayManager::unmapScene(ramses::sceneId_t sceneId)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    handleUnmapCommand(sceneId);
}

void DisplayManager::unsubscribeScene(ramses::sceneId_t sceneId)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    handleUnsubscribeCommand(sceneId);
}

void DisplayManager::hideScene(ramses::sceneId_t sceneId)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    handleHideCommand(sceneId);
}

ramses::displayId_t DisplayManager::createDisplay(const ramses::DisplayConfig& config)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    const ramses::displayId_t displayId = m_ramsesRenderer.createDisplay(config);
    m_ramsesRenderer.flush();

    return displayId;
}

void DisplayManager::destroyDisplay(ramses::displayId_t displayId)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    m_ramsesRenderer.destroyDisplay(displayId);
    m_ramsesRenderer.flush();
}

void DisplayManager::linkData(ramses::sceneId_t        providerSceneId,
                              ramses::dataProviderId_t providerId,
                              ramses::sceneId_t        consumerSceneId,
                              ramses::dataConsumerId_t consumerId)
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    m_ramsesRenderer.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
}

void DisplayManager::dispatchAndFlush()
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    m_ramsesRenderer.dispatchEvents(*this);
    m_ramsesRenderer.flush();
}

bool DisplayManager::isRunning() const
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    return m_isRunning;
}

bool DisplayManager::isSceneShown(ramses::sceneId_t sceneId) const
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    if (m_currentSceneStates.find(sceneId) != m_currentSceneStates.end())
    {
        return m_currentSceneStates.at(sceneId) == ESceneState_Rendered;
    }
    return false;
}

bool DisplayManager::isDisplayCreated(ramses::displayId_t display) const
{
    std::unique_lock<std::recursive_mutex> lock(m_lock);
    bool retval = m_createdDisplays.find(display) != m_createdDisplays.end();
    return retval;
}

DisplayManager::ESceneState DisplayManager::getCurrentSceneState(ramses::sceneId_t sceneId)
{
    if (m_currentSceneStates.find(sceneId) != m_currentSceneStates.end())
    {
        return m_currentSceneStates[sceneId];
    }
    return ESceneState_Unpublished;
}

DisplayManager::ESceneState DisplayManager::getTargetSceneState(ramses::sceneId_t sceneId)
{
    if (m_targetSceneStates.find(sceneId) != m_targetSceneStates.end())
    {
        return m_targetSceneStates[sceneId];
    }
    return ESceneState_Unpublished;
}

bool DisplayManager::isInTargetState(ramses::sceneId_t sceneId)
{
    return getCurrentSceneState(sceneId) == getTargetSceneState(sceneId);
}

void DisplayManager::goToTargetState(ramses::sceneId_t sceneId)
{
    if (isInTargetState(sceneId))
    {
        return;
    }

    ESceneState currentSceneState = getCurrentSceneState(sceneId);
    ESceneState targetSceneState  = getTargetSceneState(sceneId);

    switch (currentSceneState)
    {
    case ESceneState_Unpublished:
        // cannot do anything here. Event handler scenePublished will trigger this again
        break;

    case ESceneState_Published:
        switch (targetSceneState)
        {
        case ESceneState_Subscribed:
        case ESceneState_Mapped:
        case ESceneState_Rendered:
            // Subscribe to scene
            if (m_ramsesRenderer.subscribeScene(sceneId) == ramses::StatusOK)
            {
                m_currentSceneStates[sceneId] = ESceneState_GoingToSubscribed;
            }
            break;

        default:
            // no other target state when in published!
            assert(false);
        }
        break;

    case ESceneState_Subscribed:
        switch (targetSceneState)
        {
        case ESceneState_Mapped:
        case ESceneState_Rendered:
        {
            assert(m_scenesMappingInfo.find(sceneId) != m_scenesMappingInfo.end());
            const MappingInfo& mapInfo = m_scenesMappingInfo[sceneId];
            // only subscribe, if display is created
            if (isDisplayCreated(mapInfo.display))
            {
                if (m_ramsesRenderer.mapScene(mapInfo.display, sceneId, mapInfo.renderOrder) == ramses::StatusOK)
                {
                    m_currentSceneStates[sceneId] = ESceneState_GoingToMapped;
                }
            }
            else
            {
                printf("DisplayManager::goToTargetState display not created!\n");
            }
            break;
        }
        case ESceneState_Published:
            if (m_ramsesRenderer.unsubscribeScene(sceneId) == ramses::StatusOK)
            {
                m_currentSceneStates[sceneId] = ESceneState_GoingToPublished;
            }
            break;

        default:
            assert(false);
        }
        break;

    case ESceneState_Mapped:

        switch (targetSceneState)
        {
        case ESceneState_Rendered:
            if (m_ramsesRenderer.showScene(sceneId) == ramses::StatusOK)
            {
                m_currentSceneStates[sceneId] = ESceneState_GoingToRendered;
            }
            break;

        case ESceneState_Subscribed:
        case ESceneState_Published:
            if (m_ramsesRenderer.unmapScene(sceneId) == ramses::StatusOK)
            {
                m_currentSceneStates[sceneId] = ESceneState_GoingToSubscribed;
            }
            break;

        default:
            assert(false);
        }
        break;

    case ESceneState_Rendered:

        switch (targetSceneState)
        {
        case ESceneState_Mapped:
        case ESceneState_Subscribed:
        case ESceneState_Published:
            if (m_ramsesRenderer.hideScene(sceneId) == ramses::StatusOK)
            {
                m_currentSceneStates[sceneId] = ESceneState_GoingToMapped;
            }
            break;

        default:
            assert(false);
        }
        break;

    case ESceneState_GoingToPublished:
    case ESceneState_GoingToSubscribed:
    case ESceneState_GoingToMapped:
    case ESceneState_GoingToRendered:
        // Cannot do anything, waiting for renderer event handlers to continue
        break;

    default:
        assert(false);
    }
}

const std::string& DisplayManager::GetSceneStateString(ESceneState sceneState)
{
    static std::string sceneStateNames[] =
    {
            "ESceneState_Unpublished = 0",
            "ESceneState_Published",
            "ESceneState_Subscribed",
            "ESceneState_Mapped",
            "ESceneState_Rendered",
            "ESceneState_GoingToPublished",
            "ESceneState_GoingToSubscribed",
            "ESceneState_GoingToMapped",
            "ESceneState_GoingToRendered"
    };

    return sceneStateNames[sceneState];
}

void DisplayManager::handleShowCommand(ramses::sceneId_t sceneId, MappingInfo mappingInfo)
{
    ESceneState currentSceneState = getCurrentSceneState(sceneId);

    // check whether scene was already mapped
    switch (currentSceneState)
    {
    // check if scene is unpublished
    case ESceneState_Unpublished:
        m_currentSceneStates[sceneId] = ESceneState_GoingToPublished;
        break;

    // check whether scene was already mapped
    case ESceneState_Mapped:
    case ESceneState_Rendered:
    case ESceneState_GoingToMapped:
    case ESceneState_GoingToRendered:
        assert(m_scenesMappingInfo.find(sceneId) != m_scenesMappingInfo.end());
        if (!(m_scenesMappingInfo[sceneId] == mappingInfo))
        {
            printf("DisplayManager::handleShowCommand: cannot execute show command for scene with id: %ld because it was mapped with different parameters before!\n", sceneId);
            return;
        }
        // trigger confirmation immediatly when alreay shown (avoids race between command and showing)
        if (currentSceneState == ESceneState_Rendered)
        {
            if (mappingInfo.confirmationText != "")
            {
                processConfirmationEchoCommand(mappingInfo.confirmationText.c_str());
                mappingInfo.confirmationText = "";
            }
        }
        break;

    default:
        break;
    }

    m_scenesMappingInfo[sceneId] = mappingInfo;
    m_targetSceneStates[sceneId] = ESceneState_Rendered;

    goToTargetState(sceneId);
}

void DisplayManager::handleUnsubscribeCommand(ramses::sceneId_t sceneId)
{
    switch (getCurrentSceneState(sceneId))
    {
    case ESceneState_Rendered:
    case ESceneState_GoingToRendered:
    case ESceneState_Mapped:
    case ESceneState_GoingToMapped:
    case ESceneState_Subscribed:
    case ESceneState_GoingToSubscribed:
        m_scenesMappingInfo.erase(sceneId);
        m_targetSceneStates[sceneId] = ESceneState_Published;
        goToTargetState(sceneId);
        break;
    default:
        printf("DisplayManager::handleUnsubscribeCommand: cannot execute unsubscribe command for scene with id: %ld because it was not subscribed before!\n", sceneId);
    }
}

void DisplayManager::handleUnmapCommand(ramses::sceneId_t sceneId)
{
    switch (getCurrentSceneState(sceneId))
    {
    case ESceneState_Rendered:
    case ESceneState_GoingToRendered:
    case ESceneState_Mapped:
    case ESceneState_GoingToMapped:
        m_scenesMappingInfo.erase(sceneId);
        m_targetSceneStates[sceneId] = ESceneState_Subscribed;
        goToTargetState(sceneId);
        break;
    default:
        printf("DisplayManager::handleUnmapCommand: cannot execute unmap command for scene with id: %ld because it is already mapped with different parameters!", sceneId);
    }
}

void DisplayManager::handleHideCommand(ramses::sceneId_t sceneId)
{
    switch (getCurrentSceneState(sceneId))
    {
    case ESceneState_Rendered:
    case ESceneState_GoingToRendered:
        m_targetSceneStates[sceneId] = ESceneState_Mapped;
        goToTargetState(sceneId);
        break;
    default:
        printf("DisplayManager::handleHideCommand: cannot execute hide command for scene with id: %ld because it was not rendered/shown before!", sceneId);
    }
}

/* IRendererEventHandler handlers */
void DisplayManager::scenePublished(ramses::sceneId_t sceneId)
{
    assert(m_scenesMappingInfo.find(sceneId) != m_scenesMappingInfo.end() ||
           m_currentSceneStates[sceneId] == ESceneState_GoingToPublished);

    // update current scene state
    m_currentSceneStates[sceneId] = ESceneState_Published;

    // update current target scene state
    if (getTargetSceneState(sceneId) == ESceneState_Unpublished)
    {
        if (m_autoShow)
        {
            m_targetSceneStates[sceneId] = ESceneState_Rendered;
            MappingInfo mapInfo = {0u, 0, ""};
            m_scenesMappingInfo[sceneId] = mapInfo;
        }
        else
        {
            m_targetSceneStates[sceneId] = ESceneState_Published;
        }
    }

    goToTargetState(sceneId);
}

void DisplayManager::sceneUnpublished(ramses::sceneId_t sceneId)
{
    assert(getCurrentSceneState(sceneId) == ESceneState_Published ||
           getCurrentSceneState(sceneId) == ESceneState_GoingToSubscribed);

    m_currentSceneStates.erase(sceneId);

    // only remove mapping request if in auto-mode (scene gets mapped again anyway), otherwise keep as scene might be
    // available again after reconnect
    if (getTargetSceneState(sceneId) != ESceneState_Rendered || m_autoShow)
    {
        m_targetSceneStates.erase(sceneId);
        m_scenesMappingInfo.erase(sceneId);
    }
}

void DisplayManager::sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        // another request, other then display manager
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] = ESceneState_Subscribed;
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to subscribed
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] =  ESceneState_Published;
        }
        printf("DisplayManager::sceneSubscribed: Could not subscribe scene with id : %ld\n", sceneId);
        break;
    case ramses::ERendererEventResult_INDIRECT:
    default:
        assert(false);
    }
}

void DisplayManager::sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToPublished)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] =  ESceneState_Published;
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_INDIRECT:
        m_currentSceneStates[sceneId] =  ESceneState_Published;
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to published
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToPublished)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] = ESceneState_Subscribed;
            printf("DisplayManager::sceneUnsubscribed: Could not unsubscribe scene with id : %ld\n", sceneId);
        }
        break;
    default:
        assert(false);
    }
}

void DisplayManager::sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        // another request, other then display manager
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] =  ESceneState_Mapped;
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to mapped
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] =  ESceneState_Subscribed;
            printf("DisplayManager::sceneMapped: Could not map scene with id : %ld\n", sceneId);
        }
        break;
    case ramses::ERendererEventResult_INDIRECT:
    default:
        assert(false);
    }
}

void DisplayManager::sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        // another request, other then display manager
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] = ESceneState_Subscribed;
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_INDIRECT:
        m_currentSceneStates[sceneId] = ESceneState_Subscribed;
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to subscribed
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] =  ESceneState_Mapped;
            printf("DisplayManager::sceneUnmapped: Could not unmap scene with id : %ld\n", sceneId);
        }
        break;
    default:
        assert(false);
    }
}

void DisplayManager::sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        // another request, other then display manager
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToRendered)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] =  ESceneState_Rendered;
        if (m_scenesMappingInfo[sceneId].confirmationText != "")
        {
            processConfirmationEchoCommand(m_scenesMappingInfo[sceneId].confirmationText.c_str());
            m_scenesMappingInfo[sceneId].confirmationText = "";
        }
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to rendered
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToRendered)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] = ESceneState_Mapped;
            printf("DisplayManager::sceneShown: Could not map scene with id : %ld\n", sceneId);
        }
        break;
    case ramses::ERendererEventResult_INDIRECT:
    default:
        assert(false);
    }
}

void DisplayManager::sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    switch (result)
    {
    case ramses::ERendererEventResult_OK:
        if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
        {
            printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
            return;
        }
        m_currentSceneStates[sceneId] = ESceneState_Mapped;
        goToTargetState(sceneId);
        break;
    case ramses::ERendererEventResult_INDIRECT:
        m_currentSceneStates[sceneId] = ESceneState_Mapped;
        break;
    case ramses::ERendererEventResult_FAIL:
        if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
        {
            // if the scene was not unpublished while it was still going to mapped
            if (getCurrentSceneState(sceneId) == ESceneState_GoingToMapped)
            {
                printf("DisplayManager - ignoring unexpected state change event for scene %ld\n", sceneId);
                return;
            }
            m_currentSceneStates[sceneId] = ESceneState_Rendered;
            printf("DisplayManager::sceneHidden: Could not hide scene with id : %ld\n", sceneId);
        }
        break;
    default:
        assert(false);
    }
}

void DisplayManager::displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
{
    if (ramses::ERendererEventResult_OK == result)
    {
        m_createdDisplays.insert(displayId);
        for (auto mapIt: m_scenesMappingInfo)
        {
            if (displayId == mapIt.second.display)
            {
                goToTargetState(mapIt.first);
            }
        }
    }
}

void DisplayManager::displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
{
    if (ramses::ERendererEventResult_OK == result)
    {
        m_createdDisplays.erase(displayId);
    }
}

void DisplayManager::windowClosed(ramses::displayId_t displayId)
{
    m_isRunning = false;
}

void DisplayManager::processConfirmationEchoCommand(const char* text)
{
    printf("DisplayManager::processConfirmationEchoCommand confirmation: %s\n", text);
}

void DisplayManager::mouseEvent(ramses::displayId_t displayId,
                                ramses::EMouseEvent eventType,
                                int32_t             mousePosX,
                                int32_t             mousePosY)
{
    if (m_inputReceiver)
    {
        m_inputReceiver->mouseEvent(eventType, mousePosX, mousePosY);
    }
}
