//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Reader.h"
#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/CitymodelScene.h"
#include "ramses-citymodel/EObjectType.h"
#include "ramses-citymodel/Material.h"
#include "ramses-citymodel/TileResourceContainer.h"
#include "ramses-citymodel/Vector2.h"
#include "ramses-citymodel/Vector3.h"
#include "ramses-citymodel/Vector4.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/AppearanceEnums.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/UniformInput.h"

#include "lz4.h"
#include "istream"
#include "assert.h"
#include "cstring"

Reader::FileReference::FileReference(uint64_t position, uint32_t compressedSize, uint32_t uncompressedSize)
    : m_position(position)
    , m_compressedSize(compressedSize)
    , m_uncompressedSize(uncompressedSize)
{
}

uint64_t Reader::FileReference::position() const
{
    return m_position;
}

uint32_t Reader::FileReference::compressedSize() const
{
    return m_compressedSize;
}

uint32_t Reader::FileReference::uncompressedSize() const
{
    return m_uncompressedSize;
}

Reader::Reader(Citymodel& citymodel)
    : m_citymodel(citymodel)

{
}

void* Reader::readObject(TileResourceContainer& resourceContainer)
{
    EObjectType type;
    uint32_t    valueAsUInt32 = 0;
    read_uint32(valueAsUInt32);
    type = static_cast<EObjectType>(valueAsUInt32);

    void*    retval(0);
    uint32_t id(0);

    switch (type)
    {
    case EType_Null:
    {
        return 0;
    }
    case EType_Index:
    {
        uint32_t index;
        read_uint32(index);
        if (index >= m_object.size())
        {
            printf("Reader::readObject ERROR - id %d out of range mObject.size: %ld\n", id, m_object.size());
        }
        return m_object[index];
    }

    case EType_Node:
    {
        id     = createId();
        retval = readNode(resourceContainer);
        break;
    }
    case EType_MeshNode:
    {
        id     = createId();
        retval = readMeshNode(resourceContainer);
        break;
    }
    case EType_Material:
    {
        id     = createId();
        retval = readMaterial(resourceContainer);
        break;
    }
    case EType_GeometryNode:
    {
        id     = createId();
        retval = readGeometryNode(resourceContainer);
        break;
    }
    case EType_VertexArrayResource2f:
    {
        id     = createId();
        retval = readVector2fArrayResource(resourceContainer);
        break;
    }
    case EType_VertexArrayResource3f:
    {
        id     = createId();
        retval = readVector3fArrayResource(resourceContainer);
        break;
    }
    case EType_VertexArrayResource4f:
    {
        id     = createId();
        retval = readVector4fArrayResource(resourceContainer);
        break;
    }
    case EType_IndexArrayResource:
    {
        id     = createId();
        retval = const_cast<void*>(reinterpret_cast<const void*>(readIndexArrayResource(resourceContainer)));
        break;
    }
    case EType_Texture2DResource:
    {
        id     = createId();
        retval = readTexture2DResource(resourceContainer);
        break;
    }
    case EType_Scene:
    {
        id     = createId();
        retval = readScene(resourceContainer);
        break;
    }

    case EType_Tile:
    {
        id     = createId();
        retval = readTile();
        break;
    }

    default:
    {
        printf("CReader::readObject Wrong object id: %d !\n", type);
        return 0;
    }
    }

    assert(retval != NULL);
    if (retval)
    {
        m_object[id] = retval;
    }
    return retval;
}

void Reader::open(const std::string& filename)
{
    m_object.clear();
    m_f.close();
    m_f.open(filename.c_str(), std::fstream::binary | std::fstream::in);
    if (!m_f.good())
    {
        printf("CReader::open Could not open file: %s !!!\n", filename.c_str());
        exit(1);
    }

    uint32_t numberOfObjects = 0;
    m_f.seekg(-static_cast<int32_t>(sizeof(numberOfObjects)), m_f.end);
    m_f.read(reinterpret_cast<char*>(&numberOfObjects), sizeof(numberOfObjects));

    m_f.seekg(-static_cast<int32_t>(sizeof(FileReference) * numberOfObjects + sizeof(numberOfObjects)), m_f.end);
    for (uint32_t i = 0; i < numberOfObjects; i++)
    {
        uint64_t position;
        uint32_t compressedSize;
        uint32_t uncompressedSize;

        m_f.read(reinterpret_cast<char*>(&position), sizeof(position));
        m_f.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
        m_f.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));

        m_objectReferences.push_back(FileReference(position, compressedSize, uncompressedSize));
    }
}

void* Reader::read(uint32_t index, TileResourceContainer& resourceContainer, bool resetIds)
{
    uint32_t objectCount = static_cast<uint32_t>(m_object.size());

    assert(index < m_objectReferences.size());
    const FileReference& fileRef = m_objectReferences[index];

    char* compressedDataBuffer = new char[fileRef.compressedSize()];
    // CTime mTimer;
    m_f.seekg(fileRef.position());
    m_f.read(compressedDataBuffer, fileRef.compressedSize());

    m_dataBuffer = new uint8_t[fileRef.uncompressedSize()];
    m_data       = m_dataBuffer;

    uint32_t decompressedSize = LZ4_decompress_safe(compressedDataBuffer,
                                                    reinterpret_cast<char*>(m_dataBuffer),
                                                    fileRef.compressedSize(),
                                                    fileRef.uncompressedSize());
    if (decompressedSize != fileRef.uncompressedSize())
    {
        printf("CReader::open Failed to read decompress object decompressed size: %d differs from expected size: %d\n",
               decompressedSize,
               fileRef.uncompressedSize());
        exit(1);
    }

    delete[] compressedDataBuffer;

    if (!m_f.good())
    {
        printf("CReader::open Failed to read object\n");
        exit(1);
    }

    void* object = readObject(resourceContainer);

    delete[] m_dataBuffer;
    m_dataBuffer = 0;
    m_data       = 0;

    if (resetIds)
    {
        m_object.resize(objectCount);
    }

    return object;
}

std::mutex& Reader::getSceneLock()
{
    return m_sceneLock;
}

void Reader::read_uint8(uint8_t& value)
{
    std::memcpy(&value, m_data, sizeof(value));
    m_data += sizeof(value);
}

void Reader::read_uint32(uint32_t& value)
{
    memcpy(&value, m_data, sizeof(value));
    m_data += sizeof(value);
}

void Reader::read_int32(int32_t& value)
{
    memcpy(&value, m_data, sizeof(value));
    m_data += sizeof(value);
}

void Reader::read_uint64(uint64_t& value)
{
    memcpy(&value, m_data, sizeof(value));
    m_data += sizeof(value);
}

void Reader::read_float(float& value)
{
    memcpy(&value, m_data, sizeof(value));
    m_data += sizeof(value);
}

void Reader::read(Vector3& value)
{
    float x;
    float y;
    float z;
    read_float(x);
    read_float(y);
    read_float(z);
    value = Vector3(x, y, z);
}

void Reader::read(Vector4& color)
{
    float r;
    float g;
    float b;
    float a;
    read_float(r);
    read_float(g);
    read_float(b);
    read_float(a);
    color = Vector4(r, g, b, a);
}

void Reader::read(BoundingBox& bbox)
{
    Vector3 min;
    Vector3 max;
    read(min);
    read(max);
    bbox.set(min, max);
}

void Reader::read_string(std::string& value)
{
    uint32_t n;
    read_uint32(n);
    value = std::string(reinterpret_cast<const char*>(m_data), n);

    m_data += n;
}

ramses::Node* Reader::readNode(TileResourceContainer& resourceContainer)
{
    m_sceneLock.lock();
    ramses::Node* node = m_citymodel.getRamsesScene().createNode();
    m_sceneLock.unlock();
    resourceContainer.addSceneObject(node);

    readNode(node, resourceContainer);

    return node;
}

void Reader::readNode(ramses::Node* node, TileResourceContainer& resourceContainer)
{
    Vector3 rotation;
    Vector3 translation;
    Vector3 scaling;
    read(rotation);
    read(translation);
    read(scaling);

    m_sceneLock.lock();
    node->setRotation(rotation.getX(), rotation.getY(), rotation.getZ());
    node->setTranslation(translation.getX(), translation.getY(), translation.getZ());
    node->setScaling(scaling.getX(), scaling.getY(), scaling.getZ());
    m_sceneLock.unlock();

    uint32_t childCount;
    read_uint32(childCount);
    for (uint32_t i = 0; i < childCount; i++)
    {
        ramses::Node* child = static_cast<ramses::Node*>(readObject(resourceContainer));
        if (!child)
        {
            printf("CReader::readObject ERROR - Could not read child node !!!\n");
        }
        else
        {
            m_sceneLock.lock();
            node->addChild(*child);
            m_sceneLock.unlock();
        }
    }
}

ramses::Node* Reader::readMeshNode(TileResourceContainer& resourceContainer)
{
    m_sceneLock.lock();
    ramses::MeshNode* mesh = m_citymodel.getRamsesScene().createMeshNode();
    m_sceneLock.unlock();
    resourceContainer.addSceneObject(mesh);

    readNode(mesh, resourceContainer);

    uint32_t startIndex;
    int32_t  indexCount;
    read_uint32(startIndex);
    read_int32(indexCount);

    Material* material = static_cast<Material*>(readObject(resourceContainer));
    if (!material)
    {
        printf("CReader::readMesh ERROR - Could not read material !!!\n");
        assert(material);
        return nullptr;
    }

    uint32_t renderOrder;
    read_uint32(renderOrder);

    GeometryNode* geometryNode = static_cast<GeometryNode*>(readObject(resourceContainer));

    ramses::Appearance& appearance = material->getAppearance();
    const ramses::Effect& effect = material->getEffect();

    m_sceneLock.lock();

    m_citymodel.getRenderGroup().addMeshNode(*mesh, renderOrder);
    ramses::GeometryBinding* geometry = m_citymodel.getRamsesScene().createGeometryBinding(effect);

    ramses::AttributeInput normalsInput;
    if (effect.findAttributeInput("a_normal", normalsInput) == ramses::StatusOK)
    {
        geometry->setInputBuffer(normalsInput, *geometryNode->m_normals);
    }

    ramses::AttributeInput positionsInput;
    if (effect.findAttributeInput("a_position", positionsInput) == ramses::StatusOK)
    {
        geometry->setInputBuffer(positionsInput, *geometryNode->m_positions);
    }

    if (nullptr != geometryNode->m_texCoords)
    {
        ramses::AttributeInput texCoordsInput;
        if (effect.findAttributeInput("a_texcoord", texCoordsInput) == ramses::StatusOK)
        {
            geometry->setInputBuffer(texCoordsInput, *geometryNode->m_texCoords);
        }
    }
    if (nullptr != geometryNode->m_texCoords2)
    {
        ramses::AttributeInput texCoords2Input;
        if (effect.findAttributeInput("a_customAttribute", texCoords2Input) == ramses::StatusOK)
        {
            geometry->setInputBuffer(texCoords2Input, *geometryNode->m_texCoords2);
        }
    }
    geometry->setIndices(*geometryNode->m_indexArray);

    mesh->setAppearance(appearance);
    mesh->setGeometryBinding(*geometry);
    mesh->setStartIndex(startIndex);
    mesh->setIndexCount(indexCount);

    m_sceneLock.unlock();

    resourceContainer.addSceneObject(geometry);

    return mesh;
}

CTMuint Reader::CTMRead(void* buffer, CTMuint count, void* userData)
{
    Reader* reader = static_cast<Reader*>(userData);
    return reader->ctmRead(buffer, count);
}

CTMuint Reader::ctmRead(void* buffer, CTMuint count)
{
    memcpy(buffer, m_data, count);
    m_data += count;
    return count;
}

GeometryNode* Reader::readGeometryNode(TileResourceContainer& resourceContainer)
{
    uint32_t effectNumber;
    read_uint32(effectNumber);

    uint8_t useCTM;
    read_uint8(useCTM);

    ramses::Effect* effect       = getEffect(effectNumber);
    GeometryNode*   geometryNode = new GeometryNode();
    resourceContainer.addGeometryNode(geometryNode);

    const ramses::Vector3fArray* positions  = 0;
    const ramses::Vector3fArray* normals    = 0;
    const ramses::Vector2fArray* texCoords  = 0;
    const ramses::Vector4fArray* texCoords2 = 0;
    const ramses::UInt32Array*   indexArray = 0;

    if (useCTM)
    {
        CTMimporter ctm;
        ctm.LoadCustom(CTMRead, this);

        const uint32_t numberVertices = ctm.GetInteger(CTM_VERTEX_COUNT);
        const uint32_t numberIndices  = ctm.GetInteger(CTM_TRIANGLE_COUNT) * 3;

        const CTMfloat* positionsData = ctm.GetFloatArray(CTM_VERTICES);
        geometryNode->m_positionsData.resize(numberVertices);
        std::memcpy(geometryNode->m_positionsData.data(), positionsData, sizeof(CTMfloat) * numberVertices * 3);

        const CTMuint* indexData = ctm.GetIntegerArray(CTM_INDICES);
        geometryNode->m_indexData.resize(numberIndices);
        std::memcpy(geometryNode->m_indexData.data(), indexData, sizeof(CTMuint) * numberIndices);

        /// Releasing lock between this heavy-weight operations, so that the main thread can continue in between.

        m_sceneLock.lock();
        positions = m_citymodel.getRamsesClient().createConstVector3fArray(numberVertices, positionsData);
        m_sceneLock.unlock();

        m_sceneLock.lock();
        texCoords = m_citymodel.getRamsesClient().createConstVector2fArray(numberVertices, ctm.GetFloatArray(CTM_UV_MAP_1));
        m_sceneLock.unlock();

        m_sceneLock.lock();
        indexArray = m_citymodel.getRamsesClient().createConstUInt32Array(numberIndices, indexData);
        m_sceneLock.unlock();

        resourceContainer.addResource(positions);
        resourceContainer.addResource(texCoords);
        resourceContainer.addResource(indexArray);
    }
    else
    {
        void* positionsObject  = readObject(resourceContainer);
        void* normalsObject    = readObject(resourceContainer);
        void* texCoordsObject  = readObject(resourceContainer);
        void* texCoords2Object = readObject(resourceContainer);
        void* indexArrayObject = readObject(resourceContainer);

        if (0 != positionsObject)
        {
            positions = static_cast<ramses::Vector3fArray*>(positionsObject);
        }

        if (0 != normalsObject)
        {
            normals = static_cast<ramses::Vector3fArray*>(normalsObject);
        }

        if (0 != texCoordsObject)
        {
            texCoords = static_cast<ramses::Vector2fArray*>(texCoordsObject);
        }

        if (0 != texCoords2Object)
        {
            texCoords2 = static_cast<ramses::Vector4fArray*>(texCoords2Object);
        }

        if (0 != indexArrayObject)
        {
            indexArray = static_cast<ramses::UInt32Array*>(indexArrayObject);
        }
    }

    geometryNode->m_indexArray = indexArray;
    geometryNode->m_positions  = positions;
    geometryNode->m_normals    = normals;
    geometryNode->m_texCoords  = texCoords;
    geometryNode->m_texCoords2 = texCoords2;
    geometryNode->m_effect     = effect;

    return geometryNode;
}

Material* Reader::readMaterial(TileResourceContainer& resourceContainer)
{
    Vector4 diffuseColor;
    read(diffuseColor);

    uint32_t effectNumber;
    read_uint32(effectNumber);

    ramses::Texture2D*      texture = static_cast<ramses::Texture2D*>(readObject(resourceContainer));
    ramses::TextureSampler* sampler(0);
    ramses::Effect*         effect = getEffect(effectNumber);

    m_sceneLock.lock();
    ramses::Appearance* appearance = m_citymodel.getRamsesScene().createAppearance(*effect);
    appearance->setColorWriteMask(true, true, true, false);

    if (effectNumber == 6 || effectNumber == 5 || effectNumber == 2 || effectNumber == 3)
    {
        appearance->setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
                                       ramses::EBlendFactor_OneMinusSrcAlpha,
                                       ramses::EBlendFactor_One,
                                       ramses::EBlendFactor_One);
        appearance->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    }
    m_sceneLock.unlock();

    resourceContainer.addSceneObject(appearance);

    if (texture)
    {
        ramses::UniformInput input;
        m_sceneLock.lock();
        sampler = m_citymodel.getRamsesScene().createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                    ramses::ETextureAddressMode_Repeat,
                                                                    ramses::ETextureSamplingMethod_Bilinear,
                                                                    *texture);

        const ramses::status_t stat = effect->findUniformInput("u_texture", input);
        if (stat == ramses::StatusOK)
        {
            appearance->setInputTexture(input, *sampler);
        }
        m_sceneLock.unlock();

        resourceContainer.addSceneObject(sampler);
    }
    else
    {
        ramses::UniformInput   input;
        m_sceneLock.lock();
        const ramses::status_t stat = effect->findUniformInput("u_color", input);
        if (stat == ramses::StatusOK)
        {
            appearance->setInputValueVector4f(input, diffuseColor.getX(), diffuseColor.getY(), diffuseColor.getZ(), diffuseColor.getW());
        }
        m_sceneLock.unlock();
    }

    ramses::UniformInput m_carPosInput;
    ramses::UniformInput m_lightConeScaleInput;

    m_sceneLock.lock();
    effect->findUniformInput("u_carPos", m_carPosInput);
    effect->findUniformInput("u_lightConeScale", m_lightConeScaleInput);

    if (m_carPosInput.isValid())
    {
        appearance->bindInput(m_carPosInput, m_scene->getDataVectorOfCarPosition());
    }

    if (m_lightConeScaleInput.isValid())
    {
        appearance->bindInput(m_lightConeScaleInput, m_scene->getDataOfLightConeScale());
    }
    m_sceneLock.unlock();

    Material* material = new Material(*appearance, *effect, texture, sampler);
    resourceContainer.addMaterial(material);

    return material;
}

CitymodelScene* Reader::readScene(TileResourceContainer& resourceContainer)
{
    CitymodelScene* scene = new CitymodelScene(m_citymodel.getRamsesScene());
    m_scene               = scene;
    {
        uint32_t n;
        read_uint32(n);
        for (uint32_t i = 0; i < n; i++)
        {
            void* material = readObject(resourceContainer);
            if (!material)
            {
                printf("CReader::readScene Could not read material !!!\n");
                exit(1);
            }
        }
    }

    {
        uint32_t n;
        read_uint32(n);
        for (uint32_t i = 0; i < n; i++)
        {
            void* tile = readObject(resourceContainer);

            if (!tile)
            {
                printf("CReader::readScene Could not read tile !!!\n");
                exit(1);
            }
            Tile* ctile = static_cast<Tile*>(tile);
            scene->addTile(ctile);
        }
    }

    void* object = readObject(resourceContainer);
    scene->setCarsor(static_cast<ramses::Node*>(object));

    readAnimationPath(scene->getAnimationPath());
    readNames(scene->getNames());
    readNamePoints(scene->getNamePoints());
    readRoutePoints(scene->getRoutePoints());

    return scene;
}

Tile* Reader::readTile()
{
    BoundingBox bbox;
    read(bbox);
    Tile* tile = new Tile(bbox, m_citymodel, m_tileIndex++);
    return tile;
}

void Reader::read(uint8_t* dest, uint32_t size)
{
    memcpy(dest, m_data, size);
    m_data += size;
}

uint8_t* Reader::read(uint32_t size)
{
    uint8_t* retval = m_data;
    m_data += size;
    return retval;
}

void* Reader::readVector2fArrayResource(TileResourceContainer& resourceContainer)
{
    uint32_t n;
    read_uint32(n);

    uint32_t size = sizeof(float) * 2 * n;
    uint8_t* data = new uint8_t[size];
    read(data, size);

	m_sceneLock.lock();
    const ramses::Resource* returnValue =
        m_citymodel.getRamsesClient().createConstVector2fArray(n, reinterpret_cast<float*>(data));
    m_sceneLock.unlock();

    delete[] data;
    resourceContainer.addResource(returnValue);

    return const_cast<ramses::Resource*>(returnValue);
}

void* Reader::readVector3fArrayResource(TileResourceContainer& resourceContainer)
{
    uint32_t n;
    read_uint32(n);

    uint32_t size = sizeof(float) * 3 * n;
    uint8_t* data = new uint8_t[size];
    read(data, size);

    m_sceneLock.lock();
    const ramses::Resource* returnValue =
        m_citymodel.getRamsesClient().createConstVector3fArray(n, reinterpret_cast<float*>(data));
    m_sceneLock.unlock();

    delete[] data;
    resourceContainer.addResource(returnValue);

    return const_cast<ramses::Resource*>(returnValue);
}

void* Reader::readVector4fArrayResource(TileResourceContainer& resourceContainer)
{
    uint32_t n;
    read_uint32(n);

    uint32_t size = sizeof(float) * 4 * n;
    uint8_t* data = new uint8_t[size];
    read(data, size);

	m_sceneLock.lock();
    const ramses::Resource* returnValue =
        m_citymodel.getRamsesClient().createConstVector4fArray(n, reinterpret_cast<float*>(data));
    m_sceneLock.unlock();

    delete[] data;
    resourceContainer.addResource(returnValue);

    return const_cast<ramses::Resource*>(returnValue);
}

const ramses::UInt32Array* Reader::readIndexArrayResource(TileResourceContainer& resourceContainer)
{
    uint32_t n;
    read_uint32(n);

    int      size = sizeof(uint32_t) * n;
    uint8_t* data = new uint8_t[size];
    read(data, size);

    m_sceneLock.lock();
    const ramses::UInt32Array* array = m_citymodel.getRamsesClient().createConstUInt32Array(
        n, reinterpret_cast<uint32_t*>(data));
    m_sceneLock.unlock();
    delete[] data;
    resourceContainer.addResource(array);
    return array;
}

ramses::Texture2D* Reader::readTexture2DResource(TileResourceContainer& resourceContainer)
{
    uint32_t textureSize;
    read_uint32(textureSize);

    uint8_t* textureData = read(textureSize);

    struct astc_header
    {
        unsigned char magic[4];
        unsigned char blockdim_x;
        unsigned char blockdim_y;
        unsigned char blockdim_z;
        unsigned char xsize[3];
        unsigned char ysize[3];
        unsigned char zsize[3];
    };

    astc_header* header = reinterpret_cast<astc_header*>(textureData);

    uint32_t width  = header->xsize[0] + (header->xsize[1] << 8) + (header->xsize[2] << 16);
    uint32_t height = header->ysize[0] + (header->ysize[1] << 8) + (header->ysize[2] << 16);

    const uint32_t       numMipMaps = 1;
    ramses::MipLevelData mipLevelData(textureSize - sizeof(astc_header), textureData + sizeof(astc_header));

    m_sceneLock.lock();
    ramses::Texture2D* texture = m_citymodel.getRamsesClient().createTexture2D(
        width, height, ramses::ETextureFormat_ASTC_RGBA_12x12, numMipMaps, &mipLevelData, false);
    m_sceneLock.unlock();

    resourceContainer.addResource(texture);

    return texture;
}

uint32_t Reader::createId()
{
    uint32_t i = static_cast<uint32_t>(m_object.size());
    m_object.push_back(0);
    return i;
}

void Reader::readAnimationPath(AnimationPath& animationPath)
{
    uint32_t n;
    read_uint32(n);
    for (uint32_t i = 0; i < n; i++)
    {
        Vector3 carPosition;
        Vector3 carRotation;
        read(carPosition);
        read(carRotation);

        animationPath.add(AnimationPath::Key(carPosition, carRotation));
    }
}

void Reader::readNames(std::vector<std::string>& names)
{
    uint32_t n;
    read_uint32(n);
    for (uint32_t i = 0; i < n; i++)
    {
        std::string name;
        read_string(name);
        names.push_back(name);
    }
}

void Reader::readNamePoints(std::vector<Vector3>& namePoints)
{
    uint32_t n;
    read_uint32(n);
    for (uint32_t i = 0; i < n; i++)
    {
        Vector3 point;
        read(point);
        namePoints.push_back(point);
    }
}

void Reader::readRoutePoints(std::vector<Vector3>& routePoints)
{
    uint32_t n;
    read_uint32(n);
    for (uint32_t i = 0; i < n; i++)
    {
        Vector3 point;
        read(point);
        routePoints.push_back(point);
    }
}

ramses::Effect* Reader::getEffect(uint32_t effectNumber)
{
    assert(effectNumber < m_effects.size());
    return m_effects[effectNumber];
}

void Reader::addEffect(ramses::Effect* effect)
{
    m_effects.push_back(effect);
}

Vector3 Reader::ConvertXYZRotationToZYX(const Vector3& rotationXYZ)
{
    Vector3 rotationZYX;
    Matrix44::RotationEulerXYZ(rotationXYZ).toRotationEulerZYX(rotationZYX);
    return rotationZYX;
}
