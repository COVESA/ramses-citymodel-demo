//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_ANIMATIONPATH_H
#define RAMSES_CITYMODEL_ANIMATIONPATH_H

#include "Vector3.h"
#include "vector"
#include "stdint.h"

/// Animation path for car and camera.
class AnimationPath
{
public:
    /// An animation key frame.
    class Key
    {
    public:
        /// Constructor.
        /** @param carPosition Position of the car.
         *  @param carRotation Rotation of the car. */
        Key(const Vector3& carPosition, const Vector3& carRotation);

        /// Returns the car position.
        /** @return The car position. */
        const Vector3& getCarPosition();

        /// Returns the car rotation.
        /** @return The car rotation. */
        const Vector3& getCarRotation();

    private:
        /// The car position.
        Vector3 m_carPosition;

        /// The car rotation.
        Vector3 m_carRotation;
    };

    /// Constructor.
    AnimationPath();

    /// Adds a key frame to the animation path.
    /** @param key The key frame.*/
    void add(const Key& key);

    /// Returns the i-th key frame.
    /** @param i Index of the frame to return.
     *  @return The key frame. */
    Key* getKey(uint32_t i);

    /// Number of key frames in the animation path.
    /** @return Number of key frames. */
    uint32_t getNumberOfKeys() const;

private:
    /// List of animation key frames.
    std::vector<Key> m_keys;
};

#endif
