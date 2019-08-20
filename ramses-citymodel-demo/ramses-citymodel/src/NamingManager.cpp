//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/NamingManager.h"
#include "ramses-citymodel/Name2D.h"
#include "ramses-citymodel/Math.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderGroup.h"

#include "math.h"
#include <algorithm>

NamingManager::NamingManager(ramses::RamsesClient& client,
                             ramses::Scene&        scene,
                             ramses::Node*         parent,
                             ramses::RenderPass&   renderPass,
                             float                 width,
                             float                 height,
                             float                 fovy,
                             bool                  useFading,
                             const std::string& resourceFilePath)
    : m_halfSize(width * 0.5f, height * 0.5f)
    , m_client(client)
    , m_scene(scene)
    , m_useFading(useFading)
    , m_fontInstance(ramses::InvalidFontInstanceId)
    , m_textCache(scene, m_fontRegistry, 1024u, 1024u)
{
    m_renderGroup = m_scene.createRenderGroup();
    renderPass.addRenderGroup(*m_renderGroup, 1);

    m_camera = createOrthoCamera(width, height, fovy);
    parent->addChild(*m_camera);

    /// Create effect:
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile((resourceFilePath + "/ramses-citymodel-text-effect.vert").c_str());
    effectDesc.setFragmentShaderFromFile((resourceFilePath + "/ramses-citymodel-text-effect.frag").c_str());
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setUniformSemantic("u_texture", ramses::EEffectUniformSemantic_TextTexture);
    effectDesc.setAttributeSemantic("a_position", ramses::EEffectAttributeSemantic_TextPositions);
    effectDesc.setAttributeSemantic("a_texcoord", ramses::EEffectAttributeSemantic_TextTextureCoordinates);

    m_effect = m_client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "text effect");

    m_font = m_fontRegistry.createFreetype2Font((resourceFilePath + "/ramses-demoLib-SourceSansPro-Regular.ttf").c_str());
    const uint32_t fontSize = std::max(static_cast<uint32_t>(height / 1080.0f * 48.0f), 1u);
    m_fontInstance          = m_fontRegistry.createFreetype2FontInstance(m_font, fontSize);
}

NamingManager::~NamingManager()
{
    m_fontRegistry.deleteFontInstance(m_fontInstance);
    m_fontRegistry.deleteFont(m_font);
    m_client.destroy(*m_effect);
    m_scene.destroy(*m_renderGroup);
    m_scene.destroy(*m_camera);
}

ramses::Node* NamingManager::createOrthoCamera(float width, float height, float fovy)
{
    // Computes translation/scale such that the rectangle (0,0,0), (width, height,0)
    // maps orthogonal to the viewport at a certain distance.
    ramses::Node* camera  = m_scene.createNode();
    float         tangent = tan(Math::Deg2Rad(fovy / 2.0f));
    float         d       = height / (2.0f * tangent);

    // Near, far plane distance of renderer is 0.1 .. 1500 per default, choose a value for distance in between.
    constexpr float distance = 1000.0f;

    const float scaleFactor = distance / d;
    camera->setScaling(scaleFactor, scaleFactor, scaleFactor);
    camera->setTranslation(-width / 2.0f * scaleFactor, -height / 2.0f * scaleFactor, -distance);

    return camera;
}

void NamingManager::add(Name2D* name)
{
    m_names.push_back(name);
    name->useFading(m_useFading);

    name->createGeometry(m_textCache, m_fontInstance, *m_effect, *m_renderGroup);
    name->setNamingManager(this);

    ramses::Node* node = name->topNode();
    if (node)
    {
        m_camera->addChild(*node);
    }

    addToInvisibleNames(name);
}

void NamingManager::addPermanent(Name* name)
{
    name->createGeometry(m_textCache, m_fontInstance, *m_effect, *m_renderGroup);
    ramses::Node* node = name->topNode();
    if (node)
    {
        m_camera->addChild(*node);
    }
}

void NamingManager::removePermanent(Name* name)
{
    ramses::Node* node = name->topNode();
    if (node)
    {
        m_camera->removeChild(*node);
    }
}

void NamingManager::update(const Matrix44& viewProjectionMatrix)
{
    m_colliderNames.clear();

    const uint32_t maxInvisibleNamesToCheck = 5;

    m_viewProjectionMatrix = viewProjectionMatrix;

    std::vector<Name2D*> names;

    for (std::list<Name2D*>::iterator i = m_visibleNames.begin(); i != m_visibleNames.end(); ++i)
    {
        names.push_back(*i);
    }

    uint32_t n = m_invisibleNames.size();
    // We only restrict number of invisible names to check when we use fading (i.e. we are in
    // animation mode). When we use a static frame (!m_useFading) we want get all street names
    // visible for the first frame and don't alter the number of names to check.
    if (m_useFading && n > maxInvisibleNamesToCheck)
    {
        n = maxInvisibleNamesToCheck;
    }

    while (n > 0)
    {
        Name2D* name = m_invisibleNames.front();
        m_invisibleNames.pop_front();
        m_invisibleNames.push_back(name);
        names.push_back(name);
        n--;
    }

    for (Name2D* name: names)
    {
        name->update();
    }
}

bool NamingManager::projectCheckXY(const Vector3& p, Vector2& p2d, bool& xyVisible)
{
    Vector4 pt = m_viewProjectionMatrix * Vector4(p);

    float w = pt.getW();

    // Clip:
    if (pt.getZ() <= -w)
    {
        return false;
    }

    float oow = 1.0f / w;

    p2d = Vector2((pt.getX() * oow + 1.0f) * m_halfSize.getX(), (pt.getY() * oow + 1.0f) * m_halfSize.getY());

    // Clip with left, right, bottom, top plane:
    xyVisible = pt.getX() >= -w && pt.getX() <= w && pt.getY() >= -w && pt.getY() <= w;

    return true;
}

bool NamingManager::project(const Vector3& p, Vector2& p2d)
{
    Vector4 pt = m_viewProjectionMatrix * Vector4(p);

    float w = pt.getW();

    // Clip:
    if (w <= 0.0)
    {
        return false;
    }

    float oow = 1.0f / w;

    p2d = Vector2((pt.getX() * oow + 1.0f) * m_halfSize.getX(), (pt.getY() * oow + 1.0f) * m_halfSize.getY());
    return true;
}

void NamingManager::addToVisibleNames(Name2D* name)
{
    m_invisibleNames.remove(name);
    m_visibleNames.push_back(name);
    ramses::Node* node = name->topNode();
    if (node)
    {
        name->setVisibility(true);
    }
}

void NamingManager::addToInvisibleNames(Name2D* name)
{
    m_visibleNames.remove(name);
    m_invisibleNames.push_back(name);
    ramses::Node* node = name->topNode();
    if (node)
    {
        name->setVisibility(false);
    }
}

bool NamingManager::checkCollision(Name2D* name)
{
    for (Name2D* colliderName: m_colliderNames)
    {
        if (colliderName->checkCollision(name))
        {
            return true;
        }
    }
    return false;
}

void NamingManager::addToColliders(Name2D* name)
{
    m_colliderNames.push_back(name);
}
