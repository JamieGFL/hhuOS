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
 */

#include <stdint.h>

#ifndef HHUOS_GLOBALSYSTEMINTERRUPT_H
#define HHUOS_GLOBALSYSTEMINTERRUPT_H

namespace Kernel {

/**
 * Abstracts the hardware interrupt model from the system.
 *
 * They cannot be named statically, as this depends on the system configuration.
 * GlobalSystemInterrupts map 1:1 to IO APIC interrupt inputs.
 * They do not translate 1:1 to InterruptRequests or InterruptVectors.
 */
enum GlobalSystemInterrupt : uint32_t {
};

}

#endif
