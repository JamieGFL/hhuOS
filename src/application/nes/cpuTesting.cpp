

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
#include "bus_nes.h"

bus_nes nesBus{};
    Instruction_Map instructions{};
    const unsigned int VIDEO_HEIGHT = 32;
    const unsigned int VIDEO_WIDTH = 64;

    static Util::String hex(uint32_t intValue, uint8_t length){
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

        // Draw each flag
        Util::Array<Util::Pair<Util::String, bool>> flags = {
                Util::Pair<Util::String, bool>("N", nesBus.cpu->status & N_FLAG),
                Util::Pair<Util::String, bool>("V", nesBus.cpu->status & V_FLAG),
                Util::Pair<Util::String, bool>("U", nesBus.cpu->status & U_FLAG),
                Util::Pair<Util::String, bool>("B", nesBus.cpu->status & B_FLAG),
                Util::Pair<Util::String, bool>("D", nesBus.cpu->status & D_FLAG),
                Util::Pair<Util::String, bool>("I", nesBus.cpu->status & I_FLAG),
                Util::Pair<Util::String, bool>("Z", nesBus.cpu->status & Z_FLAG),
                Util::Pair<Util::String, bool>("C", nesBus.cpu->status & C_FLAG)
        };

        int flagX = x + 64;
        for (const auto& [label, state] : flags) {
            drawText(label, flagX, y, state ? activeColor : inactiveColor, stringDrawer);
            flagX += 16;
        }

        // Draw register values
        drawText(Util::String("PC: $") + hex(nesBus.cpu->PC, 4), x, y + 20, textColor, stringDrawer);
        drawText(Util::String("A: $") + hex(nesBus.cpu->A, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 30, textColor, stringDrawer);
        drawText(Util::String("X: $") + hex(nesBus.cpu->X, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 40, textColor, stringDrawer);
        drawText(Util::String("Y: $") + hex(nesBus.cpu->Y, 2) + "  [" + Util::String::format("%d", 0x00) + "]", x, y + 50, textColor, stringDrawer);
        drawText(Util::String("Stack P: $") + hex(nesBus.cpu->SP, 4), x, y + 60, textColor, stringDrawer);
    }

    void drawInstructions(int x, int y, int nLines, Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
        Util::Graphic::Color textColor(255, 255, 255); // White color for other instructions
        Util::Graphic::Color highlightColor(255, 255, 0); // Yellow color for the current instruction

        Util::Graphic::Color darkGreenColor(0, 60, 0, 255);
        // clear instruction area
        for (int i = 0; i < nLines + 1000; ++i) {
            for (int j = 0; j < 1000; ++j) {
                pixelDrawer->drawPixel(x + j, y + i * 10, darkGreenColor);
            }
        }

        // Find the index of the current instruction based on the program counter (pc)
        int currentIndex = 0;
        for (; currentIndex < instructions.count; ++currentIndex) {
            if (instructions.entries[currentIndex].address == nesBus.cpu->PC) {
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


    void update(Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
        // Draw
        drawRAM( 8, 2, 0x0000, 16, 16, stringDrawer);
        drawRAM( 8, 172, 0x8000, 16, 16, stringDrawer);
        drawCPU( 454, 2, stringDrawer);

        Util::Graphic::Color darkGreenColor(0, 60, 0, 255);
        for (int y = 70; y < 700; ++y) {
            for (int x = 450; x < 800; ++x) {
                pixelDrawer->drawPixel(x, y, darkGreenColor);
            }
        }
        drawInstructions(454, 72, 10, stringDrawer, pixelDrawer);
    };


    void load_program(const Util::String &program, uint16_t start_address) {
        uint16_t nOffset = start_address;

        // Split the program string by spaces
        Util::Array<Util::String> tokens = program.split(" ");

        for (uint32_t i = 0; i < tokens.length(); i++) {
            // Convert each token from hex string to uint8_t and store in RAM
            nesBus.busBase->ram[nOffset++] = (uint8_t)Util::String::parseHexInt(tokens[i]);
        }
    }

    // main loop
    int main(){
        bus *b = new bus();
        bInit(&nesBus, b);
        cInit(nesBus.cpu);
        connectBus(nesBus.cpu, b);


        Util::String program = "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";
        uint16_t nOffset = 0x8000;  // Starting address
        load_program(program, nOffset);

        // Reset Vector, CPU will start execution from this address > 0x8000
        nesBus.busBase->ram[0xFFFC] = 0x00;
        nesBus.busBase->ram[0xFFFD] = 0x80;

        // disassembly
        instructions = disassemble(0x0000, 0xFFFF, nesBus.busBase);


        Util::Io::File lfbFile("/device/lfb");

        lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({VIDEO_WIDTH, VIDEO_HEIGHT}));

        auto* lfb = new Util::Graphic::LinearFrameBuffer(lfbFile);

        auto pixelDrawer = Util::Graphic::PixelDrawer(*lfb);
        auto stringDrawer = Util::Graphic::StringDrawer(pixelDrawer);
        Util::Graphic::Ansi::prepareGraphicalApplication(true);
        Util::Io::File::setAccessMode(Util::Io::STANDARD_INPUT, Util::Io::File::NON_BLOCKING);
        Util::Io::KeyDecoder keyDecoder(new Util::Io::DeLayout());

        // Colors
        Util::Graphic::Color darkGreenColor(0, 60, 0, 255); // RGB(0, 100, 0) with full opacity

        // Fill the entire screen with dark green
        for (int y = 0; y < 700; ++y) {
            for (int x = 0; x < 800; ++x) {
                pixelDrawer.drawPixel(x, y, darkGreenColor);
            }
        }

        update(&stringDrawer, &pixelDrawer);
        while (true) {
            auto keyCode = Util::System::in.read();

            if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
                Util::Io::Key key = keyDecoder.getCurrentKey();

                if(key.isPressed())
                {
                    switch (key.getScancode()) {
                        // Space for cpu.clock() loop until complete
                        case Util::Io::Key::SPACE:
                            advanceClock(nesBus.cpu);
                            update(&stringDrawer, &pixelDrawer);
                            break;

                            // 'R' key for cpu.reset()
                        case Util::Io::Key::R:
                            reset(nesBus.cpu);
                            update(&stringDrawer, &pixelDrawer);
                            break;

                            // 'I' key for cpu.irq()
                        case Util::Io::Key::Q:
                            irq(nesBus.cpu);
                            update(&stringDrawer, &pixelDrawer);
                            break;

                            // 'N' key for cpu.nmi()
                        case Util::Io::Key::W:
                            nmi(nesBus.cpu);
                            update(&stringDrawer, &pixelDrawer);
                            break;

                            // Exit if Esc is pressed
                        case Util::Io::Key::ESC:
                            return true;
                        default:
                            break;
                    }
                }
            }
        }

        return 0;
    }





