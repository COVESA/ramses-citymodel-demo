//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/ImageBox.h"
#include "ramses-client.h"

ImageBox::ImageBox(ramses::Texture2D&    texture,
                   uint32_t              width,
                   uint32_t              height,
                   EBlendMode            blendMode,
                   ramses::RamsesClient& client,
                   ramses::Scene&        scene,
                   ramses::RenderGroup*  renderGroup,
                   int32_t               renderOrder,
                   const ramses::Effect& effect,
                   bool                  flipVertical,
                   ramses::Node*         parent)
    : GraphicalItem(scene, client)
    , m_width(width)
    , m_height(height)
    , m_flipVertical(flipVertical)
    , m_translateNode(*scene.createNode())
    , m_scaleNode(*scene.createNode())
{
    ramses::TextureSampler* textureSampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureSamplingMethod_Linear_MipMapNearest,
                                                                        ramses::ETextureSamplingMethod_Linear,
                                                                        texture);
    init(textureSampler, blendMode, renderGroup, renderOrder, effect, parent);
}

ImageBox::ImageBox(ramses::StreamTexture& texture,
                   uint32_t               width,
                   uint32_t               height,
                   EBlendMode             blendMode,
                   ramses::RamsesClient&  client,
                   ramses::Scene&         scene,
                   ramses::RenderGroup*   renderGroup,
                   int32_t                renderOrder,
                   const ramses::Effect&  effect,
                   bool                   flipVertical,
                   ramses::Node*          parent)
    : GraphicalItem(scene, client)
    , m_width(width)
    , m_height(height)
    , m_flipVertical(flipVertical)
    , m_translateNode(*scene.createNode())
    , m_scaleNode(*scene.createNode())
{
    ramses::TextureSampler* textureSampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureSamplingMethod_Linear_MipMapNearest,
                                                                        ramses::ETextureSamplingMethod_Linear,
                                                                        texture);
    init(textureSampler, blendMode, renderGroup, renderOrder, effect, parent);
}

void ImageBox::init(ramses::TextureSampler* textureSampler,
                    EBlendMode              blendMode,
                    ramses::RenderGroup*    renderGroup,
                    int32_t                 renderOrder,
                    const ramses::Effect&   effect,
                    ramses::Node*           parent)
{
    m_sceneObjects.push_back(textureSampler);

    if (parent)
    {
        parent->addChild(m_translateNode);
    }
    m_translateNode.addChild(m_scaleNode);
    m_sceneObjects.push_back(&m_scaleNode);
    m_sceneObjects.push_back(&m_translateNode);

    m_appearance = m_scene.createAppearance(effect);
    m_sceneObjects.push_back(m_appearance);

    setBlendMode(*m_appearance, blendMode);

    createGeometry(*textureSampler, m_client, m_scene, renderGroup, *m_appearance, renderOrder);
}

void ImageBox::setColor(float r, float g, float b, float a)
{
    ramses::UniformInput colorInput;
    m_appearance->getEffect().findUniformInput("u_color", colorInput);
    m_appearance->setInputValueVector4f(colorInput, r, g, b, a);
}

void ImageBox::setBlendMode(ramses::Appearance& appearance, EBlendMode blendMode)
{
    switch (blendMode)
    {
    case EBlendMode_Normal:
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_Zero,
                                      ramses::EBlendFactor_One);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        break;
    }
    case EBlendMode_PremultipliedAlpha:
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_Zero);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        break;
    }
    default:
        break;
    }
}

void ImageBox::createGeometry(ramses::TextureSampler& textureSampler,
                              ramses::RamsesClient&   client,
                              ramses::Scene&          scene,
                              ramses::RenderGroup*    renderGroup,
                              ramses::Appearance&     appearance,
                              int32_t                 renderOrder)
{
    float vertexPositionsArray[] = {0.0f,
                                    0.0f,
                                    static_cast<float>(m_width),
                                    0.0f,
                                    0.0f,
                                    static_cast<float>(m_height),
                                    static_cast<float>(m_width),
                                    static_cast<float>(m_height)};

    const ramses::Vector2fArray& vertexPositions = *client.createConstVector2fArray(4, vertexPositionsArray);
    m_clientResources.push_back(&vertexPositions);

    m_textureCoords = scene.createVertexDataBuffer( TextureCoordinatesSizeInBytes, ramses::EDataType_Vector2F, "some varying texturecoodinates");
    setCrop();

    uint16_t                   indicesArray[] = {0, 1, 2, 2, 1, 3};
    const ramses::UInt16Array& indices        = *client.createConstUInt16Array(6, indicesArray);

    const ramses::Effect&    effect   = appearance.getEffect();
    ramses::GeometryBinding& geometry = *scene.createGeometryBinding(effect);
    m_sceneObjects.push_back(&geometry);
    geometry.setIndices(indices);
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    effect.findAttributeInput("a_position", positionsInput);
    effect.findAttributeInput("a_texcoord", texcoordsInput);
    geometry.setInputBuffer(positionsInput, vertexPositions);
    geometry.setInputBuffer(texcoordsInput, *m_textureCoords);

    ramses::UniformInput textureInput;
    effect.findUniformInput("textureSampler", textureInput);
    appearance.setInputTexture(textureInput, textureSampler);
    appearance.setCullingMode(ramses::ECullMode_Disabled);

    ramses::MeshNode& meshNode = *scene.createMeshNode();
    m_sceneObjects.push_back(&meshNode);
    meshNode.setAppearance(appearance);
    meshNode.setGeometryBinding(geometry);
    m_scaleNode.addChild(meshNode);

    renderGroup->addMeshNode(meshNode, renderOrder);
    setPosition(0, 0);
}

void ImageBox::setPosition(int32_t x, int32_t y)
{
    m_translateNode.setTranslation(static_cast<float>(x), static_cast<float>(y), -0.5f);
}

void ImageBox::setScaling(float scale, bool doCentricScaling)
{
    m_scaleNode.setScaling(scale, scale, scale);
    if(doCentricScaling)
    {
        m_scaleNode.setTranslation( static_cast<float>(m_width) * -0.5f * (scale - 1.f) , static_cast<float>(m_height) * -0.5f * (scale - 1.f), 0.f);
    }
}

void ImageBox::bindColorToDataObject(ramses::DataObject& dataObject)
{
    ramses::UniformInput colorInput;
    m_appearance->getEffect().findUniformInput("u_color", colorInput);
    m_appearance->bindInput(colorInput, dataObject);
}

void ImageBox::setCrop(int deltaLeft, int deltaRight, int deltaTop, int deltaBottom)
{
    const float widthIncrement  = 1.f / static_cast<float>(m_width);
    const float heightIncrement = 1.f / static_cast<float>(m_height);
    const float left            = widthIncrement  * static_cast<float>(deltaLeft);
    const float right           = widthIncrement  * static_cast<float>(static_cast<int>(m_width) - deltaRight);
    const float top             = heightIncrement * static_cast<float>(static_cast<int>(m_height) - deltaTop);
    const float bottom          = heightIncrement * static_cast<float>(deltaBottom);

    TextureCoordinates textureCoordsData = { left,  bottom
                                         , right, bottom
                                         , left,  top
                                         , right, top
                                         };
    if(m_flipVertical)
        flipTextureCoordinates(textureCoordsData);
    m_textureCoords->setData(reinterpret_cast<const char*>(textureCoordsData.data()), TextureCoordinatesSizeInBytes);
}

void ImageBox::flipTextureCoordinates(TextureCoordinates& textureCoordinates)
{
    const int offset = TextureCoordinatesSize / 2;
    for(int i = 0; i < offset; ++i)
    {
        const float backupTC = textureCoordinates[i];
        textureCoordinates[i] = textureCoordinates[i+offset];
        textureCoordinates[i+offset] = backupTC;
    }
}
