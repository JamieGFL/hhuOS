/*
 * Copyright (C) 2018-2023 Heinrich-Heine-Universitaet Duesseldorf,
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
 * The game engine is based on a bachelor's thesis, written by Malte Sehmer.
 * The original source code can be found here: https://github.com/Malte2036/hhuOS
 */

#ifndef HHUOS_GAME_H
#define HHUOS_GAME_H

#include "lib/util/collection/Array.h"
#include "lib/util/collection/Collection.h"
#include "lib/util/collection/Iterator.h"
#include "Scene.h"
#include "lib/util/collection/ArrayListBlockingQueue.h"

namespace Util {
namespace Game {
class Graphics2D;
}  // namespace Game
}  // namespace Util

namespace Util::Game {

class Game {

friend class Engine;

public:
    /**
     * Default Constructor.
     */
    Game() = default;

    /**
     * Copy Constructor.
     */
    Game(const Game &other) = delete;

    /**
     * Assignment operator.
     */
    Game &operator=(const Game &other) = delete;

    /**
     * Destructor.
     */
    ~Game();

    void stop();

    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] Scene& getCurrentScene();

    void pushScene(Scene *scene);

    void switchToNextScene();

private:

    void initializeNextScene(Graphics2D &graphics);

    ArrayListBlockingQueue<Scene*> scenes;
    bool firstScene = true;
    bool sceneSwitched = true;

    bool running = true;
};

}

#endif
