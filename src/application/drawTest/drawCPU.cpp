#include <time.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "lib/util/io/key/KeyDecoder.h"
#include "lib/util/io/key/Key.h"
#include "lib/util/io/key/layout/DeLayout.h"
#include "lib/util/graphic/Colors.h"
#include "lib/util/graphic/BufferedLinearFrameBuffer.h"
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/graphic/PixelDrawer.h"
#include "lib/util/graphic/StringDrawer.h"
#include "lib/util/graphic/Font.h"
#include "lib/util/graphic/font/Acorn8x8.h"
#include "lib/util/graphic/Colors.h"
#include "lib/util/base/String.h"
#include "lib/util/collection/Pair.h"
#include "lib/util/collection/Array.h"
#include "lib/util/io/file/File.h"
#include "lib/util/graphic/Ansi.h"

#define MAX_INSTRUCTION_LENGTH 64
#define MAP_LENGTH 256

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

typedef struct {
    uint16_t address;
    char instruction[MAX_INSTRUCTION_LENGTH];
} mapEntry;

typedef struct {
    mapEntry entries[MAP_LENGTH];
    size_t count;
    size_t capacity;
} Instruction_Map;

int pc;
Instruction_Map instructions;


Util::String hex(uint32_t intValue, uint8_t length){
    Util::String hexstring("00000000");
    if (length < hexstring.length()) {
        hexstring = hexstring.substring(0, length);
    }
    for(int i = length - 1; i >= 0; i--){
        hexstring[i] = "0123456789ABCDEF"[intValue & 0xF];
        intValue >>= 4;
    }

    return hexstring;
}

void drawText(const Util::String& text, uint32_t x, uint32_t y, Util::Graphic::Color color, Util::Graphic::StringDrawer* stringDrawer){
    stringDrawer->drawString(Util::Graphic::Fonts::ACORN_8x8, x, y, static_cast<const char*>(text), color, Util::Graphic::Colors::INVISIBLE);
}

void drawRAM(int x, int y, uint16_t addr, int rows, int columns, Util::Graphic::StringDrawer* stringDrawer){
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    for (int row = 0; row < rows; ++row) {
        // mock data for testing instead of hex(addr, 4)
        Util::String line = Util::String("$") + hex(addr, 4) + Util::String(": ");
        for (int col = 0; col < columns; ++col) {
            // mock data for testing instead of hex(bus.bRead(&bus, addr, 1), 2)
            line += hex(0x00, 2) + Util::String(" ");
            addr++;
        }
        drawText(line, x, y + row * 10, textColor, stringDrawer); // Adjust spacing as needed
    }
}

void drawCPU(int x, int y, Util::Graphic::StringDrawer* stringDrawer){
    Util::Graphic::Color activeColor(0, 255, 0);   // Green for active flags
    Util::Graphic::Color inactiveColor(255, 0, 0); // Red for inactive flags
    Util::Graphic::Color textColor(255, 255, 255);     // White for text

    drawText(Util::String("STATUS:"), x, y, textColor, stringDrawer);

    // mock drawing each flag without the bus and enums
    Util::Array<Util::Pair<Util::String, bool>> flags = {
            Util::Pair<Util::String, bool>("N", true),
            Util::Pair<Util::String, bool>("V", false),
            Util::Pair<Util::String, bool>("U", true),
            Util::Pair<Util::String, bool>("B", false),
            Util::Pair<Util::String, bool>("D", true),
            Util::Pair<Util::String, bool>("I", false),
            Util::Pair<Util::String, bool>("Z", true),
            Util::Pair<Util::String, bool>("C", false)
    };

    int flagX = x + 64;

    for (uint32_t i = 0; i < flags.length(); ++i) {
        Util::String flagLabel = flags[i].first;
        bool state = flags[i].second;
        drawText(flagLabel, flagX, y, state ? activeColor : inactiveColor, stringDrawer);
        flagX += 16; // Adjust spacing as necessary
    }

    // Draw register values
    drawText(Util::String("PC: $") + hex(0x0000, 4), x, y + 20, textColor, stringDrawer);
    drawText(Util::String("A: $") + hex(0x00, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 30, textColor, stringDrawer);
    drawText(Util::String("X: $") + hex(0x00, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 40, textColor, stringDrawer);
    drawText(Util::String("Y: $") + hex(0x00, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 50, textColor, stringDrawer);
    drawText(Util::String("Stack P: $") + hex(0x0000, 4), x, y + 60, textColor, stringDrawer);
}

void drawInstructions(int x, int y, int nLines, Util::Graphic::StringDrawer* stringDrawer){
    Util::Graphic::Color textColor(255, 255, 255); // White color for other instructions
    Util::Graphic::Color highlightColor(255, 255, 0); // Yellow color for the current instruction

    // Find the index of the current instruction based on the program counter (pc)
    size_t currentIndex = 0;
    for (; currentIndex < instructions.count; ++currentIndex) {
        if (instructions.entries[currentIndex].address == pc) {
            break; // Found the current instruction
        }
    }

    // If the current instruction is found
    if (currentIndex < instructions.count) {
        int startIndex = (currentIndex >= nLines / 2) ? (currentIndex - nLines / 2) : 0; // Calculate starting index
        int endIndex = (currentIndex + nLines / 2 + 1 < instructions.count)
                       ? (currentIndex + nLines / 2 + 1)
                       : instructions.count; // Calculate ending index

        // Draw instructions from startIndex to endIndex
        for (int i = startIndex; i < endIndex; ++i) {
            // Check if the current index matches the program counter
            Util::Graphic::Color currentColor = (i == currentIndex) ? highlightColor : textColor;

            drawText(
                    Util::String("$") + hex(instructions.entries[i].address, 4) + ": " + instructions.entries[i].instruction,
                    x,
                    y + (i - startIndex) * 10,
                    currentColor,
                    stringDrawer
            );
        }
    }
}


void update(Util::Graphic::StringDrawer* stringDrawer){
    // Draw
    drawRAM( 8, 2, 0x0000, 16, 16, stringDrawer);
    drawRAM( 8, 172, 0x8000, 16, 16, stringDrawer);
    drawCPU( 454, 2, stringDrawer);
    drawInstructions(454, 72, 26, stringDrawer);
};

int main(){
    Util::Io::File lfbFile("/device/lfb");

    // window width and height
    int windowWidth = 640;
    int windowHeight = 320;
    lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({VIDEO_WIDTH, VIDEO_HEIGHT}));

    auto* lfb = new Util::Graphic::LinearFrameBuffer(lfbFile);

    auto pixelDrawer = Util::Graphic::PixelDrawer(*lfb);
    auto stringDrawer = Util::Graphic::StringDrawer(pixelDrawer);
    Util::Graphic::Ansi::prepareGraphicalApplication(true);
    lfb->clear();

    Util::Io::File::setAccessMode(Util::Io::STANDARD_INPUT, Util::Io::File::NON_BLOCKING);
    auto keyDecoder = Util::Io::KeyDecoder(new Util::Io::DeLayout());

    // adding fake instructions for testing
    mapEntry entry;
    entry.address = 0x0000;
    strcpy(entry.instruction, "LDA #$00");
    instructions.entries[0] = entry;
    mapEntry entry1;
    entry1.address = 0x0001;
    strcpy(entry1.instruction, "LDX #$00");
    instructions.entries[1] = entry1;
    mapEntry entry2;
    entry2.address = 0x0002;
    strcpy(entry2.instruction, "LDY #$00");
    instructions.entries[2] = entry2;
    mapEntry entry3;
    entry3.address = 0x0003;
    strcpy(entry3.instruction, "STA $0000");
    instructions.entries[3] = entry3;
    mapEntry entry4;
    entry4.address = 0x0004;
    strcpy(entry4.instruction, "STX $0000");
    instructions.entries[4] = entry4;
    mapEntry entry5;
    entry5.address = 0x0005;
    strcpy(entry5.instruction, "STY $0000");
    instructions.entries[5] = entry5;
    mapEntry entry6;
    entry6.address = 0x0006;
    strcpy(entry6.instruction, "LDA $0000");
    instructions.entries[6] = entry6;


    instructions.count = 7;
    instructions.capacity = 7;

    // Colors
    Util::Graphic::Color darkGreenColor(0, 60, 0, 255); // RGB(0, 100, 0) with full opacity

    // Fill the entire screen with dark green
    for (int y = 0; y < 700; ++y) {
        for (int x = 0; x < 800; ++x) {
            pixelDrawer.drawPixel(x, y, darkGreenColor);
        }
    }
    pc = 0x0000;

    update(&stringDrawer);
    while(true){
        auto keyCode = Util::System::in.read();

        if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
            Util::Io::Key key = keyDecoder.getCurrentKey();

            switch (key.getScancode()) {
                case Util::Io::Key::SPACE:
                    pc += 0x0001;
                    update(&stringDrawer);
                    break;
                case Util::Io::Key::R:
                    pc -= 0x0001;
                    update(&stringDrawer);
                    break;

                    // Exit if Esc is pressed
                case Util::Io::Key::ESC:
                    return true;
                default:
                    break;
            }
        }
    }

    return 0;
}