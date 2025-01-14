#include "lib/util/io/key/KeyDecoder.h"
#include "lib/util/io/key/layout/DeLayout.h"
#include "lib/util/graphic/Colors.h"
#include "lib/util/graphic/BufferedLinearFrameBuffer.h"
#include "lib/util/graphic/PixelDrawer.h"
#include "lib/util/graphic/StringDrawer.h"
#include "lib/util/graphic/Font.h"
#include "lib/util/graphic/font/Acorn8x8.h"
#include "lib/util/base/String.h"
#include "lib/util/collection/Pair.h"
#include "lib/util/io/file/File.h"
#include "lib/util/graphic/Ansi.h"
#include "nesBus.h"
#include "lib/util/io/stream/FileInputStream.h"
#include "lib/util/time/Timestamp.h"
#include <cstdio>

// Graphics
auto lfb = (Util::Graphic::LinearFrameBuffer*)nullptr;
auto bLFB = (Util::Graphic::BufferedLinearFrameBuffer*)nullptr;
auto pixelDrawer = (Util::Graphic::PixelDrawer*)nullptr;
auto stringDrawer = (Util::Graphic::StringDrawer*)nullptr;

// Timing
static uint64_t lastFrameTime;
static uint64_t lastClockCycleTime;

// NES components
static bus_nes nesBus;
static bus b;
static cartridge cart;

// NES state
bool running = false;
uint8_t palette = 0x00;

// Timer
float timeLeft = 0.0f;

// Rendering Scale
int scale = 1;

// FPS
Util::Time::Timestamp fpsTimer;
uint32_t fpsCounter = 0;
uint32_t fps = 0;

// Disassembly
Instruction_Map instructions{};

// Draw State
bool drawCPUData = false;
bool drawOAMData = false;
bool drawRAMData = false;
bool drawInstructionsData = false;
bool drawPatternTables = false;
bool drawScreen = true;
bool drawTiming = false;
uint16_t addr = 0x0000;

// Screen
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

// Palette
auto paletteScreen = new Util::Graphic::Color[64]{
        {84, 84, 84, 255}, {0, 30, 116, 255}, {8, 16, 144, 255}, {48, 0, 136, 255},
        {68, 0, 100, 255}, {92, 0, 48, 255}, {84, 4, 0, 255}, {60, 24, 0, 255},
        {32, 42, 0, 255}, {8, 58, 0, 255}, {0, 64, 0, 255}, {0, 60, 0, 255},
        {0, 50, 60, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255},
        {152, 150, 152, 255}, {8, 76, 196, 255}, {48, 50, 236, 255}, {92, 30, 228, 255},
        {136, 20, 176, 255}, {160, 20, 100, 255}, {152, 34, 32, 255}, {120, 60, 0, 255},
        {84, 90, 0, 255}, {40, 114, 0, 255}, {8, 124, 0, 255}, {0, 118, 40, 255},
        {0, 102, 120, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255},
        {236, 238, 236, 255}, {76, 154, 236, 255}, {120, 124, 236, 255}, {176, 98, 236, 255},
        {228, 84, 236, 255}, {236, 88, 180, 255}, {236, 106, 100, 255}, {212, 136, 32, 255}, {160, 170, 0, 255},
        {116, 196, 0, 255}, {76, 208, 32, 255}, {56, 204, 108, 255}, {56, 180, 204, 255}, {60, 60, 60, 255},
        {0, 0, 0, 255}, {0, 0, 0, 255}, {236, 238, 236, 255}, {168, 204, 236, 255}, {188, 188, 236, 255},
        {212, 178, 236, 255}, {236, 174, 236, 255}, {236, 174, 212, 255}, {236, 180, 176, 255}, {228, 196, 144, 255},
        {204, 210, 120, 255}, {180, 222, 120, 255}, {168, 226, 144, 255}, {152, 226, 180, 255}, {160, 214, 228, 255},
        {160, 162, 160, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}
};

void run(float elapsedTime);
void processAdvanceInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key);
void processUtilityInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key);
void drawOAM();

// converts integer to hex string
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

// draws text on the screen
void drawText(const Util::String& text, uint32_t x, uint32_t y, Util::Graphic::Color color){
    stringDrawer->drawString(Util::Graphic::Fonts::ACORN_8x8, x, y, static_cast<const char*>(text), color, Util::Graphic::Colors::INVISIBLE);
}

// draws RAM on the screen
void drawRAM(int x, int y, int rows, int columns, uint16_t newAddr){
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    for (int row = 0; row < rows; ++row) {
        Util::String line = Util::String("$") + hex(newAddr, 4) + Util::String(": ");
        for (int col = 0; col < columns; ++col) {
            line += hex(nesBusRead(nesBus.busBase, newAddr, 1), 2) + Util::String(" ");
            newAddr++;
        }
        drawText(line, x, y + row * 10, textColor);
    }
}

// draws CPU on the screen
void drawCPU(int x, int y){
    Util::Graphic::Color activeColor(0, 255, 0);   // Green for active flags
    Util::Graphic::Color inactiveColor(255, 0, 0); // Red for inactive flags
    Util::Graphic::Color textColor(255, 255, 255); // White for text

    drawText(Util::String("STATUS:"), x, y, textColor);

    // flags
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

    // draw flags
    int flagX = x + 64;
    for (const auto& [label, state] : flags) {
        drawText(label, flagX, y, state ? activeColor : inactiveColor);
        flagX += 16;
    }

    // Draw register values
    drawText(Util::String("PC: $") + hex(nesBus.cpu->PC, 4), x, y + 20, textColor);
    drawText(Util::String("A: $") + hex(nesBus.cpu->A, 2) + "  [" + Util::String::format("%d", nesBus.cpu->A) + "]", x, y + 30, textColor);
    drawText(Util::String("X: $") + hex(nesBus.cpu->X, 2) + "  [" + Util::String::format("%d", nesBus.cpu->X) + "]", x, y + 40, textColor);
    drawText(Util::String("Y: $") + hex(nesBus.cpu->Y, 2) + "  [" + Util::String::format("%d", nesBus.cpu->Y) + "]", x, y + 50, textColor);
    drawText(Util::String("Stack P: $") + hex(nesBus.cpu->SP, 4), x, y + 60, textColor);
}

void drawInstructions(int x, int y, int nLines){
    Util::Graphic::Color textColor(255, 255, 255); // White color for other instructions
    Util::Graphic::Color highlightColor(255, 255, 0); // Yellow color for the current instruction

    auto currentPC = nesBus.cpu->PC;
    int lineY = (nLines >> 1) * 10 + y;

    // draw instructions before current PC, draw current instruction
    if(currentPC != instructions.entries[MAP_LENGTH-1].address) {
        drawText(instructions.entries[currentPC].instruction,
                 x, lineY, highlightColor);
        while(lineY < (nLines * 10) + y) {
            if(currentPC++ != instructions.entries[MAP_LENGTH-1].address && instructions.entries[currentPC].address  != 0 ) {
                lineY += 10;
                drawText(instructions.entries[currentPC].instruction,
                         x, lineY, textColor);
            }
        }
    }

    // draw instructions after current PC
    currentPC = nesBus.cpu->PC;
    lineY = (nLines >> 1) * 10 + y;
    if(currentPC != instructions.entries[MAP_LENGTH-1].address)
    {
        while(lineY > y) {
            if(currentPC-- != instructions.entries[MAP_LENGTH-1].address && instructions.entries[currentPC].address  != 0 ) {
                lineY -= 10;
                drawText(instructions.entries[currentPC].instruction,
                         x, lineY, textColor);
            }
        }
    }
}

// draw Timing data on the right side of the screen
void drawTimingData() {
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    drawText(Util::String("Draw Time: ") + Util::String::format("%d", lastFrameTime) + Util::String(" ms"), 540, 2, textColor);
    drawText(Util::String("NES Clock Cycle Time: ") + Util::String::format("%d", lastClockCycleTime) + Util::String(" ms"), 540, 12, textColor);
    // both combined
    drawText(Util::String("Total Time: ") + Util::String::format("%d", lastFrameTime + lastClockCycleTime) + Util::String(" ms"), 540, 22, textColor);
}

void drawFrame(int x, int y, int width, int height, image* img){
    uint64_t start = Util::Time::getSystemTime().toMilliseconds();
    if(scale == 1) {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                pixelDrawer->drawPixel(x + j, y + i, paletteScreen[nesBus.ppu->screen->data[i * width + j]]);
            }
        }
    }
    // with scale
    else {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                for (int k = 0; k < scale; ++k) {
                    for (int l = 0; l < scale; ++l) {
                        pixelDrawer->drawPixel(x + j * scale + k, y + i * scale + l,
                                               paletteScreen[nesBus.ppu->screen->data[i * width + j]]);
                    }
                }
            }
        }
    }
    if(drawTiming){
        drawTimingData();
    }

    bLFB->flush();

    lastFrameTime = Util::Time::getSystemTime().toMilliseconds() - start;
}

void drawImage(int x, int y, int width, int height, image* img){
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t p = getImagePixel(img, j, i);
            pixelDrawer->drawPixel(x + j, y + i , paletteScreen[p]);
        }
    }
}

void drawOAM() {
    Util::Graphic::Color textColor(255, 255, 255);
    for (int i = 0; i < 26; ++i) {
        Util::String line = hex(i, 2) + Util::String(": (")
                            + Util::String::format("%d", nesBus.ppu->oamPointer[i * 4 + 3]) + Util::String(", ")
                            + Util::String::format("%d", nesBus.ppu->oamPointer[i * 4 + 0]) + Util::String(") ")
                            + Util::String("ID: ") + hex(nesBus.ppu->oamPointer[i * 4 + 1], 2)
                            + Util::String(" AT: ") + hex(nesBus.ppu->oamPointer[i * 4 + 2], 2);
        drawText(line, 540, 72 + i * 10, textColor);
    }
}

void drawUtilityFunctionsStatus(int x, int y) {
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    drawText(Util::String("F1: Toggle Screen"), x, y + 22, textColor);
    drawText(Util::String("F2: Toggle OAM Data"), x, y + 32, textColor);
    drawText(Util::String("F3: Toggle RAM Data"), x, y + 42, textColor);
    drawText(Util::String("F4: Toggle Instructions"), x, y + 52, textColor);
    drawText(Util::String("F5: Toggle CPU Data"), x, y + 62, textColor);
    drawText(Util::String("F6: Toggle Pattern Tables"), x, y + 72, textColor);
    drawText(Util::String("F7: Toggle Timing"), x, y + 82, textColor);
    drawText(Util::String("R: Reset NES"), x, y + 92, textColor);

    drawText(Util::String("SPACE: Pause/Resume"), x + 220, y + 22, textColor);
    drawText(Util::String("ENTER: Run"), x + 220, y + 32, textColor);
    drawText(Util::String("C: Step"), x + 220, y + 42, textColor);
    drawText(Util::String("F: Frame"), x + 220, y + 52, textColor);
    drawText(Util::String("ESC: Exit"), x + 220, y + 62, textColor);

    drawText(Util::String("Scroll through RAM: F8/F9"), x + 440, y + 22, textColor);
    drawText(Util::String("+: Scale Up"), x + 440, y + 32, textColor);
    drawText(Util::String("-: Scale Down"), x + 440, y + 42, textColor);
    drawText(Util::String("Q: Change Palette"), x + 440, y + 52, textColor);

}

void update() {
    bLFB->clear();

    drawUtilityFunctionsStatus(20, 510);

    if(drawCPUData){
        drawCPU(540, 2);
    }
    if(drawOAMData){
        drawOAM();
    }
    if(drawRAMData){
        // 300, 72 for right side of screen
        drawRAM(10, 265, 26, 16, addr);
    }
    if(drawInstructionsData) {
        drawInstructions(540, 72, 26);
    }
    if(drawPatternTables){
        drawImage(530, 350, 128, 128, getPatternTableImage(nesBus.ppu, 0, palette));
        drawImage(662, 350, 128, 128, getPatternTableImage(nesBus.ppu, 1, palette));
    }

    // draw screen
    if(drawScreen){
        drawText(Util::String("FPS: ") + Util::String::format("%d", fps), 0, 0, Util::Graphic::Colors::WHITE);
        drawFrame(0, 20, 256, 240, getScreenImage(&nesBus));
    }

}

// how to use
// commandline: nes <rom>
// Example: nes nestest.nes

// main loop
int main(int argc, char ** argv){
    // Initialize NES
    cpu6502 cpu;
    ppu2C02 ppu;

    cInit(&cpu);
    ppuInit(&ppu);

    bInit(&nesBus, &b, &cpu, &ppu);
    setBus(&nesBus);

    // Build gamefile string and initialize cartridge
    char path[256];
    if(argc == 1){
        snprintf(path, sizeof(path), "user/nes/nestest.nes");
    }
    else
    {
    const char* basePath = "user/nes/";
    snprintf(path, sizeof(path), "%s%s", basePath, argv[1]);
    }
    cartridgeInit(&cart, path);
    insertCartridge(&nesBus, &cart);

    //disassembly
    instructions = disassemble(0x0000, 0xFFFF, nesBus.busBase);

    // reset bus
    busReset(&nesBus);

    // Preparing Rendering
    Util::Io::File lfbFile("/device/lfb");

    lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({VIDEO_WIDTH, VIDEO_HEIGHT}));

    lfb = new Util::Graphic::LinearFrameBuffer(lfbFile);
    bLFB = new Util::Graphic::BufferedLinearFrameBuffer(*lfb, true);

    pixelDrawer = new Util::Graphic::PixelDrawer(*bLFB);
    stringDrawer = new Util::Graphic::StringDrawer(*pixelDrawer);
    Util::Graphic::Ansi::prepareGraphicalApplication(true);
    Util::Io::File::setAccessMode(Util::Io::STANDARD_INPUT, Util::Io::File::NON_BLOCKING);
    Util::Io::KeyDecoder keyDecoder(new Util::Io::DeLayout());

    float elapsedTime = 0;

    Util::Time::Timestamp start = Util::Time::getSystemTime();
    update();
    while (true) {
        auto currentTime = Util::Time::getSystemTime();
        elapsedTime = (float)(currentTime - start).toMilliseconds();

        auto keyCode = Util::System::in.read();
        Util::Io::Key key = keyDecoder.getCurrentKey();

        // NES input processing
        nesBus.controller[0] = 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::A) ? 0x80 : 0x00; // A
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::E) ? 0x40 : 0x00; // B
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::X) ? 0x20 : 0x00; // select
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::S) ? 0x10 : 0x00; // start
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::UP) ? 0x08 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::DOWN) ? 0x04 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::LEFT) ? 0x02 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::RIGHT) ? 0x01 : 0x00;

        if(running){
            if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
                // stop NES
                if (key.isPressed() && key.getScancode() == Util::Io::Key::SPACE) {
                    running = false;
                }
            }
            // run NES
            run(elapsedTime);
        }
        else {
            // process inputs to step through NES in steps
            processAdvanceInputs(keyDecoder, keyCode, key);
        }
        // process utility inputs
        processUtilityInputs(keyDecoder, keyCode, key);

        update();

        // exit
        if(key.isPressed() && key.getScancode() == Util::Io::Key::ESC){
            break;
        }
        // draw
        bLFB->flush();

        // FPS
        fpsCounter++;
        auto frameTime = Util::Time::getSystemTime() - currentTime;
        fpsTimer += frameTime;

        if (fpsTimer >= Util::Time::Timestamp::ofSeconds(1)) {
            fps = fpsCounter;
            fpsCounter = 0;
            fpsTimer.reset();
        }
    }
    // clean up
    delete lfb;
    delete bLFB;
    //bDestroy(&nesBus);


    return 0;
}

void processUtilityInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key) {
    if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
        if (key.isPressed() && key.getScancode() == Util::Io::Key::ENTER) {
            running = true;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::R) {
            busReset(&nesBus);
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::Q) {
            (++palette) &= 0x07;
        }
        // scale
        if (key.isPressed() && key.getScancode() == Util::Io::Key::PLUS) {
            scale++;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::MINUS) {
            if (scale > 1) { scale--; }
        }

        if (key.isPressed() && key.getScancode() == Util::Io::Key::F1) {
            drawScreen = !drawScreen;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F2) {
            drawOAMData = !drawOAMData;
            addr ++;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F3) {
            drawRAMData = !drawRAMData;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F8) {
                addr -= 16;
            }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F9) {
                addr += 16;
            }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F4) {
            drawInstructionsData = !drawInstructionsData;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F5) {
            drawCPUData = !drawCPUData;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F6) {
            drawPatternTables = !drawPatternTables;
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F7) {
            drawTiming = !drawTiming;
        }
    }

}

void processAdvanceInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key) {
    if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
        if (key.isPressed() && key.getScancode() == Util::Io::Key::C) {
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F) {
            do {
                busClock(&nesBus);
            } while (!frameComplete(&nesBus));
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            setFrameComplete(&nesBus, false);
        }
    }
}

void run(float elapsedTime) {
    if (timeLeft > 0.0f) {
        timeLeft -= elapsedTime;
    } else {
        timeLeft += (1.0f / 60.0f) - elapsedTime;
        uint64_t start = Util::Time::getSystemTime().toMilliseconds();
        do (busClock(&nesBus)); while (!frameComplete(&nesBus));
        lastClockCycleTime = Util::Time::getSystemTime().toMilliseconds() - start;
        setFrameComplete(&nesBus, false);
    }
}
