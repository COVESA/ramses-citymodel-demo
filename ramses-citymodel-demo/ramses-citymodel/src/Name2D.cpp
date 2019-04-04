//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Name2D.h"
#include "ramses-citymodel/Math.h"
#include "ramses-citymodel/NamingManager.h"

#include "math.h"

Name2D::Name2D(const std::string&              text,
               ramses::Scene&                  scene,
               ramses::RamsesClient&           client,
               const Vector3& p0,
               const Vector3& p1)
    : Name(text, scene, client)
    , m_p0(p0)
    , m_p1(p1)
    , m_namingManager(0)
    , m_alpha(0.0)
    , m_validForRendering(false)
    , m_destinationVisible(false)
    , m_useFading(true)
{
}

void Name2D::setNamingManager(NamingManager* namingRoot)
{
    m_namingManager = namingRoot;
}

void Name2D::useFading(bool doFading)
{
    m_useFading = doFading;
}

void Name2D::update()
{
    updatePosition();
    checkCollision();
    updateAlpha();
}

void Name2D::updatePosition()
{
    Vector2 p0;
    Vector2 p1;
    bool                     xyVisible = false;
    bool                     visibleP0 = m_namingManager->projectCheckXY(m_p0, p0, xyVisible);
    bool                     visibleP1 = m_namingManager->project(m_p1, p1);

    m_validForRendering = visibleP0 && visibleP1;

    m_destinationVisible = xyVisible && m_validForRendering;

    Vector2 x = p1 - p0;

    if (std::abs(x.length()) > std::numeric_limits<float>::epsilon())
    {
        x = x.normalize();

        if (x.getX() < 0.0)
        {
            // Flip names that are wrong way round
            x = -x;
        }

        float angle = Math::Rad2Deg(atan2(x.getY(), x.getX()));

        if (m_translateNode != nullptr && m_rotateNode != nullptr && (m_alpha > 0.0 || m_destinationVisible))
        {
            m_translateNode->setTranslation(p0.getX(), p0.getY(), 0.0);
            m_rotateNode->setRotation(0.0, 0.0, -angle);
        }
    }

    computeBounding();
}

void Name2D::checkCollision()
{
    if (m_destinationVisible)
    {
        if (m_namingManager->checkCollision(this))
        {
            m_destinationVisible = false;
        }
        else
        {
            m_namingManager->addToColliders(this);
        }
    }
}

void Name2D::updateAlpha()
{
    const float alphaDelta = 1.0f / (60.0f * 1.0f);
    if (m_destinationVisible)
    {
        if (m_alpha < 1.0)
        {
            if (m_alpha == 0.0)
            {
                m_namingManager->addToVisibleNames(this);
            }
            if (m_useFading)
            {
                m_alpha += alphaDelta;
                if (m_alpha > 1.0)
                {
                    m_alpha = 1.0;
                }
            }
            else
            {
                m_alpha = 1.0;
            }
        }
    }
    else
    {
        if (m_alpha > 0.0)
        {
            if (m_useFading)
            {
                m_alpha -= alphaDelta;
                if (m_alpha <= 0.0)
                {
                    m_alpha = 0.0;
                    m_namingManager->addToInvisibleNames(this);
                }
            }
            else
            {
                m_alpha = 0.0;
                m_namingManager->addToInvisibleNames(this);
            }
        }
    }
    setAlpha(m_alpha);
}

void Name2D::computeBounding()
{
    if (m_visibilityNode != nullptr)
    {
        // convert mMin/MaxBounding from object space of mMesh to common world space

        Matrix44 toWorldSpace = Name2D::GetWorldSpaceMatrixOfNode(*m_visibilityNode);

        m_boundingPoints[0] = toWorldSpace * Vector4(m_minBounding.getX(), m_minBounding.getY(), 0.0, 1.0);
        m_boundingPoints[1] = toWorldSpace * Vector4(m_minBounding.getX(), m_maxBounding.getY(), 0.0, 1.0);
        m_boundingPoints[2] = toWorldSpace * Vector4(m_maxBounding.getX(), m_minBounding.getY(), 0.0, 1.0);
        m_boundingPoints[3] = toWorldSpace * Vector4(m_maxBounding.getX(), m_maxBounding.getY(), 0.0, 1.0);
    }
}

bool Name2D::checkCollision(Name2D* name)
{
    return checkCollisionThisToOther(name) && name->checkCollisionThisToOther(this);
}

bool Name2D::checkCollisionThisToOther(Name2D* name)
{
    if (m_visibilityNode != nullptr)
    {
        // convert bounding points of other to object space of this, allows comparison to mMin/MaxBounding of this (are
        // in object space of this)
        Matrix44 toObjectSpace = GetObjectSpaceMatrixOfNode(*m_visibilityNode);

        Vector4 p[4];
        for (uint32_t i = 0; i < 4; i++)
        {
            p[i] = toObjectSpace * name->boundingPoint(i);
        }

        // Check left:
        if (p[0].getX() < m_minBounding.getX() && p[1].getX() < m_minBounding.getX() && p[2].getX() < m_minBounding.getX() &&
            p[3].getX() < m_minBounding.getX())
        {
            return false;
        }

        // Check right:
        if (p[0].getX() > m_maxBounding.getX() && p[1].getX() > m_maxBounding.getX() && p[2].getX() > m_maxBounding.getX() &&
            p[3].getX() > m_maxBounding.getX())
        {
            return false;
        }

        // Check bottom:
        if (p[0].getY() < m_minBounding.getY() && p[1].getY() < m_minBounding.getY() && p[2].getY() < m_minBounding.getY() &&
            p[3].getY() < m_minBounding.getY())
        {
            return false;
        }

        // Check top:
        if (p[0].getY() > m_maxBounding.getY() && p[1].getY() > m_maxBounding.getY() && p[2].getY() > m_maxBounding.getY() &&
            p[3].getY() > m_maxBounding.getY())
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

Matrix44 Name2D::GetWorldSpaceMatrixOfNode(ramses::Node& node)
{
    float m[16];
    node.getModelMatrix(m);
    return Matrix44(m[0], m[4], m[8],  m[12],
                    m[1], m[5], m[9],  m[13],
                    m[2], m[6], m[10], m[14],
                    m[3], m[7], m[11], m[15]);
}

Matrix44 Name2D::GetObjectSpaceMatrixOfNode(ramses::Node& node)
{
    float m[16];
    node.getInverseModelMatrix(m);
    return Matrix44(m[0], m[4], m[8],  m[12],
                    m[1], m[5], m[9],  m[13],
                    m[2], m[6], m[10], m[14],
                    m[3], m[7], m[11], m[15]);
}

const Vector4& Name2D::boundingPoint(uint32_t i) const
{
    return m_boundingPoints[i];
}
