

#include <cstring>
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
#include "lib/util/io/stream/FileInputStream.h"
#include <time.h>
#include <cstdio>

auto lfb = (Util::Graphic::LinearFrameBuffer*)nullptr;
auto bLFB = (Util::Graphic::BufferedLinearFrameBuffer*)nullptr;
auto pixelDrawer = (Util::Graphic::PixelDrawer*)nullptr;
auto stringDrawer = (Util::Graphic::StringDrawer*)nullptr;

// nes
static  bus_nes nesBus;
static bus b;
static cartridge cart;
bool running = false;
float timeLeft = 0.0f;

uint8_t palette = 0x00;

Instruction_Map instructions{};
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

Util::Graphic::Color clearColor(0, 60, 0, 255); // dark green

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

void processUtilityInputs(const Util::Io::Key &key);

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

void drawText(const Util::String& text, uint32_t x, uint32_t y, Util::Graphic::Color color){
    stringDrawer->drawString(Util::Graphic::Fonts::ACORN_8x8, x, y, static_cast<const char*>(text), color, Util::Graphic::Colors::INVISIBLE);
}

void drawRAM(int x, int y, uint16_t addr, int rows, int columns){
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    for (int row = 0; row < rows; ++row) {
        // mock data for testing instead of hex(addr, 4)
        Util::String line = Util::String("$") + hex(addr, 4) + Util::String(": ");
        for (int col = 0; col < columns; ++col) {
            // mock data for testing instead of hex(bus.bRead(&bus, addr, 1), 2)
            line += hex(nes_bus_Read(nesBus.busBase, addr, 1), 2) + Util::String(" ");
            addr++;
        }
        drawText(line, x, y + row * 10, textColor); // Adjust spacing as needed
    }
}

void drawCPU(int x, int y){
    Util::Graphic::Color activeColor(0, 255, 0);   // Green for active flags
    Util::Graphic::Color inactiveColor(255, 0, 0); // Red for inactive flags
    Util::Graphic::Color textColor(255, 255, 255);     // White for text

    drawText(Util::String("STATUS:"), x, y, textColor);

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

extern "C" void drawFrame(uint8_t* frame){
    for(int i = 0; i < 256; i++){
        for(int j = 0; j < 240; j++){
            uint8_t p = frame[i * 240 + j];
            pixelDrawer->drawPixel( j * 2 , i * 2, paletteScreen[p]);
        }
    }
}

void drawImage(int x, int y, int width, int height, image* img, int scale = 1){

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t p = getImagePixel(img, j, i);
            //Util::Graphic::Color color(img->data[i * width + j].r, img->data[i * width + j].g, img->data[i * width + j].b, img->data[i * width + j].a);
                    pixelDrawer->drawPixel(x + j, y + i , paletteScreen[p]);

        }
    }
}

void drawFrame(int x, int y, int width, int height, image* img, int scale = 1){

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            //int8_t p = getImagePixel(img, j, i);
            //Util::Graphic::Color color(img->data[i * width + j].r, img->data[i * width + j].g, img->data[i * width + j].b, img->data[i * width + j].a);
            pixelDrawer->drawPixel(x + j, y + i , paletteScreen[nesBus.ppu->screen->data[i * width + j]]);
        }
    }
}

void update(){
    bLFB->clear();

    // Fill the entire screen with dark green
//    for (int y = 0; y < 700; ++y) {
//        for (int x = 0; x < 800; ++x) {
//            pixelDrawer->drawPixel(x, y, clearColor);
//        }
//    }
    // Draw
    //drawRAM( 8, 2, 0x0000, 16, 16, stringDrawer, pixelDrawer);
    //drawRAM( 8, 172, 0x8000, 16, 16, stringDrawer, pixelDrawer);
    drawCPU( 540, 2);

    // x 454, y 72
    drawInstructions(540, 72, 26);

    // visualize pattern table
    drawImage(540, 350, 128, 128, getPatternTableImage(nesBus.ppu, 0, palette), 1);
    drawImage(672, 350, 128, 128, getPatternTableImage(nesBus.ppu, 1, palette), 1);

    // draw screen
    drawFrame(0, 20, 256, 240, getScreenImage(&nesBus), 2);


    // draw nametable ids
//    for (int i = 0; i < 500; ++i) {
//        for (int j = 0; j < 520; ++j) {
//            pixelDrawer->drawPixel( j, i, clearColor);
//        }
//    }
//
//    for(uint8_t y = 0; y < 30; y++){
//        for(uint8_t x = 0; x < 32; x++){
//            drawText(hex((uint32_t)nesBus.ppu->nametable[0][y * 32 + x], 2), x * 16, y * 16, Util::Graphic::Colors::WHITE, stringDrawer);
//        }
    }
//};

// load with string of hex values
uint32_t load_program(const Util::String &program, uint16_t nOffset) {

    // Split the program string by spaces
    Util::Array<Util::String> tokens = program.split(" ");

    for (uint32_t i = 0; i < tokens.length(); i++) {
        // Convert each token from hex string to uint8_t and store in RAM
        nesBus.busBase->ram[nOffset++] = (uint8_t)Util::String::parseHexInt(tokens[i]);
    }
    return tokens.length();
}
// ppuIn->paletteScreen[0x00] = createPixel(84, 84, 84, 255);
//    ppuIn->paletteScreen[0x01] = createPixel(0, 30, 116, 255);
//    ppuIn->paletteScreen[0x02] = createPixel(8, 16, 144, 255);
//    ppuIn->paletteScreen[0x03] = createPixel(48, 0, 136, 255);
//    ppuIn->paletteScreen[0x04] = createPixel(68, 0, 100, 255);
//    ppuIn->paletteScreen[0x05] = createPixel(92, 0, 48, 255);
//    ppuIn->paletteScreen[0x06] = createPixel(84, 4, 0, 255);
//    ppuIn->paletteScreen[0x07] = createPixel(60, 24, 0, 255);
//    ppuIn->paletteScreen[0x08] = createPixel(32, 42, 0, 255);
//    ppuIn->paletteScreen[0x09] = createPixel(8, 58, 0, 255);
//    ppuIn->paletteScreen[0x0A] = createPixel(0, 64, 0, 255);
//    ppuIn->paletteScreen[0x0B] = createPixel(0, 60, 0, 255);
//    ppuIn->paletteScreen[0x0C] = createPixel(0, 50, 60, 255);
//    ppuIn->paletteScreen[0x0D] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x0E] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x0F] = createPixel(0, 0, 0, 255);
//
//    ppuIn->paletteScreen[0x10] = createPixel(152, 150, 152, 255);
//    ppuIn->paletteScreen[0x11] = createPixel(8, 76, 196, 255);
//    ppuIn->paletteScreen[0x12] = createPixel(48, 50, 236, 255);
//    ppuIn->paletteScreen[0x13] = createPixel(92, 30, 228, 255);
//    ppuIn->paletteScreen[0x14] = createPixel(136, 20, 176, 255);
//    ppuIn->paletteScreen[0x15] = createPixel(160, 20, 100, 255);
//    ppuIn->paletteScreen[0x16] = createPixel(152, 34, 32, 255);
//    ppuIn->paletteScreen[0x17] = createPixel(120, 60, 0, 255);
//    ppuIn->paletteScreen[0x18] = createPixel(84, 90, 0, 255);
//    ppuIn->paletteScreen[0x19] = createPixel(40, 114, 0, 255);
//    ppuIn->paletteScreen[0x1A] = createPixel(8, 124, 0, 255);
//    ppuIn->paletteScreen[0x1B] = createPixel(0, 118, 40, 255);
//    ppuIn->paletteScreen[0x1C] = createPixel(0, 102, 120, 255);
//    ppuIn->paletteScreen[0x1D] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x1E] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x1F] = createPixel(0, 0, 0, 255);
//
//    ppuIn->paletteScreen[0x20] = createPixel(236, 238, 236, 255);
//    ppuIn->paletteScreen[0x21] = createPixel(76, 154, 236, 255);
//    ppuIn->paletteScreen[0x22] = createPixel(120, 124, 236, 255);
//    ppuIn->paletteScreen[0x23] = createPixel(176, 98, 236, 255);
//    ppuIn->paletteScreen[0x24] = createPixel(228, 84, 236, 255);
//    ppuIn->paletteScreen[0x25] = createPixel(236, 88, 180, 255);
//    ppuIn->paletteScreen[0x26] = createPixel(236, 106, 100, 255);
//    ppuIn->paletteScreen[0x27] = createPixel(212, 136, 32, 255);
//    ppuIn->paletteScreen[0x28] = createPixel(160, 170, 0, 255);
//    ppuIn->paletteScreen[0x29] = createPixel(116, 196, 0, 255);
//    ppuIn->paletteScreen[0x2A] = createPixel(76, 208, 32, 255);
//    ppuIn->paletteScreen[0x2B] = createPixel(56, 204, 108, 255);
//    ppuIn->paletteScreen[0x2C] = createPixel(56, 180, 204, 255);
//    ppuIn->paletteScreen[0x2D] = createPixel(60, 60, 60, 255);
//    ppuIn->paletteScreen[0x2E] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x2F] = createPixel(0, 0, 0, 255);
//
//    ppuIn->paletteScreen[0x30] = createPixel(236, 238, 236, 255);
//    ppuIn->paletteScreen[0x31] = createPixel(168, 204, 236, 255);
//    ppuIn->paletteScreen[0x32] = createPixel(188, 188, 236, 255);
//    ppuIn->paletteScreen[0x33] = createPixel(212, 178, 236, 255);
//    ppuIn->paletteScreen[0x34] = createPixel(236, 174, 236, 255);
//    ppuIn->paletteScreen[0x35] = createPixel(236, 174, 212, 255);
//    ppuIn->paletteScreen[0x36] = createPixel(236, 180, 176, 255);
//    ppuIn->paletteScreen[0x37] = createPixel(228, 196, 144, 255);
//    ppuIn->paletteScreen[0x38] = createPixel(204, 210, 120, 255);
//    ppuIn->paletteScreen[0x39] = createPixel(180, 222, 120, 255);
//    ppuIn->paletteScreen[0x3A] = createPixel(168, 226, 144, 255);
//    ppuIn->paletteScreen[0x3B] = createPixel(152, 226, 180, 255);
//    ppuIn->paletteScreen[0x3C] = createPixel(160, 214, 228, 255);
//    ppuIn->paletteScreen[0x3D] = createPixel(160, 162, 160, 255);
//    ppuIn->paletteScreen[0x3E] = createPixel(0, 0, 0, 255);
//    ppuIn->paletteScreen[0x3F] = createPixel(0, 0, 0, 255);
//void initNES(){
//    Util::Graphic::Color paletteScreen[64];
//    // Create a palette with color values from comment
//    Util::Array<Util::Graphic::Color> paletteColors = {
//            {84, 84, 84, 255}, {0, 30, 116, 255}, {8, 16, 144, 255}, {48, 0, 136, 255},
//            {68, 0, 100, 255}, {92, 0, 48, 255}, {84, 4, 0, 255}, {60, 24, 0, 255},
//            {32, 42, 0, 255}, {8, 58, 0, 255}, {0, 64, 0, 255}, {0, 60, 0, 255},
//            {0, 50, 60, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255},
//            {152, 150, 152, 255}, {8, 76, 196, 255}, {48, 50, 236, 255}, {92, 30, 228, 255},
//            {136, 20, 176, 255}, {160, 20, 100, 255}, {152, 34, 32, 255}, {120, 60, 0, 255},
//            {84, 90, 0, 255}, {40, 114, 0, 255}, {8, 124, 0, 255}, {0, 118, 40, 255},
//            {0, 102, 120, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}, {0, 0, 0, 255},
//            {236, 238, 236, 255}, {76, 154, 236, 255}, {120, 124, 236, 255}, {176, 98, 236, 255},
//            {228, 84, 236, 255}, {236, 88, 180, 255}, {236, 106, 100, 255}, {212, 136, 32, 255},
//            {160, 170, 0, 255}, {116, 196, 0, 255}, {76, 208, 32, 255}, {56, 204, 108, 255},
//            {56, 180, 204, 255}, {60, 60, 60, 255}, {0, 0, 0, 255}, {0, 0, 0, 255},
//            {236, 238, 236, 255}, {168, 204, 236, 255}, {188, 188, 236, 255}, {212, 178, 236, 255},
//            {236, 174, 236, 255}, {236, 174, 212, 255}, {236, 180, 176, 255}, {228, 196, 144, 255},
//            {204, 210, 120, 255}, {180, 222, 120, 255}, {168, 226, 144, 255}, {152, 226, 180, 255},
//            {160, 214, 228, 255}, {160, 162, 160, 255}, {0, 0, 0, 255}, {0, 0, 0, 255}  };
//}


// how to use
// commandline: nes <rom>
// Beispiel: nes nestest.nes


// main loop
int main(int argc, char ** argv){
    cpu6502 cpu;
    ppu2C02 ppu;

    cInit(&cpu);
    ppuInit(&ppu);

    bInit(&nesBus, &b, &cpu, &ppu);
    setBus(&nesBus);

    //initNES();

    // build string
    char path[256]; // Allocate enough space for the full path
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

    // reset cpu
    reset(nesBus.cpu);

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
    clock_t start, end;

    update();
    while (true) {
        auto keyCode = Util::System::in.read();
        elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
        start = clock();


        Util::Io::Key key = keyDecoder.getCurrentKey();

        nesBus.controller[0] = 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::X) ? 0x80 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::E) ? 0x40 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::A) ? 0x20 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::S) ? 0x10 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::UP) ? 0x08 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::DOWN) ? 0x04 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::LEFT) ? 0x02 : 0x00;
        nesBus.controller[0] |= (key.isPressed() && key.getScancode() == Util::Io::Key::RIGHT) ? 0x01 : 0x00;

        if(running){
            if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
                if (key.isPressed() && key.getScancode() == Util::Io::Key::SPACE) {
                    running = false;
                }
            }
            run(elapsedTime);
        }
        else {
            processAdvanceInputs(keyDecoder, keyCode, key);
        }
        processUtilityInputs(key);

        if(key.isPressed() && key.getScancode() == Util::Io::Key::ESC){
            break;
        }
        bLFB->flush();
        end = clock();
    }
    // clean up
    delete lfb;
    delete bLFB;
    //bDestroy(&nesBus);


    return 0;
}

void processUtilityInputs(const Util::Io::Key &key) {
    if(key.isPressed() && key.getScancode() == Util::Io::Key::ENTER){
        running = true;
    }
    if(key.isPressed() && key.getScancode() == Util::Io::Key::R){
        reset(nesBus.cpu);
        update();
    }
    if(key.isPressed() && key.getScancode() == Util::Io::Key::Q){
        (++palette) &= 0x07;
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
            update();
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F) {
            do {
                busClock(&nesBus);
            } while (!frameComplete(&nesBus));
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            setFrameComplete(&nesBus, false);
            update();
        }
    }
}

void run(float elapsedTime) {
    if(timeLeft > 0.0f){
        timeLeft -= elapsedTime;
    } else {
        timeLeft += (1.0f / 60.0f) - elapsedTime;
        do (busClock(&nesBus)); while (!frameComplete(&nesBus));
        setFrameComplete(&nesBus, false);
        update();
    }
}


