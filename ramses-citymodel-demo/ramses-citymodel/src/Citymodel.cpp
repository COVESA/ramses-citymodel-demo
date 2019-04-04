//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/AnimationPath.h"
#include "ramses-citymodel/CitymodelScene.h"
#include "ramses-citymodel/CullingNode.h"
#include "ramses-citymodel/Math.h"
#include "ramses-citymodel/Name.h"
#include "ramses-citymodel/Name2D.h"
#include "ramses-citymodel/Tile.h"
#include "ramses-citymodel/Timer.h"
#include "ramses-citymodel/Vector3.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "sstream"
#include "iomanip"
#include "random"
#include "thread"
#include "assert.h"


Citymodel::Citymodel(CitymodelArguments arguments, ramses::RamsesFramework& framework)
    : m_arguments(arguments)
    , m_framework(framework)
    , m_pitch(1.0f, 0.1f, 4.0f)
    , m_yaw(1.0f, 0.1f, 3.0f)
    , m_distance(1.0f, 0.05f, 4.0f)
    , m_xPosition(1.0f, 0.1f, 4.0f)
    , m_yPosition(1.0f, 0.1f, 4.0f)
    , m_zPosition(1.0f, 0.1f, 4.0f)
    , m_destinationPosition(7670.512695f, 5716.101562f, 8.422584f)
{
    m_xPosition.reset(m_destinationPosition.getX());
    m_yPosition.reset(m_destinationPosition.getY());
    m_zPosition.reset(m_destinationPosition.getZ());
    m_pitch.reset(m_destinationPitch);
    m_distance.reset(m_destinationDistance);

    init();
}

Citymodel::~Citymodel()
{
    m_pager.terminate();
    m_guiOverlay.deinit();
    delete m_naming;
    delete m_ramsesClient;
    m_ramsesClient = nullptr;

    delete m_reader;
    m_reader = nullptr;
}

float Citymodel::getFovy() const
{
    return m_arguments.m_fovy;
}

ramses::sceneId_t Citymodel::getSceneId() const
{
    return m_arguments.m_sceneId;
}

void Citymodel::init()
{
    m_ramsesClient = new ramses::RamsesClient("citymodel", m_framework);

    // create a scene for distributing content
    m_ramsesScene = m_ramsesClient->createScene(m_arguments.m_sceneId, ramses::SceneConfig(), "citymodel scene");
    m_camera      = m_ramsesScene->createRemoteCamera();
    m_renderPass  = m_ramsesScene->createRenderPass();
    m_renderPass->setClearFlags(ramses::EClearFlags_None);
    m_renderPass->setCamera(*m_camera);

    m_renderGroup = m_ramsesScene->createRenderGroup();
    assert(m_renderGroup != NULL);
    m_renderPass->addRenderGroup(*m_renderGroup);

    m_reader = new Reader(*this);

    // when a specific frame is set, we don't do animation
    if (m_arguments.m_staticFrame >= 0)
    {
        m_showAnimation = false;
    }

    computeProjectionMatrix();

    createEffects();
    createMarkerGeometry();

    m_rootCameraTranslate = m_ramsesScene->createNode();

    m_cameraRotate    = m_ramsesScene->createNode();
    m_cameraTranslate = m_ramsesScene->createNode();

    m_cameraTranslate->addChild(*m_cameraRotate);
    m_cameraRotate->addChild(*m_rootCameraTranslate);
    m_rootCameraTranslate->addChild(*m_camera);

    readScene();
    if (!m_arguments.m_disableRoute)
    {
        m_route = new LineContainer(Vector3(1.0f, 0.8f, 0.0f),
                                    Vector3(0.8f, 0.4f, 0.0f),
                                    1.0f,
                                    Vector3(0.0f, 0.0f, 1.0f),
                                    *m_ramsesScene,
                                    *m_ramsesClient,
                                    *m_renderGroup,
                                    m_arguments.m_resPath);

        m_routePoints = m_scene->getRoutePoints();
        createRoute();
    }

    m_naming = new NamingManager(*m_ramsesClient,
                                 *m_ramsesScene,
                                 m_camera,
                                 *m_renderPass,
                                 m_arguments.m_windowWidth,
                                 m_arguments.m_windowHeight,
                                 m_arguments.m_fovy,
                                 m_showAnimation,
                                 m_arguments.m_resPath);

    if (!m_arguments.m_disableNaming)
    {
        m_namingPoints = m_scene->getNamePoints();
        createNaming();
    }

    const float aspect = static_cast<float>(m_arguments.m_windowWidth) / static_cast<float>(m_arguments.m_windowHeight);
    m_frustum.init(m_arguments.m_fovy, aspect, 1500.0f);

    if (!m_showAnimation)
    {
        m_frame = m_arguments.m_staticFrame;
    }

    m_guiOverlay.init(m_arguments.m_resPath, m_ramsesClient, m_ramsesScene, Vector2(m_arguments.m_windowWidth, m_arguments.m_windowHeight));
    m_mouseAndTouchInputSpeed = 1080.0f / static_cast<float>(m_arguments.m_windowHeight);
}

bool Citymodel::shouldExit() const
{
    return m_exit;
}

void Citymodel::readScene()
{
    m_reader->open(m_arguments.m_filePath + "/ramses-citymodel.rex");

    TileResourceContainer globalResources;
    m_scene = static_cast<CitymodelScene*>(m_reader->read(0, globalResources, false));

    if (!m_scene)
    {
        printf("Could not read scene !!!\n");
        exit(1);
    }

    buildTree();

    ramses::Node* carsorModel = m_scene->getCarsor();

    const float f = 0.5f;

    m_carsor              = m_ramsesScene->createNode();
    m_carsorTranslation   = m_ramsesScene->createNode();
    m_carsorRotation      = m_ramsesScene->createNode();
    ramses::Node* scaler = m_ramsesScene->createNode();
    scaler->setScaling(f, f, f);

    m_carsor->addChild(*m_carsorTranslation);
    m_carsorTranslation->addChild(*m_carsorRotation);
    m_carsorRotation->addChild(*scaler);

    if (nullptr != carsorModel)
    {
        scaler->addChild(*carsorModel);
    }
}

void Citymodel::buildTree()
{
    m_cullingTree = new CullingNode(m_scene->getTiles(), this);
}

void Citymodel::doAnimation(float dt)
{
    AnimationPath::Key* key = m_scene->getAnimationPath().getKey(m_frame);

    if (nullptr != key)
    {
        if (m_carsor)
        {
            m_carsorTranslation->setTranslation(key->getCarPosition().getX(), key->getCarPosition().getY(), key->getCarPosition().getZ() + 1.0f);
            m_carsorRotation->setRotation(key->getCarRotation().getX(), key->getCarRotation().getY(), key->getCarRotation().getZ());
        }
    }

    float pitch   = m_pitch.compute(dt, m_destinationPitch);
    float xRotate = -90.0f + pitch;

    float xPosition;
    float yPosition;
    float zPosition;
    if (key && (m_interactionMode == eFollowCarsor))
    {
        xPosition = key->getCarPosition().getX();
        yPosition = key->getCarPosition().getY();
        zPosition = key->getCarPosition().getZ();

        m_destinationYaw = key->getCarRotation().getZ();

        float currentAngle = m_yaw.get();
        float delta        = m_destinationYaw - currentAngle;
        while (fabs(delta) > 180.0)
        {
            if (delta >= 0.0)
            {
                delta -= 360.0;
            }
            else
            {
                delta += 360.0;
            }
        }
        m_destinationYaw = currentAngle + delta;

        m_destinationPosition = Vector3(xPosition, yPosition, zPosition);
        m_xPosition.reset(xPosition);
        m_yPosition.reset(yPosition);
        m_zPosition.reset(zPosition);

        setCarPosInMaterials(m_destinationPosition);
    }
    else
    {
        if (m_lastInputTime.getTime() > 10.0)
        {
            m_destinationPosition = key->getCarPosition();
            m_destinationPitch      = 90.0;
            m_destinationDistance   = 400.0;
        }

        xPosition = m_xPosition.compute(dt, m_destinationPosition.getX());
        yPosition = m_yPosition.compute(dt, m_destinationPosition.getY());
        zPosition = m_zPosition.compute(dt, m_destinationPosition.getZ());
        setCarPosInMaterials(Vector3(xPosition, yPosition, zPosition));
    }

    const float zRotate = m_yaw.compute(dt, m_destinationYaw);

    m_cameraRotate->setRotation(xRotate, 0.0, zRotate);
    m_cameraTranslate->setTranslation(xPosition, yPosition, zPosition);

    const float distance = m_distance.compute(dt, m_destinationDistance);
    m_rootCameraTranslate->setTranslation(0.0f, 0.0f, distance);

    Matrix44 viewMatrix    = Name2D::GetObjectSpaceMatrixOfNode(*m_camera);
    Matrix44 invViewMatrix = viewMatrix.inverse();

    Vector3 camPos = invViewMatrix.getTranslationVector();

    const float lightConeFactor = 35.0f / distance;
    m_scene->setLightConeFactor(lightConeFactor);

    if (m_naming)
    {
        const Matrix44 viewProjectionMatrix = mProjectionMatrix * viewMatrix;
        m_naming->update(viewProjectionMatrix);
    }
}

void Citymodel::doFrame(float dt)
{
    m_reader->getSceneLock().lock();

    doAnimation(m_showAnimation ? dt : 0.0f);
    doCulling();
    doPaging();

    m_ramsesScene->flush();

    if (!m_ramsesScene->isPublished() && (m_pager.getNumTilesToLoad() == 0))
    {
        m_ramsesScene->publish();
    }

    m_reader->getSceneLock().unlock();

    if (m_showAnimation)
    {
        m_frame += 1;
        const uint32_t frames = m_scene->getAnimationPath().getNumberOfKeys();
        if (m_frame >= frames)
        {
            m_numRoundsDriven++;
            m_frame = 0;

            if (m_arguments.m_roundsToDrive != 0 && m_numRoundsDriven == m_arguments.m_roundsToDrive)
            {
                m_exit = true;
            }
        }
    }

    if (m_arguments.m_showPerformanceValues)
    {
        const uint32_t measurePeriod(60);

        if (++m_measureFrames == measurePeriod)
        {
            float curTime = m_fpsMeasureTimer.getTime();
            float fps     = float(measurePeriod) / curTime;
            float cpuTime = m_fpsMeasureTimer.getCpuTime();
            float cpuLoad = cpuTime / curTime * 100.0f;

            m_measureFrames = 0;

            updatePerformanceValuesString(fps, cpuLoad);
            m_fpsMeasureTimer.reset();
        }
    }
}

void Citymodel::doPaging()
{
    m_openTilesToLoad += static_cast<int32_t>(m_tilesAddToRead.size());
    m_pager.add(m_tilesAddToRead);
    m_tilesAddToRead.clear();

    m_pager.remove(m_tilesRemoveToRead);
    m_tilesRemoveToRead.clear();

    std::vector<Tile*> loadedTiles;
    m_pager.get(loadedTiles);
    for (uint32_t i = 0; i < loadedTiles.size(); i++)
    {
        loadedTiles[i]->loaded();
    }
    m_openTilesToLoad -= static_cast<int32_t>(loadedTiles.size());
    assert(m_openTilesToLoad >= 0);

    std::set<Tile*>::iterator rit = m_tilesToDelete.begin();
    while (rit != m_tilesToDelete.end())
    {
        Tile* tile = *rit;
        rit++;
        tile->decDeleteCounter();
    }
}

void Citymodel::doCulling()
{
    Matrix44 invViewMatrix = Name2D::GetWorldSpaceMatrixOfNode(*m_camera);
    Matrix44 newmatrix     = invViewMatrix;
    m_frustum.transform(newmatrix);
    if (m_cullingTree)
    {
        m_cullingTree->computeVisible(0x01f, m_frustum);
    }
}

Reader& Citymodel::getReader()
{
    return *m_reader;
}

TilePager& Citymodel::getTilePager()
{
    return m_pager;
}

void Citymodel::createRoute()
{
    if (m_namingPoints.size() >= 2)
    {
        for (uint32_t i = 0; i < m_namingPoints.size() - 1; i += 2)
        {
            std::vector<Vector3> namingPoints;
            namingPoints.push_back(m_namingPoints[i]);
            namingPoints.push_back(m_namingPoints[i + 1]);
            m_route->addPolyline(namingPoints, LineContainer::ECapType_Round, LineContainer::ECapType_Round);
        }
    }

    m_route->addPolyline(m_routePoints, LineContainer::ECapType_Round, LineContainer::ECapType_Round);
    m_route->updateGeometry();
}

void Citymodel::createEffects()
{
    ramses::EffectDescription effectDesc;

    effectDesc.setUniformSemantic("u_mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setUniformSemantic("u_mvMatrix", ramses::EEffectUniformSemantic_ModelViewMatrix);

    m_reader->addEffect(createEffect(effectDesc, "ramses-citymodel-tile.vert", "ramses-citymodel-tile.frag"));
    m_reader->addEffect(
        createEffect(effectDesc, "ramses-citymodel-untextured.vert", "ramses-citymodel-untextured.frag"));

    m_markerEffect = createEffect(effectDesc, "ramses-citymodel-marker.vert", "ramses-citymodel-marker.frag");
}

ramses::Effect* Citymodel::createEffect(ramses::EffectDescription& effectDesc,
                                         const char*                vertexShaderFile,
                                         const char*                fragmentShaderFile)
{
    effectDesc.setVertexShaderFromFile((m_arguments.m_resPath + "/" + vertexShaderFile).c_str());
    effectDesc.setFragmentShaderFromFile((m_arguments.m_resPath + "/" + fragmentShaderFile).c_str());

    return m_ramsesClient->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "");
}

void Citymodel::createNaming()
{
    std::vector<std::string>& names = m_scene->getNames();

    std::vector<Vector3>& points = m_scene->getNamePoints();

    const uint32_t n = static_cast<const uint32_t>(points.size());

    const uint32_t nameCount = n / 2;

    std::minstd_rand randomNumberGenerator;

    for (uint32_t i = 0; i < nameCount; i++)
    {
        const uint32_t nameIndex = randomNumberGenerator() % (names.size() - 1);
        createName(points[i * 2], points[i * 2 + 1], names[nameIndex]);
    }
}

void Citymodel::createName(const Vector3& p0,
                            const Vector3& p1,
                            const std::string&              nameString)
{
    Name2D* name = new Name2D(nameString, *m_ramsesScene, *m_ramsesClient, p0, p1);
    m_naming->add(name);
}

void Citymodel::computeProjectionMatrix()
{
    const float aspect = static_cast<float>(m_arguments.m_windowWidth) / static_cast<float>(m_arguments.m_windowHeight);
    float       n      = 0.2f;
    float       f      = 1500.0f;

    const float tangent = tan(Math::Deg2Rad( m_arguments.m_fovy / 2.0f));
    const float t       = n * tangent;
    const float b       = -t;
    const float r       = t * aspect;
    const float l       = -r;

    mProjectionMatrix = Matrix44((2.0f * n) / (r - l),
                                                   0.0f,
                                                   (r + l) / (r - l),
                                                   0.0f,
                                                   0.0f,
                                                   (2.0f * n) / (t - b),
                                                   (t + b) / (t - b),
                                                   0.0f,
                                                   0.0f,
                                                   0.0f,
                                                   -(f + n) / (f - n),
                                                   (-2.0f * f * n) / (f - n),
                                                   0.0f,
                                                   0.0f,
                                                   -1.0f,
                                                   0.0f);
}

void Citymodel::updatePerformanceValuesString(float fps, float cpuLoad)
{
    if (m_naming)
    {
        if (m_statusName)
        {
            m_naming->removePermanent(m_statusName);
            delete m_statusName;
            m_statusName = 0;
        }

        std::ostringstream stringStream;

        stringStream << "FPS: " << std::fixed << std::setprecision(1) << fps << " - CPU: " << std::setprecision(2)
                     << cpuLoad << "%";

        m_statusName = new Name(stringStream.str(), *m_ramsesScene, *m_ramsesClient);

        m_naming->addPermanent(m_statusName);

        const Vector2& minBounding = m_statusName->minBounding();
        const Vector2& maxBounding = m_statusName->maxBounding();

        ramses::Node* node = m_statusName->topNode();
        assert(node);
        node->setTranslation(
            (maxBounding.getX() - minBounding.getX()) * 0.5 + 8, m_arguments.m_windowHeight - (maxBounding.getY() - minBounding.getY()) * 0.5 - 8, 0.0);
    }
}

void Citymodel::addTileToRead(Tile* tile)
{
    m_tilesAddToRead.push_back(tile);
}

void Citymodel::removeTileToRead(Tile* tile)
{
    m_tilesRemoveToRead.push_back(tile);
}

void Citymodel::addTileToDelete(Tile* tile)
{
    m_tilesToDelete.insert(tile);
}

void Citymodel::removeTileToDelete(Tile* tile)
{
    m_tilesToDelete.erase(tile);
}

void Citymodel::setCarPosInMaterials(const Vector3& carPos)
{
    m_scene->setCarPos(carPos);
}

ramses::Scene& Citymodel::getRamsesScene()
{
    return *m_ramsesScene;
}

ramses::RamsesClient& Citymodel::getRamsesClient()
{
    return *m_ramsesClient;
}

ramses::RenderGroup& Citymodel::getRenderGroup()
{
    return *m_renderGroup;
}

void Citymodel::mouseEvent(ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY)
{
    Vector2 mousePos(mousePosX, mousePosY);

    const float doubleClickTime    = 0.25;
    const float destinationSpeed   = 20.0f;
    const float maxDistance        = 1000.0f;
    const float minDistance        = 50.0f;
    const float speedAtMaxDistance = 2.0f;

    if (m_leftButtonPressed)
    {
        Vector2 mousePosDelta = mousePos - m_mousePos;
        m_destinationPitch += static_cast<float>(mousePosDelta.getY()) * 0.8f * m_mouseAndTouchInputSpeed;

        const float minPitch = 10.0f;
        if (m_destinationPitch < minPitch)
        {
            m_destinationPitch = minPitch;
        }

        const float maxPitch = 90.0f;
        if (m_destinationPitch > maxPitch)
        {
            m_destinationPitch = maxPitch;
        }

		m_destinationYaw += static_cast<float>(mousePosDelta.getX()) * 0.4f * m_mouseAndTouchInputSpeed;
    }

    if (m_middleButtonPressed)
    {
        Vector2 mousePosDelta = mousePos - m_mousePos;
        const float               v             = m_distance.get() / maxDistance * speedAtMaxDistance * m_mouseAndTouchInputSpeed;
        float                     dx            = -mousePosDelta.getX() * v;
        float                     dy            = mousePosDelta.getY() * v;

        Matrix44 rotation = Matrix44::RotationEulerXYZ(Vector3(0.0f, 0.0f, m_yaw.get()));

        Vector3 movement = rotation * Vector3(dx, dy, 0.0f);

        m_destinationPosition += movement;
    }

    switch (eventType)
    {
    case ramses::EMouseEvent_LeftButtonDown:
    {
        m_leftButtonPressed   = true;
        m_mousePosLButtonDown = mousePos;

        if (m_lastMouseLButtonClickTime.getTime() < doubleClickTime)
        {
            computeFocusPoint(mousePos);
        }
        m_lastMouseLButtonClickTime.reset();

        break;
    }
    case ramses::EMouseEvent_LeftButtonUp:
    {
        if (m_mousePosLButtonDown == mousePos)
        {
            m_destinationPitch = m_pitch.get();
            m_destinationYaw   = m_yaw.get();
        }

        m_leftButtonPressed = false;
        break;
    }
    case ramses::EMouseEvent_MiddleButtonDown:
    {
        m_middleButtonPressed = true;
        m_mousePosMButtonDown = mousePos;
        break;
    }
    case ramses::EMouseEvent_MiddleButtonUp:
    {
        if (m_mousePosMButtonDown == mousePos)
        {
            m_destinationPosition.setX(m_xPosition.get());
            m_destinationPosition.setY(m_yPosition.get());
        }

        m_middleButtonPressed = false;
        break;
    }
    case ramses::EMouseEvent_WheelUp:
    {
        m_destinationDistance -= destinationSpeed;

        if (m_destinationDistance < minDistance)
        {
            m_destinationDistance = minDistance;
        }

        break;
    }
    case ramses::EMouseEvent_WheelDown:
    {
        m_destinationDistance += destinationSpeed;

        if (m_destinationDistance > maxDistance)
        {
            m_destinationDistance = maxDistance;
        }
        break;
    }
    case ramses::EMouseEvent_RightButtonDown:
    {
        switch (m_interactionMode)
        {
        case eEditRoute:
        {
            if (m_lastMouseRButtonClickTime.getTime() < doubleClickTime)
            {
                deleteLastRoutePoint();
            }
            else
            {
                addRoutePoint(mousePos);
            }
            break;
        }
        case eEditNamingPoints:
        {
            if (m_lastMouseRButtonClickTime.getTime() < doubleClickTime)
            {
                deleteLastNamingPoint();
            }
            else
            {
                addNamingPoint(mousePos);
            }
            break;
        }

        case eFollowCarsor:
        {
            if (m_interactionModeCanBeSwitched)
            {
                setInteractionMode(eFreeMove);
            }
            break;
        }

        case eFreeMove:
        {
            if (m_interactionModeCanBeSwitched)
            {
                setInteractionMode(eFollowCarsor);
            }
            break;
        }
        }

        m_lastMouseRButtonClickTime.reset();
        break;
    }
    default:
        break;
    }

    m_mousePos = mousePos;
    m_lastInputTime.reset();
}

void Citymodel::createMarkerGeometry()
{
    m_markerGeometry = m_ramsesScene->createGeometryBinding(*m_markerEffect);

    const float vertexPositionsArray[] = {-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
                                          1.0f,  -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
                                          -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  -1.0f};

    const uint16_t indexArray[] = {0, 1, 2, 0, 2, 3, 4, 5, 1, 4, 1, 0, 5, 6, 2, 5, 2, 1,
                                   6, 7, 3, 6, 3, 2, 7, 4, 0, 7, 0, 3, 5, 4, 7, 5, 7, 6};

    const ramses::Vector3fArray* vertexPositions = m_ramsesClient->createConstVector3fArray(8, vertexPositionsArray);
    const ramses::UInt16Array*   indices         = m_ramsesClient->createConstUInt16Array(36, indexArray);

    ramses::AttributeInput positionsInput;
    m_markerEffect->findAttributeInput("a_position", positionsInput);
    m_markerGeometry->setInputBuffer(positionsInput, *vertexPositions);
    m_markerGeometry->setIndices(*indices);
}


void Citymodel::createMarker(const Vector3& position, float size)
{
    ramses::Appearance* appearance = m_ramsesScene->createAppearance(*m_markerEffect);
    ramses::MeshNode*   meshNode   = m_ramsesScene->createMeshNode();
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*m_markerGeometry);
    m_renderGroup->addMeshNode(*meshNode, 4);
    meshNode->setTranslation(position.getX(), position.getY(), position.getZ());
    meshNode->setScaling(size, size, size);
}

bool Citymodel::computeIntersection(const Vector3& p,
                                     const Vector3& d,
                                     Vector3&       position)
{
    m_reader->getSceneLock().lock();

    float                      r     = std::numeric_limits<float>::max();
    const std::vector<Tile*>& tiles = m_scene->getTiles();
    for (auto tile : tiles)
    {
        tile->computeIntersection(p, d, r);
    }
    m_reader->getSceneLock().unlock();

    if (r != std::numeric_limits<float>::max())
    {
        position = p + d * r;
        return true;
    }
    else
    {
        return false;
    }
}

void Citymodel::addRoutePoint(const Vector2& mousePos)
{
    Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        position.setZ(position.getZ() + 2.0);

        m_routePoints.push_back(position);
        createRoute();
    }
}

void Citymodel::addNamingPoint(const Vector2& mousePos)
{
    Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        position.setZ(position.getZ() + 2.0);

        m_namingPoints.push_back(position);
        createRoute();
    }
}

void Citymodel::deleteLastRoutePoint()
{
    if (!m_routePoints.empty())
    {
        m_routePoints.pop_back();
        if (!m_routePoints.empty())
        {
            m_routePoints.pop_back();
        }
        createRoute();
    }
}

void Citymodel::deleteLastNamingPoint()
{
    if (!m_namingPoints.empty())
    {
        m_namingPoints.pop_back();
        if (!m_namingPoints.empty())
        {
            m_namingPoints.pop_back();
        }
        createRoute();
    }
}

void Citymodel::computeFocusPoint(const Vector2& mousePos)
{
    Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        m_destinationPosition = position;
    }
}

bool Citymodel::computeIntersection(const Vector2& mousePos, Vector3& position)
{
    Matrix44 viewMatrix = Name2D::GetWorldSpaceMatrixOfNode(*m_camera);
    const float                distance   = 40.0f;
    const float tangens = tan(Math::Deg2Rad(m_arguments.m_fovy / 2.0f));

    float xNormalizedScreen = 2.0f * (mousePos.getX() / m_arguments.m_windowWidth - 0.5f);
    float yNormalizedScreen = 2.0f * (0.5f - mousePos.getY() / m_arguments.m_windowHeight);

    const float              z      = -distance;
    const float              y      = distance * tangens * yNormalizedScreen;
    const float              aspect = m_arguments.m_windowWidth / m_arguments.m_windowHeight;
    const float              x      = distance * tangens * aspect * xNormalizedScreen;
    Vector3 p(x, y, z);
    Vector3 pWorld = viewMatrix * p;

    Vector3 pEye = viewMatrix * Vector3(0.0f, 0.0f, 0.0f);

    return computeIntersection(pEye, pWorld - pEye, position);
}

void Citymodel::printRoute()
{
    printf("CECMClient::printRoute\n");
    for (auto point : m_routePoints)
    {
        printf("P(%ff, %ff, %ff)\n", point.getX(), point.getY(), point.getZ());
    }
}

void Citymodel::printNamingPoints()
{
    printf("CECMClient::printNamingPoints\n");
    for (auto point : m_namingPoints)
    {
        printf("P(%ff, %ff, %ff)\n", point.getX(), point.getY(), point.getZ());
    }
}

void Citymodel::setInteractionMode(EInteractionMode interactionMode)
{
    m_interactionMode = interactionMode;

    switch (m_interactionMode)
    {
    case eFollowCarsor:
    {
        m_destinationDistance = 225.0f;
        m_destinationPitch    = 25.0f;
        break;
    }
    default:
        break;
    }

    m_guiOverlay.setShowRotateIcon(m_interactionMode == eFreeMove, m_interactionModeCanBeSwitched);
}

void Citymodel::enableInteractionModeSwitchable(bool value)
{
    m_interactionModeCanBeSwitched = value;
}
