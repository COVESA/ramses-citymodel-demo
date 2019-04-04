//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/TileResourceContainer.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-citymodel/CitymodelScene.h"
#include "ramses-citymodel/Material.h"
#include "ramses-citymodel/Reader.h"

#include "assert.h"

void TileResourceContainer::addGeometryNode(GeometryNode* geometryNode)
{
    assert(nullptr != geometryNode);
    m_geometryNodes.insert(geometryNode);
}

void TileResourceContainer::destroyGeometryNodes()
{
    for (GeometryNode* geometryNode : m_geometryNodes)
    {
        delete geometryNode;
    }
    m_geometryNodes.clear();
}

void TileResourceContainer::addMaterial(Material* material)
{
    assert(nullptr != material);
    m_materials.insert(material);
}

void TileResourceContainer::destroyMaterials()
{
    for (Material* material : m_materials)
    {
        delete material;
    }
    m_materials.clear();
}

void TileResourceContainer::addResource(const ramses::Resource* resource)
{
    assert(nullptr != resource);
    m_resources.insert(resource);
}

void TileResourceContainer::addSceneObject(ramses::SceneObject* sceneObject)
{
    assert(nullptr != sceneObject);
    m_sceneObjects.insert(sceneObject);
}

void TileResourceContainer::destroyResources(ramses::RamsesClient& client)
{
    for (const ramses::Resource* resource : m_resources)
    {
        client.destroy(*resource);
    }
    m_resources.clear();
}

void TileResourceContainer::destroySceneObjects(ramses::Scene& scene, ramses::RenderGroup& renderGroup)
{
    for (ramses::SceneObject* sceneObject : m_sceneObjects)
    {
        if (sceneObject->getType() == ramses::ERamsesObjectType_MeshNode)
        {
            ramses::MeshNode* mesh = reinterpret_cast<ramses::MeshNode*>(sceneObject);
            renderGroup.removeMeshNode(*mesh);
        }
        scene.destroy(*sceneObject);
    }
    m_sceneObjects.clear();
}

void TileResourceContainer::destroy(ramses::RamsesClient& client,
                                    ramses::Scene&        scene,
                                    ramses::RenderGroup&  renderGroup)
{
    destroyMaterials();
    destroyGeometryNodes();

    destroyResources(client);
    destroySceneObjects(scene, renderGroup);
}

void TileResourceContainer::computeIntersection(const Vector3& p, const Vector3& d, float& r)
{
    for (auto geometryNode : m_geometryNodes)
    {
        const std::vector<Vector3>&  positions     = geometryNode->m_positionsData;
        const std::vector<uint32_t>& indices       = geometryNode->m_indexData;
        const uint32_t               numberIndices = static_cast<uint32_t>(indices.size());
        for (uint32_t i = 0; i < numberIndices; i += 3)
        {
            const Vector3& a            = positions[indices[i + 0]];
            const Vector3& b            = positions[indices[i + 1]];
            const Vector3& c            = positions[indices[i + 2]];
            const float    intersection = ComputeIntersectionWithTriangle(p, d, a, b, c);
            if (intersection >= 0.0)
            {
                if (intersection < r)
                {
                    r = intersection;
                }
            }
        }
    }
}

float TileResourceContainer::ComputeIntersectionWithTriangle(
    const Vector3& p, const Vector3& d, const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    constexpr float epsilon = 0.0000001f;
    const Vector3   edge1   = p1 - p0;
    const Vector3   edge2   = p2 - p0;
    const Vector3   h       = d.cross(edge2);
    const float     a       = edge1.dot(h);
    if (a > -epsilon && a < epsilon)
    {
        return -1.0f;
    }
    const float   f = 1.0f / a;
    const Vector3 s = p - p0;
    const float   u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f)
    {
        return -1.0f;
    }
    const Vector3 q = s.cross(edge1);
    const float   v = f * d.dot(q);
    if (v < 0.0f || u + v > 1.0f)
    {
        return -1.0f;
    }
    const float t = f * edge2.dot(q);
    if (t > epsilon)
    {
        return t;
    }
    else
    {
        return -1.0f;
    }
}
