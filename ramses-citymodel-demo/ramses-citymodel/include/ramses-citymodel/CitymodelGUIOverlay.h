//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELGUIOVERLAY_H
#define RAMSES_CITYMODEL_CITYMODELGUIOVERLAY_H

#include "ramses-citymodel/ImageBox.h"
#include "ramses-citymodel/Vector2.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/OrthographicCamera.h"

/// Citymodel GUI overlay
class CitymodelGUIOverlay
{
public:

    /// Creates the GUI overlay elements.
    /** @param resourceFilePath Path of resource files.
     *  @param ramsesScene The RAMSES scene.
     *  @param windowSize Size of the window.
     *  @param showLogo Flag, whether the logo shall be shown, or not. */
    void init(const std::string& resourceFilePath, ramses::RamsesClient* ramsesClient, ramses::Scene* ramsesScene, const Vector2& windowSize, bool showLogo = true);

    /// Delete created objects.
    void deinit();

    /// Sets, whether the rotate icon shall be shown or not.
    /** @param show Flag, if the icon shall be shown. */
    void setShowRotateIcon(bool show, bool interactionModeCanBeSwitched);

    /// Checks if a given position lies inside the rotate icon.
    /** @param position The position to check.
     *  @return "true", when inside the rotate icon. */
    bool checkRotateIconPressed(const Vector2& position) const;

    /// Checks if a given position lies inside the back button.
    /** @param position The position to check.
     *  @return "true", when inside the button. */
    bool checkBackButtonPressed(const Vector2& position) const;

private:
    /// Creates the GUI effect.
    void createGUIEffects(const std::string& resourceFilePath);

    /// Destroys the created effects.
    void destroyGUIEffects();

    /// Creates the GUI textures.
    void createGUITextures(const std::string& resourceFilePath);

    /// Destroys the created textures.
    void destroyGUITextures();

    /// Creates the logo.
    void createLogo();

    /// The RAMSES client.
    ramses::RamsesClient* m_ramsesClient = nullptr;

    /// The render pass for the GUI overlay.
    ramses::RenderPass* m_renderPass = nullptr;

    /// The camera for the GUI overlay.
    ramses::OrthographicCamera* m_camera = nullptr;

    /// The RAMSES scene.
    ramses::Scene* m_ramsesScene = nullptr;

    /// Size of the window in pixel.
    Vector2 m_windowSize;

    /// The ramses framework passed into.
    ramses::RamsesFramework* m_framework = nullptr;

    /// Root node for the GUI overlay.
    ramses::Node* m_guiRoot = nullptr;

    /// Render group for the GUI overlay.
    ramses::RenderGroup* m_guiRenderGroup = nullptr;

    /// RGBA texture effect.
    ramses::Effect* m_effectRGBATextured = nullptr;

    /// Texture for the rotate icon.
    ramses::Texture2D* m_rotateIconTexture = nullptr;

    /// Texture for the back icon.
    ramses::Texture2D* m_backIconTexture = nullptr;

    /// Texture for the logo.
    ramses::Texture2D* m_logoTexture = nullptr;

    /// Position of the rotate icon.
    Vector2 m_rotateIconPosition;

    /// Position of the back icon.
    Vector2 m_backIconPosition;

    /// Image box for the rotate icon.
    ImageBox* m_rotateIconBox = nullptr;

    /// Image box for the back icon.
    ImageBox* m_backIconBox = nullptr;

    /// Image box for the logo.
    ImageBox* m_logoBox = nullptr;
};

#endif
