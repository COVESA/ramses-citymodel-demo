//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/CitymodelGUIOverlay.h"
#include "ramses-utils.h"

void CitymodelGUIOverlay::init(const std::string& resourceFilePath,
                               ramses::RamsesClient*           ramsesClient,
                               ramses::Scene*                  ramsesScene,
                               const Vector2& windowSize,
                               bool showLogo)
{
    m_ramsesClient = ramsesClient;
    m_ramsesScene  = ramsesScene;
    m_windowSize   = windowSize;

    m_renderPass = m_ramsesScene->createRenderPass();

    m_camera = m_ramsesScene->createOrthographicCamera();
    m_camera->setFrustum(0.0f, m_windowSize.getX(), 0.0f, m_windowSize.getY(), 0.1f, 1.f);
    m_camera->setViewport(0, 0, m_windowSize.getX(), m_windowSize.getY());

    m_renderPass->setCamera(*m_camera);
    m_guiRenderGroup = m_ramsesScene->createRenderGroup();
    m_renderPass->addRenderGroup(*m_guiRenderGroup);

    createGUIEffects(resourceFilePath);
    createGUITextures(resourceFilePath);

    m_guiRoot = m_ramsesScene->createNode();
    m_guiRoot->setTranslation(0.0f, m_windowSize.getY(), 0.0f);
    m_guiRoot->setScaling(1.0f, -1.0f, 1.0f);

    if (showLogo)
    {
        createLogo();
    }
}

void CitymodelGUIOverlay::deinit()
{
    delete m_rotateIconBox;
    delete m_backIconBox;
    delete m_logoBox;

    if (m_renderPass != nullptr)
    {
        m_ramsesScene->destroy(*m_renderPass);
    }

    if (m_camera != nullptr)
    {
        m_ramsesScene->destroy(*m_camera);
    }

    if (m_guiRenderGroup != nullptr)
    {
        m_ramsesScene->destroy(*m_guiRenderGroup);
    }

    if (m_guiRoot != nullptr)
    {
        m_ramsesScene->destroy(*m_guiRoot);
    }

    destroyGUIEffects();
    destroyGUITextures();
}

void CitymodelGUIOverlay::createGUIEffects(const std::string& resourceFilePath)
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile((resourceFilePath + "/ramses-demoLib-rgba-textured.vert").c_str());
    effectDesc.setFragmentShaderFromFile((resourceFilePath + "/ramses-demoLib-rgba-textured.frag").c_str());
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    m_effectRGBATextured = m_ramsesClient->createEffect(effectDesc);
}

void CitymodelGUIOverlay::destroyGUIEffects()
{
    if (m_effectRGBATextured != nullptr)
    {
        m_ramsesClient->destroy(*m_effectRGBATextured);
    }
}

void CitymodelGUIOverlay::createGUITextures(const std::string& resourceFilePath)
{
    m_rotateIconTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng((resourceFilePath + "/ramses-citymodel-rotateIcon.png").c_str(), *m_ramsesClient);

    m_backIconTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng((resourceFilePath + "/ramses-citymodel-backIcon.png").c_str(), *m_ramsesClient);

    m_logoTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng((resourceFilePath + "/ramses-citymodel-mentorLogo.png").c_str(), *m_ramsesClient);
}

void CitymodelGUIOverlay::destroyGUITextures()
{
    if (m_rotateIconTexture != nullptr)
    {
        m_ramsesClient->destroy(*m_rotateIconTexture);
    }

    if (m_backIconTexture != nullptr)
    {
        m_ramsesClient->destroy(*m_backIconTexture);
    }

    if (m_logoTexture != nullptr)
    {
        m_ramsesClient->destroy(*m_logoTexture);
    }

}

void CitymodelGUIOverlay::setShowRotateIcon(bool show, bool interactionModeCanBeSwitched)
{
    if (show)
    {
        if (m_rotateIconBox == nullptr)
        {
            m_rotateIconBox = new ImageBox(*m_rotateIconTexture,
                                           m_rotateIconTexture->getWidth(),
                                           m_rotateIconTexture->getHeight(),
                                           ImageBox::EBlendMode_Normal,
                                           *m_ramsesClient,
                                           *m_ramsesScene,
                                           m_guiRenderGroup,
                                           0,
                                           *m_effectRGBATextured,
                                           false,
                                           m_guiRoot);

            m_rotateIconBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
            m_rotateIconPosition = Vector2(m_windowSize.getX() / 2, m_windowSize.getY() / 2);
            m_rotateIconBox->setPosition(m_rotateIconPosition.getX() - m_rotateIconTexture->getWidth() / 2,
                                         m_rotateIconPosition.getY() - m_rotateIconTexture->getHeight() / 2);
        }

        if (m_backIconBox == nullptr && interactionModeCanBeSwitched)
        {
            m_backIconBox = new ImageBox(*m_backIconTexture,
                                         m_backIconTexture->getWidth(),
                                         m_backIconTexture->getHeight(),
                                         ImageBox::EBlendMode_Normal,
                                         *m_ramsesClient,
                                         *m_ramsesScene,
                                         m_guiRenderGroup,
                                         0,
                                         *m_effectRGBATextured,
                                         false,
                                         m_guiRoot);

            m_backIconBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
            m_backIconPosition = Vector2(m_windowSize.getX() - m_backIconTexture->getWidth() - 8,
                                         m_windowSize.getY() - m_backIconTexture->getHeight() - 8);
            m_backIconBox->setPosition(m_backIconPosition.getX(), m_backIconPosition.getY());
        }
    }
    else
    {
        if (m_rotateIconBox != nullptr)
        {
            delete m_rotateIconBox;
            m_rotateIconBox = nullptr;
        }

        if (m_backIconBox != nullptr)
        {
            delete m_backIconBox;
            m_backIconBox = nullptr;
        }
    }
}

bool CitymodelGUIOverlay::checkRotateIconPressed(const Vector2& position) const
{
    return (position - m_rotateIconPosition).length() < 100.0f;
}

bool CitymodelGUIOverlay::checkBackButtonPressed(const Vector2& position) const
{
    return position.getX() >= m_backIconPosition.getX() && position.getY() >= m_backIconPosition.getY() &&
           position.getX() <= static_cast<int32_t>(m_backIconPosition.getX() + m_backIconTexture->getWidth()) &&
           position.getY() <= static_cast<int32_t>(m_backIconPosition.getY() + m_backIconTexture->getHeight());
}

void CitymodelGUIOverlay::createLogo()
{
    m_logoBox = new ImageBox(*m_logoTexture,
                             m_logoTexture->getWidth(),
                             m_logoTexture->getHeight(),
                             ImageBox::EBlendMode_Normal,
                             *m_ramsesClient,
                             *m_ramsesScene,
                             m_guiRenderGroup,
                             1,
                             *m_effectRGBATextured,
                             false,
                             m_guiRoot);
    m_logoBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_logoBox->setPosition(0.0f, m_windowSize.getY() - m_logoTexture->getHeight() - 16.0f);
}
