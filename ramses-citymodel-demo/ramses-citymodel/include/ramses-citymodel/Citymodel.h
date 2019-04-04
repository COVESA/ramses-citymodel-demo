//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODEL_H
#define RAMSES_CITYMODEL_CITYMODEL_H

#include "ramses-client-api/EffectDescription.h"

#include "set"
#include "string"
#include "map"
#include "vector"
#include "ramses-citymodel/CitymodelArguments.h"
#include "ramses-citymodel/CitymodelGUIOverlay.h"
#include "ramses-citymodel/CullingNode.h"
#include "ramses-citymodel/Frustum.h"
#include "ramses-citymodel/IInputReceiver.h"
#include "ramses-citymodel/LineContainer.h"
#include "ramses-citymodel/NamingManager.h"
#include "ramses-citymodel/PT2Element.h"
#include "ramses-citymodel/Reader.h"
#include "ramses-citymodel/TilePager.h"
#include "ramses-citymodel/Timer.h"
#include "ramses-citymodel/Vector2.h"

namespace ramses
{
    class RamsesFramework;
    class Camera;
    class MeshNode;
}

/// Citymodel main class.
class Citymodel : public IInputReceiver
{
public:
    /// Interaction mode of the map
    enum EInteractionMode
    {
        eFollowCarsor = 0,
        eFreeMove,
        eEditRoute,
        eEditNamingPoints
    };

    /// Constructor.
    /** @param argc Number of arguments handed over by main.
     *  @param argv Array of argument handed over by main.
     *  @param framework ramses framework used to create client.
     *  @param sceneId Scene id for the created RAMSES scene. */
    Citymodel(CitymodelArguments arguments, ramses::RamsesFramework& framework);

    /// Destructor.
    ~Citymodel();

    float getFovy() const;

    /// Returns the scene id.
    /** @return The scene id. */
    ramses::sceneId_t getSceneId() const;

    /// Returns the file reader for loading tiles and the scene.
    /** @return The reader. */
    Reader& getReader();

    /// Returns the pager for asynchronous tile loading.
    /** @return The pager. */
    TilePager& getTilePager();

    /// Add tile to be read to the pager.
    /** @param tile The tile. */
    void addTileToRead(Tile* tile);

    /// Remove tile to be read from the pager.
    /** @param tile The tile. */
    void removeTileToRead(Tile* tile);

    /// Adds a tile that is invisible to the list of tiles, that can be deleted.
    /** A tile is deleted, if it is in this list for a certain number of frames.
     *  @param tile The tile. */
    void addTileToDelete(Tile* tile);

    /// Removes a tile from the list of tiles, that can be deleted.
    /**  @param tile The tile. */
    void removeTileToDelete(Tile* tile);

    /// Returns the Ramses scene.
    /** @return The scene. */
    ramses::Scene& getRamsesScene();

    /// Returns the Ramses client.
    /** @return The client. */
    ramses::RamsesClient& getRamsesClient();

    /// Returns the Ramses render group.
    /** @return The render group. */
    ramses::RenderGroup& getRenderGroup();

    /// Returns, whether the main loop shall exit.
    /** @return "true", for exit. */
    bool shouldExit() const;

    /// Does all processing for the current frame.
    /** Animation / culling / handling of tile paging.
     *  @param dt Elapsed frame time. */
    void doFrame(float dt);

    /// Handles mouse events.
    /** @param eventType Type of the event.
     *  @param mousePosX Mouse x position.
     *  @param mousePosY Mouse y position. */
    void mouseEvent(ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override;

    /// Sets the current interaction mode.
    /** @param interactionMode The new value for the interaction mode. */
    void setInteractionMode(EInteractionMode interactionMode);

    /// Sets whether the interaction mode is switchable by user input or not.
    /** @param value The new value for the flag. */
    void enableInteractionModeSwitchable(bool value);

    /// Returns, wheter a second map instance is requested via command line option.
    /** @return "true", when a second map instance is requsted. */
    bool getSecondMap() const;

private:
    /// Initializes the scene and creates the Ramses proxy.
    void init();

    /// Creates the effects, needed for the scene.
    void createEffects();

    /// Creates an effect.
    /** @param effectDesc The effect description to be used.
     *  @param vertexShaderFile Filename of the vertex shader file.
     *  @param fragmentShaderFile Filename of the fragment shader file.
     *  @return The created effect. */
    ramses::Effect*
    createEffect(ramses::EffectDescription& effectDesc, const char* vertexShaderFile, const char* fragmentShaderFile);

    /// Does the complete setup of the renderable scene (reading / building the culling tree / creating the naming
    /// labels).
    void readScene();

    /// Builds the tree for hierarchical view frustum culling.
    void buildTree();

    /// Moves the carsor and the camera and updates the naming label positions.
    /** @param dt Elapsed time to advance the animation. */
    void doAnimation(float dt);

    /// Computes the visible tiles of the scene by hierarchical frustum culling.
    void doCulling();

    /// Does all paging steps.
    /** Adds/removes tiles to/from the asynchronous pager.
     *  Handles newly read tiles.
     *  Deletes tiles, which are not visible for a long time. */
    void doPaging();

    /// Creates the route node and adds it to the camera node.
    void createRoute();

    /// Computes the perspective projection matrix.
    /** Needed for projecting the naming labels. */
    void computeProjectionMatrix();

    /// Creates the naming root manager and all naming labels.
    void createNaming();

    /// Creates a single name label.
    /** @param p0 The anchor point in 3D for the name label.
     *  @param p1 The scond point to define the tangent direction.
     *  @param nameString The name label string. */
    void
    createName(const Vector3& p0, const Vector3& p1, const std::string& nameString);

    /// Updates the status line string (measured framerate and cpu-load).
    void updatePerformanceValuesString(float fps, float cpuLoad);

    /// Sets the car position uniform in all materials.
    /** @param carPos The car position. */
    void setCarPosInMaterials(const Vector3& carPos);

    /// Creates geometry for a marker in the scene.
    void createMarkerGeometry();

    /// Create a marker in the scene at a certain position.
    /** @param position Center position of the marker.
     *  @param size Size of the marker. */
    void createMarker(const Vector3& position, float size);

    /// Computes a ray intersection from a mouse position with the scene.
    /** @param mousePos 2D screen coordinates of the intersection ray.
     *  @param position The computed intersection of the scene is returned here.
     *  @return "true", when a intersection point was found. */
    bool computeIntersection(const Vector2& mousePos, Vector3& position);

    /// Computes a ray intersection with the scene.
    /** @param p Start point of the ray.
     *  @param d Direction of the ray.
     *  @param position The computed intersection of the scene is returned here.
     *  @return "true", when a intersection point was found. */
    bool computeIntersection(const Vector3& p, const Vector3& d, Vector3& position);

    /// Computes the focus point, which is used as a rotation center in interactive mode.
    /** @param mousePos 2D screen coordinates of the intersection ray. */
    void computeFocusPoint(const Vector2& mousePos);

    /// Adds a route point.
    /** @param mousePos 2D screen coordinates of the intersection ray. */
    void addRoutePoint(const Vector2& mousePos);

    /// Adds a naming point.
    /** @param mousePos 2D screen coordinates of the intersection ray. */
    void addNamingPoint(const Vector2& mousePos);

    /// Deletes the last route point.
    void deleteLastRoutePoint();

    /// Delets the last naming point.
    void deleteLastNamingPoint();

    /// Prints the route points on the console.
    void printRoute();

    /// Prints the naming points on the console.
    void printNamingPoints();

    CitymodelArguments m_arguments;

    /// GUI Overlay elements.
    CitymodelGUIOverlay m_guiOverlay;

    /// The ramses framework passed into.
    ramses::RamsesFramework& m_framework;

    /// The RAMSES client.
    ramses::RamsesClient* m_ramsesClient = nullptr;

    /// Translate node for the carsor.
    ramses::Node* m_carsorTranslation = nullptr;

    /// Rotate node for the carsor.
    ramses::Node* m_carsorRotation = nullptr;

    ramses::Node* m_rootCameraTranslate = nullptr;

    /// The carsor model.
    ramses::Node* m_carsor = nullptr;

    /// The current view frustum for culling.
    Frustum m_frustum;

    /// The render pass.
    ramses::RenderPass* m_renderPass = nullptr;

    /// The render group.
    ramses::RenderGroup* m_renderGroup = nullptr;

    /// The camera.
    ramses::Camera* m_camera = nullptr;

    /// Translation node of the camera.
    ramses::Node* m_cameraRotate = nullptr;

    /// Rotation node of the camera.
    ramses::Node* m_cameraTranslate = nullptr;

    /// Class for creating the route geometry.
    LineContainer* m_route = nullptr;

    /// The naming manager.
    NamingManager* m_naming = nullptr;

    /// Name label for the current performance value string.
    Name* m_statusName = nullptr;

    /// The scene of the citymodel.
    CitymodelScene* m_scene = nullptr;

    /// The RAMSES scene.
    ramses::Scene* m_ramsesScene = nullptr;

    /// The view frustum culling tree.
    CullingNode* m_cullingTree = nullptr;

    /// Effect for the debug markers.
    ramses::Effect* m_markerEffect = nullptr;

    /// The scene and tile reader.
    Reader* m_reader = nullptr;

    /// The current frame number.
    uint32_t m_frame = 0;

    /// The current projection matrix.
    Matrix44 mProjectionMatrix;

    /// The asynchronous tile pager.
    TilePager m_pager;

    /// List of new tiles, which shall be added to the pager for reading.
    std::vector<Tile*> m_tilesAddToRead;

    /// List of tiles, which shall be removed from the pager for reading.
    std::vector<Tile*> m_tilesRemoveToRead;

    /// List of tiles, that can be potentially deleted, if they are invisible long enough.
    /** Invisible tiles are added to this list, and they are then counted down and finally
     *  deleted, if they didn't get visible again. */
    std::set<Tile*> m_tilesToDelete;

    /// Setting whether scene is animated (moving scene, streetnames fade in/out, etc.)
    bool m_showAnimation = true;

    /// Number of rounds the route has been driven.
    uint32_t m_numRoundsDriven = 0;

    /// Flag, if the main loop shall exit.
    bool m_exit = false;

    /// Number of tiles that are to be loaded by the tile pager.
    int32_t m_openTilesToLoad = 0;

    /// Current pitch angle of the camera.
    PT2Element m_pitch;

    /// Destination value of the pitch angle.
    float m_destinationPitch = 25.0f;

    /// Current yaw angle of the camera.
    PT2Element m_yaw;

    /// Destination value of the yaw angle.
    float m_destinationYaw = 0.0f;

    /// Current distance value of the camera.
    PT2Element m_distance;

    /// Destination value of the distance value.
    float m_destinationDistance = 225.0f;

    /// Current camera x position.
    PT2Element m_xPosition;

    /// Current camera y position.
    PT2Element m_yPosition;

    /// Current camera z position.
    PT2Element m_zPosition;

    /// Destination position of the camera's view point.
    Vector3 m_destinationPosition;

    /// Last mouse position.
    Vector2 m_mousePos;

    /// Mouse position when the left button was pressed.
    Vector2 m_mousePosLButtonDown;

    /// Mouse position when the middle button was pressed.
    Vector2 m_mousePosMButtonDown;

    /// Flag, if the left mouse button is pressed.
    bool m_leftButtonPressed = false;

    /// Flag, if the middle mouse button is pressed.
    bool m_middleButtonPressed = false;

    /// Time of the last left button click.
    Timer m_lastMouseLButtonClickTime;

    /// Time of the last right button click.
    Timer m_lastMouseRButtonClickTime;

    /// Geometry for a marker.
    ramses::GeometryBinding* m_markerGeometry = nullptr;

    /// The route points.
    std::vector<Vector3> m_routePoints;

    /// The naming points.
    std::vector<Vector3> m_namingPoints;

    /// The current interaction mode.
    EInteractionMode m_interactionMode = eFollowCarsor;

    /// Flag, if the interaction mode can be switched via mouse or touch events.
    bool m_interactionModeCanBeSwitched = true;

    /// Time of the last mouse or touch input event.
    Timer m_lastInputTime;

    /// Timer for FPS measuring.
    Timer m_fpsMeasureTimer;

    /// Number of frames done for the current FPS measuring period.
    uint32_t m_measureFrames = 0;

    /// Speed factor for mouse and touch movements.
    float m_mouseAndTouchInputSpeed = 1.0f;
};

#endif
