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

#include <assert.h>

#define PORTABLEGL_IMPLEMENTATION
#define PGL_ASSERT(x) assert(x)
#include "lib/portablegl/portablegl.h"

#include "lib/util/base/System.h"
#include "lib/util/base/Address.h"
#include "lib/util/graphic/Ansi.h"
#include "lib/util/base/ArgumentParser.h"
#include "lib/util/io/file/File.h"
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/graphic/BufferedLinearFrameBuffer.h"

extern void info();
extern void triangle(const Util::Graphic::BufferedLinearFrameBuffer &lfb);
extern void gears(const Util::Graphic::BufferedLinearFrameBuffer &lfb);

static glContext context{};

int32_t main(int32_t argc, char *argv[]) {
    auto argumentParser = Util::ArgumentParser();
    argumentParser.setHelpText("PortableGL demo application.\n\n"
                               "Usage: portablegl <demo>\n"
                               "Demos: info, triangle\n"
                               "Options:\n"
                               "  -r, --resolution: Set display resolution\n"
                               "  -h, --help: Show this help message");

    argumentParser.addArgument("resolution", false, "r");

    if (!argumentParser.parse(argc, argv)) {
        Util::System::error << argumentParser.getErrorString() << Util::Io::PrintStream::endl << Util::Io::PrintStream::flush;
        return -1;
    }

    auto arguments = argumentParser.getUnnamedArguments();
    if (arguments.length() == 0) {
        Util::System::error << "opengl: No arguments provided! Please specify a demo (info, triangle)." << Util::Io::PrintStream::endl << Util::Io::PrintStream::flush;
        return -1;
    }

    const auto &demo = arguments[0];
    if (demo == "info") {
        info();
        return 0;
    }

    Util::Graphic::Ansi::prepareGraphicalApplication(true);
    auto lfbFile = Util::Io::File("/device/lfb");

    if (argumentParser.hasArgument("resolution")) {
        auto split1 = argumentParser.getArgument("resolution").split("x");
        auto split2 = split1[1].split("@");

        uint32_t resolutionX = Util::String::parseInt(split1[0]);
        uint32_t resolutionY = Util::String::parseInt(split2[0]);
        uint32_t colorDepth = split2.length() > 1 ? Util::String::parseInt(split2[1]) : 32;

        lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({resolutionX, resolutionY, colorDepth}));
    }

    auto lfb = Util::Graphic::LinearFrameBuffer(lfbFile);

    // PortableGL expects line pitch to be exactly the same as the resolution width in bytes.
    // However, the LFB might have a different pitch, so we need to create a buffered LFB with the correct pitch.
    // The buffered LFB will take the different pitch into account when copying the buffer to the LFB.
    uint16_t pitch = lfb.getResolutionX() * ((lfb.getColorDepth() == 15 ? 16 : lfb.getColorDepth()) / 8);
    auto bufferedLfb = Util::Graphic::BufferedLinearFrameBuffer(lfb, pitch);

    // Initialize PortableGL context
    auto *screenBuffer = reinterpret_cast<uint32_t*>(bufferedLfb.getBuffer().get());
    auto success = init_glContext(&context, &screenBuffer, bufferedLfb.getResolutionX(), bufferedLfb.getResolutionY(), bufferedLfb.getColorDepth(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!success) {
        Util::System::error << "portablegl: Failed to initialize GL context!" << Util::Io::PrintStream::endl << Util::Io::PrintStream::flush;
        exit(-1);
    }

    if (demo == "triangle") {
        triangle(bufferedLfb);
    } else if (demo == "gears") {
        gears(bufferedLfb);
    } else {
        Util::System::error << "opengl: Invalid demo '" << demo << "'!" << Util::Io::PrintStream::endl << Util::Io::PrintStream::flush;
        return -1;
    }

    free_glContext(&context);
    Util::Graphic::Ansi::cleanupGraphicalApplication();
    return 0;
}