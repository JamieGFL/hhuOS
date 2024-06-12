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

#include "WaveFile.h"

namespace Util {
namespace Io {
class File;
}  // namespace Io
}  // namespace Util

namespace Util::Sound {

WaveFile::WaveFile(const Io::File &file) : Io::FilterInputStream(stream), stream(file) {
    read(reinterpret_cast<uint8_t*>(&riffChunk), 0, sizeof(RiffChunk));
    read(reinterpret_cast<uint8_t*>(&formatChunk), 0, sizeof(formatChunk));

    auto readBytes = read(reinterpret_cast<uint8_t*>(&dataChunk), 0, sizeof(dataChunk));
    while (dataChunk.dataSignature[0] != 'd' && dataChunk.dataSignature[1] != 'a' && dataChunk.dataSignature[2] != 't' && dataChunk.dataSignature[3] != 'a') {
        if (readBytes <= 0) {
            Util::Exception::throwException(Util::Exception::INVALID_ARGUMENT, "WaveFile: No 'data' chunk found!");
        }

        skip(dataChunk.chunkSize);
        readBytes = read(reinterpret_cast<uint8_t*>(&dataChunk), 0, sizeof(dataChunk));
    }
}

uint32_t WaveFile::getDataSize() const {
    return dataChunk.chunkSize;
}

WaveFile::AudioFormat WaveFile::getAudioFormat() const {
    return formatChunk.audioFormat;
}

uint16_t WaveFile::getNumChannels() const {
    return formatChunk.numChannels;
}

uint32_t WaveFile::getSamplesPerSecond() const {
    return formatChunk.samplesPerSecond;
}

uint32_t WaveFile::getBytesPerSecond() const {
    return formatChunk.bytesPerSecond;
}

uint16_t WaveFile::getBitsPerSample() const {
    return formatChunk.bitsPerSample;
}

uint16_t WaveFile::getFrameSize() const {
    return formatChunk.frameSize;
}

uint32_t WaveFile::getSampleCount() const {
    return dataChunk.chunkSize / formatChunk.frameSize;
}

}