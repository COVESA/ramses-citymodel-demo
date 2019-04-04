//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/TilePager.h"
#include "ramses-citymodel/Tile.h"

TilePager::TilePager()
    : m_thread(TilePager::Run, this)
{
}

TilePager::~TilePager()
{
    terminate();
}

void TilePager::terminate()
{
    if (!m_cancelRequested)
    {
        m_cancelRequested = true;
        m_nonEmptyCondition.notify_one();
        m_thread.join();
    }
}

void TilePager::add(std::vector<Tile*> tiles)
{
    if (!tiles.empty())
    {
        m_mutex.lock();
        bool wasEmpty = m_queue.empty();
        for (uint32_t i = 0; i < tiles.size(); i++)
        {
            m_queue.push_front(tiles[i]);
        }
        if (wasEmpty)
        {
            m_nonEmptyCondition.notify_one();
        }
        m_mutex.unlock();
    }
}

void TilePager::remove(std::vector<Tile*> tiles)
{
    if (!tiles.empty())
    {
        m_mutex.lock();
        for (uint32_t i = 0; i < tiles.size(); i++)
        {
            remove(tiles[i]);
        }
        m_mutex.unlock();
    }
}

void TilePager::remove(Tile* tile)
{
    for (uint32_t i = 0; i < m_queue.size(); i++)
    {
        if (m_queue[i] == tile)
        {
            m_queue.erase(m_queue.begin() + i);
            return;
        }
    }
}

void TilePager::get(std::vector<Tile*>& tiles)
{
    m_mutex.lock();
    tiles = m_readTiles;
    m_readTiles.clear();
    m_mutex.unlock();
}

uint32_t TilePager::getNumTilesToLoad()
{
    m_mutex.lock();
    unsigned int numTiles = static_cast<unsigned int>(m_queue.size());
    m_mutex.unlock();
    return numTiles;
}

void TilePager::Run(TilePager* tilePager)
{
    tilePager->run();
}

void TilePager::run()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    while (!m_cancelRequested)
    {
        m_nonEmptyCondition.wait(lock, [this]{return !this->m_queue.empty() || this->m_cancelRequested;});
        if (!m_queue.empty())
        {
            Tile* tile = m_queue.back();
            m_queue.pop_back();

            lock.unlock();
            tile->doReadNode();
            lock.lock();
            m_readTiles.push_back(tile);
        }
    }
}
