/*
 * Copyright (C) 2018-2024 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 * Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * The dino game is based on a bachelor's thesis, written by Malte Sehmer.
 * The original source code can be found here: https://github.com/Malte2036/hhuOS
 */

#ifndef HHUOS_BLOCK_H
#define HHUOS_BLOCK_H

#include <stdint.h>

#include "lib/util/game/2d/Entity.h"
#include "lib/util/game/2d/Sprite.h"
#include "lib/util/base/String.h"

namespace Util {
namespace Game {
class Graphics;
namespace D2 {
class CollisionEvent;
class TranslationEvent;
}  // namespace D2
}  // namespace Game
namespace Math {
class Vector2D;
}  // namespace Math
}  // namespace Util

class Block : public Util::Game::D2::Entity {

public:

    enum Tag : uint32_t {
        BOX = 1,
        GRASS = 2,
        DIRT = 3,
        WATER = 4
    };

    /**
     * Constructor.
     */
    Block(Tag tag, const Util::Math::Vector2D &position, uint32_t countX, uint32_t countY);

    /**
     * Copy Constructor.
     */
    Block(const Block &other) = delete;

    /**
     * Assignment operator.
     */
    Block &operator=(const Block &other) = delete;

    /**
     * Destructor.
     */
    ~Block() override = default;

    void initialize() override;

    void onUpdate(double delta) override;

    void draw(Util::Game::Graphics &graphics) override;

    void onTranslationEvent(Util::Game::D2::TranslationEvent &event) override;

    void onCollisionEvent(Util::Game::D2::CollisionEvent &event) override;

    static const constexpr double SIZE = 0.08;

private:

    static Util::String getSpritePath(Tag tag);

    uint32_t countX;
    uint32_t countY;
    Util::Game::D2::Sprite sprite;
};

#endif