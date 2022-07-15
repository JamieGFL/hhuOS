/*
 * Copyright (C) 2018-2022 Heinrich-Heine-Universitaet Duesseldorf,
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
 */

#include <cstdint>
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/graphic/Colors.h"
#include "lib/util/graphic/PixelDrawer.h"
#include "lib/util/async/Thread.h"
#include "lib/util/math/Random.h"

enum Direction {
    UP = 0, RIGHT = 90, DOWN = 180, LEFT = 270
};

struct Ant {
    int32_t limitX, limitY;
    int32_t x, y;
    Direction direction = UP;
    Util::Graphic::Color color = Util::Graphic::Colors::WHITE;
    Util::Math::Random random = Util::Math::Random();

    Ant(int32_t limitX, int32_t limitY) : limitX(limitX), limitY(limitY), x(static_cast<int32_t>(Util::Math::Random().nextRandomNumber() * limitX)), y(static_cast<int32_t>(Util::Math::Random().nextRandomNumber() * limitY)) {}

    void move() {
        switch (direction) {
            case UP:
                y--;
                break;
            case RIGHT:
                x++;
                break;
            case DOWN:
                y++;
                break;
            case LEFT:
                x--;
                break;
        }

        bool crossedBorder = false;

        if (x < 0) {
            x = limitX - 1;
            crossedBorder = true;
        }

        if (y < 0) {
            y = limitY - 1;
            crossedBorder = true;
        }

        if (x >= limitX) {
            x = 0;
            crossedBorder = true;
        }

        if (y >= limitY) {
            y = 0;
            crossedBorder = true;
        }

        if (crossedBorder) {
            color = {static_cast<uint8_t>(random.nextRandomNumber() * 256), static_cast<uint8_t>(random.nextRandomNumber() * 256), static_cast<uint8_t>(random.nextRandomNumber() * 256)};
        }
    }

    void turnClockWise() {
        direction = static_cast<Direction>((direction + 90) % 360);
    }

    void turnCounterClockWise() {
        direction = direction == 0 ? LEFT : static_cast<Direction>(direction - 90);
    }
};

int32_t main(int32_t argc, char *argv[]) {
    auto sleepInterval = Util::Memory::String::parseInt(argv[1]);
    Util::Graphic::LinearFrameBuffer lfb(Util::File::File("/device/lfb"));
    Util::Graphic::PixelDrawer drawer(lfb);
    Ant ant(lfb.getResolutionX(), lfb.getResolutionY());
    lfb.clear();

    for (int i = 0;; i++) {
        auto pixel = lfb.readPixel(ant.x, ant.y);
        if (pixel == Util::Graphic::Colors::BLACK) {
            drawer.drawPixel(ant.x, ant.y, ant.color);
            ant.turnClockWise();
        } else {
            drawer.drawPixel(ant.x, ant.y, Util::Graphic::Colors::BLACK);
            ant.turnCounterClockWise();
        }

        ant.move();
        if (sleepInterval != 0 && i % sleepInterval == 0) {
            Util::Async::Thread::sleep({0, 10000000});
        }
    }
}