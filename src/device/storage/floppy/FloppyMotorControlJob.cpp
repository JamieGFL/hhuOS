/*
 * Copyright (C) 2018 Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * Heinrich-Heine University
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

#include "FloppyMotorControlJob.h"

namespace Device::Storage {

FloppyMotorControlJob::FloppyMotorControlJob(FloppyDevice &device) : device(device) {}

void FloppyMotorControlJob::run() {
    if (device.getMotorState() != FloppyController::WAIT) {
        return;
    }

    remainingTime -= INTERVAL;
    if (remainingTime <= 0) {
        if (device.getMotorState() == FloppyController::WAIT) {
            device.getController().killMotor(device);
        }

        resetTime();
    }
}

void FloppyMotorControlJob::resetTime() {
    remainingTime = TIME;
}

}