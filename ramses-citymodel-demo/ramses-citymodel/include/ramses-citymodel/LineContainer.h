//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_LINECONTAINER_H
#define RAMSES_CITYMODEL_LINECONTAINER_H

#include "ramses-citymodel/Vector3.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/UInt16Array.h"

#include "vector"
#include "string"

namespace ramses
{
    class RamsesClient;
}

/// Class for a line container.
/** All polylines in this container have the same style. */
class LineContainer
{
public:
    /// Line cap type.
    enum ECapType
    {
        ECapType_Round = 0,
        ECapType_Flat
    };

    /// Constructor.
    LineContainer(const Vector3&        color,
                  const Vector3&        colorInvisible,
                  float                 width,
                  const Vector3&        up,
                  ramses::Scene&        scene,
                  ramses::RamsesClient& client,
                  ramses::RenderGroup&  renderGroup,
                  const std::string&    resourceFilePath);

    /// Activates rendering of invisible parts with a separate color.
    /** @param color The color for rendering the invisible parts. */
    void activateRenderingOfInvisibleParts(const Vector3& color);

    /// Adds a polyline to this line container.
    /** @param points The points of the polyline.
     *  @param startCap Type of the start cap.
     *  @param endCap Type of the end cap. */
    void addPolyline(const std::vector<Vector3>& points, ECapType startCap, ECapType endCap);

    /// Creates The ramses node for the line container.
    void updateGeometry();

private:
    /// Creates the effects needed for rendering.
    void createEffects(const std::string& resourceFilePath);

    /// Create the geometry for rendering.
    void createGeometry();

    /// Adds a line segment to this container.
    /** @param p1 First point of the segment.
     *  @param p2 Second point of the segment.
     *  @param startCap Flag, whether the segment has a round start cap or not.
     *  @param endCap Flag, whether the segment has a round end cap or not.*/
    void addSegment(const Vector3& p1, const Vector3& p2, bool startCap, bool endCap);

    /// Vector of the line segments vertices.
    std::vector<float> m_vertex;

    /// Vector of the line segments anti aliasing parameters.
    std::vector<float> m_param;

    /// Fill color for the line.
    Vector3 m_color;

    /// Color for rendering invisible parts.
    Vector3 m_colorInvisible;

    /// Width of the line in pixel.
    float m_width = 0.0;

    /// Up vector.
    Vector3 m_up;

    /// Flag, if the invisible parts shall be rendered. The color mColorInvisible is then used.
    bool m_renderInvisible = false;

    /// Scaling parameters for effect.
    /** mScale[0]: 1 / pixelToWorldScale
     *  mScale[1]: width / 2 [Pixel]
     *  mScale[2]: pixelToWorldScale * width / 2 [Pixel] */
    Vector3 m_scale;

    /// Effect for building transparency in the alpha channel.
    ramses::Effect* m_effectLine = nullptr;

    /// Effect for filling the visible parts in the color buffer.
    ramses::Effect* m_effectLineFillVisible = nullptr;

    /// Effect for filling the invisible parts in the color buffer.
    ramses::Effect* m_effectLineFillInvisible = nullptr;

    /// The RAMSES client.
    ramses::RamsesClient& m_ramsesClient;

    /// The RAMSES scene.
    ramses::Scene& m_ramsesScene;

    /// The RAMSES render group.
    ramses::RenderGroup& m_renderGroup;

    /// Geometry binding for rendering the fill alpha pass.
    ramses::GeometryBinding* m_geometryFillAlpha;

    /// Geometry binding for rendering the visible part.
    ramses::GeometryBinding* m_geometryRenderVisible;

    /// Geometry binding for rendering the invisible part.
    ramses::GeometryBinding* m_geometryRenderInvisible;

    /// Mesh node for rendering the fill alpha pass.
    ramses::MeshNode* m_meshFillAlpha;

    /// Mesh node for rendering the visible part.
    ramses::MeshNode* m_meshRenderVisible;

    /// Mesh node for rendering the invisible part.
    ramses::MeshNode* m_meshRenderInvisible;

    /// Position vertex data.
    const ramses::Vector3fArray* m_positions = nullptr;

    /// Normals vertex data.
    const ramses::Vector3fArray* m_normals = nullptr;
};

#endif
