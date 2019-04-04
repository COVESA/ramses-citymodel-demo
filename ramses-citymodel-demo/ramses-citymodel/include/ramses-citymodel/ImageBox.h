//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMOLIB_IMAGEBOX_H
#define RAMSES_DEMOLIB_IMAGEBOX_H

#include "ramses-client.h"
#include "ramses-citymodel/GraphicalItem.h"
#include "array"

class ImageBox : public GraphicalItem
{
public:
    enum EBlendMode
    {
        EBlendMode_Off,
        EBlendMode_Normal,
        EBlendMode_PremultipliedAlpha
    };

    ImageBox(ramses::Texture2D&    texture,
             uint32_t              width,
             uint32_t              height,
             EBlendMode            blendMode,
             ramses::RamsesClient& client,
             ramses::Scene&        scene,
             ramses::RenderGroup*  renderGroup,
             int32_t               renderOrder,
             const ramses::Effect& effect,
             bool                  flipVertical,
             ramses::Node*         parent);

    ImageBox(ramses::StreamTexture& texture,
             uint32_t               width,
             uint32_t               height,
             EBlendMode             blendMode,
             ramses::RamsesClient&  client,
             ramses::Scene&         scene,
             ramses::RenderGroup*   renderGroup,
             int32_t                renderOrder,
             const ramses::Effect&  effect,
             bool                   flipVertical,
             ramses::Node*          parent);

    void setColor(float r, float g, float b, float a);
    void setPosition(int32_t x, int32_t y);
    void setScaling(float s = 1.f, bool doCentricScaling = false);
    void setCrop(int deltaLeft = 0, int deltaRight = 0, int deltaTop = 0, int deltaBottom = 0);
    void bindColorToDataObject(ramses::DataObject& dataObject);

private:
    using TextureCoordinates = std::array<float, 8>;
    static constexpr uint32_t TextureCoordinatesSize        = std::tuple_size<TextureCoordinates>::value;
    static constexpr uint32_t TextureCoordinatesNumber      = TextureCoordinatesSize / 2;
    static constexpr uint32_t TextureCoordinatesSizeInBytes = TextureCoordinatesSize * sizeof(TextureCoordinates::value_type);

    void init(ramses::TextureSampler* textureSampler,
              EBlendMode              blendMode,
              ramses::RenderGroup*    renderGroup,
              int32_t                 renderOrder,
              const ramses::Effect&   effect,
              ramses::Node*           parent);
    void setBlendMode(ramses::Appearance& appearance, EBlendMode blendMode);
    void flipTextureCoordinates(TextureCoordinates& textureCoordinates);
    void createGeometry(ramses::TextureSampler& textureSampler,
                        ramses::RamsesClient&   client,
                        ramses::Scene&          scene,
                        ramses::RenderGroup*    renderGroup,
                        ramses::Appearance&     appearance,
                        int32_t                 renderOrder);

    const uint32_t            m_width;
    const uint32_t            m_height;
    const bool                m_flipVertical;
    ramses::Node&             m_translateNode;
    ramses::Node&             m_scaleNode;
    ramses::Appearance*       m_appearance    = nullptr;
    ramses::VertexDataBuffer* m_textureCoords = nullptr;
};

#endif
