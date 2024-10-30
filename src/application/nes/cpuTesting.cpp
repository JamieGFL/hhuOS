#include "cpu6502.h"
#include "bus_interface.h"
#include "bus_nes.h"

class cpuTesting
{
public:
    cpuTesting();
    ~cpuTesting();

    bus bus;
    Instruction_Map instructions;
    auto stringDrawer;
    auto pixelDrawer;

    std::string hex(uint32_t intValue, uint8_t length){
        std::string hexstring(length, '0');
        for(int i = length - 1; i >= 0; i--, intValue >>= 4){
            hexstring[i] = "0123456789ABCDEF"[intValue & 0xF];
        }
        return hexstring;
    }

    void drawRAM(int x, int y, uint16_t addr, int rows, int columns){
        Util::Graphic::Color textColor(255, 255, 255); // White color for text

        for (int row = 0; row < rows; ++row) {
            std::string line = "$" + hex(addr, 4) + ": ";
            for (int col = 0; col < columns; ++col) {
                line += hex(bus.bRead(&bus, addr, 1), 2) + " ";
                addr++;
            }
            drawText(line, x, y + row * 10, textColor); // Adjust spacing as needed
        }
    }

    void drawCPU(int x, int y){
        Util::Graphic::Color activeColor(0, 255, 0);   // Green for active flags
        Util::Graphic::Color inactiveColor(255, 0, 0); // Red for inactive flags
        Util::Graphic::Color textColor(255, 255, 255);     // White for text

        drawText("STATUS:", x, y, textColor);

        // Draw each flag
        const std::array<std::pair<std::string, uint8_t>, 8> flags = {{
                                                                              {"N", bus.cpu.status & N_FLAG},
                                                                              {"V", bus.cpu.status & V_FLAG},
                                                                              {"U", bus.cpu.status & U_FLAG},
                                                                              {"B", bus.cpu.status & B_FLAG},
                                                                              {"D", bus.cpu.status & D_FLAG},
                                                                              {"I", bus.cpu.status & I_FLAG},
                                                                              {"Z", bus.cpu.status & Z_FLAG},
                                                                              {"C", bus.cpu.status & C_FLAG}
                                                                      }};
        int flagX = x + 64;
        for (const auto& [label, state] : flags) {
            drawText(label, flagX, y, state ? activeColor : inactiveColor);
            flagX += 16; // Adjust spacing as necessary
        }

        // Draw register values
        drawText("PC: $" + hex(bus.cpu.PC, 4), x, y + 20, textColor);
        drawText("A: $" + hex(bus.cpu.A, 2) + "  [" + std::to_string(bus.cpu.A) + "]", x, y + 30, textColor);
        drawText("X: $" + hex(bus.cpu.X, 2) + "  [" + std::to_string(bus.cpu.X) + "]", x, y + 40, textColor);
        drawText("Y: $" + hex(bus.cpu.Y, 2) + "  [" + std::to_string(bus.cpu.Y) + "]", x, y + 50, textColor);
        drawText("Stack P: $" + hex(bus.cpu.SP, 4), x, y + 60, textColor);
    }

    void drawInstructions(int x, int y, int nLines){
        Util::Graphic::Color textColor(255, 255, 255); // White color for other instructions
        Util::Graphic::Color highlightColor(255, 255, 0); // Yellow color for the current instruction

        // Find the index of the current instruction based on the program counter (pc)
        size_t currentIndex = 0;
        for (; currentIndex < instructions.count; ++currentIndex) {
            if (instructions.entries[currentIndex].address == bus.cpu.pc) {
                break; // Found the current instruction
            }
        }

        // If the current instruction is found
        if (currentIndex < instructions.count) {
            int startIndex = std::max(0, static_cast<int>(currentIndex) - nLines / 2); // Calculate starting index
            int endIndex = std::min(static_cast<int>(instructions.count), static_cast<int>(currentIndex) + nLines / 2 + 1); // Calculate ending index

            // Draw instructions from startIndex to endIndex
            for (int i = startIndex; i < endIndex; ++i) {
                // Check if the current index matches the program counter
                Util::Graphic::Color currentColor = (i == currentIndex) ? highlightColor : textColor;

                drawText(
                        "$" + hex(instructions.entries[i].address, 4) + ": " + instructions.entries[i].instruction,
                        x,
                        y + (i - startIndex) * 10,
                        currentColor
                );
            }
        }
    }

    void drawText(const std::string& text, uint32_t x, uint32_t y, Util::Graphic::Color color){
        stringDrawer.drawString(Util::Graphic::Fonts::ACORN_8x8, x, y, text.c_str(), color, Util::Graphic::Colors::INVISIBLE);
    }

    void update(){
        // Colors
        Util::Graphic::Color darkGreenColor(0, 100, 0, 255); // RGB(0, 100, 0) with full opacity

        // Fill the entire screen with dark green
        for (int y = 0; y < windowHeight; ++y) {
            for (int x = 0; x < windowWidth; ++x) {
                pixelDrawer->drawPixel(x, y, darkGreenColor);
            }
        }

        // Draw
        drawRAM( 2, 2, 0x0000, 16, 16);
        drawRAM( 2, 172, 0x8000, 16, 16);
        drawCPU( 448, 2);
        drawInstructions(448, 72, 26);


    };

    bool processInput(){
        Util::Io::KeyDecoder keyDecoder(new Util::Io::DeLayout());

        while (true) {
            auto keyCode = Util::System::in.read();

            if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
                Util::Io::Key key = keyDecoder.getCurrentKey();
                uint8_t scancode = key.getScancode();

                switch (scancode) {
                    // Space for cpu.clock() loop until complete
                    case Util::Io::Key::SPACE:
                        do { cpu.clock(); } while (!cpu.complete());
                        break;

                        // 'R' key for cpu.reset()
                    case Util::Io::Key::R:
                        cpu.reset();
                        break;

                        // 'I' key for cpu.irq()
                    case Util::Io::Key::I:
                        cpu.irq();
                        break;

                        // 'N' key for cpu.nmi()
                    case Util::Io::Key::N:
                        cpu.nmi();
                        break;

                        // Exit if Esc is pressed
                    case Util::Io::Key::ESC:
                        return true;
                    default:
                        break;
                }
            }
        }
    };

    // main loop
    int main(){
        bInit(&bus);
        cInit(&bus.cpu);
        cpu.connectBus(&bus.cpu);


        std::stringstream ss;
        ss << "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";
        uint16_t nOffset = 0x8000;
        while(!ss.eof()){
            std::string b;
            ss >> b;
            bus.ram[nOffset++] = (uint8_t)std::stoul(b, nullptr, 16);
        }

        // Reset Vector, CPU will start execution from this address > 0x8000
        bus.ram[0xFFFC] = 0x00;
        bus.ram[0xFFFD] = 0x80;

        // disassembly
        instructions = disassemble(0x0000, 0xFFFF, &bus);


        Util::Io::File lfbFile("/device/lfb");

        lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({windowWidth, windowHeight}));

        lfb = new Util::Graphic::LinearFrameBuffer(lfbFile);

        pixelDrawer = Util::Graphic::PixelDrawer(lfb);
        stringDrawer = Util::Graphic::StringDrawer(pixelDrawer);
        Util::Graphic::Ansi::prepareGraphicalApplication(false);
        lfb.clear();

        while(true){
            update();

            if(processInput()){
                break;
            }
        }

        reset();

        return 0;
    }
};




