

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

auto lfb = (Util::Graphic::LinearFrameBuffer*)nullptr;

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

void run(Util::Io::KeyDecoder &keyDecoder, float elapsedTime, int16_t keyCode, const Util::Io::Key &key,
         Util::Graphic::PixelDrawer &pixelDrawer, Util::Graphic::StringDrawer &stringDrawer);

void processAdvanceInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key,
                          Util::Graphic::PixelDrawer &pixelDrawer, Util::Graphic::StringDrawer &stringDrawer);

bool processGeneralInput(const Util::Io::Key &key, Util::Graphic::PixelDrawer &pixelDrawer,
                         Util::Graphic::StringDrawer &stringDrawer);

void clearScreen(const Util::Graphic::PixelDrawer &pixelDrawer);

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

void drawRAM(int x, int y, uint16_t addr, int rows, int columns, Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
    Util::Graphic::Color textColor(255, 255, 255); // White color for text

    //clear RAM area
    for (int i = 0; i < rows * 15; ++i) {
        for (int j = 0; j < columns * 15; ++j) {
            pixelDrawer->drawPixel(x + j, y + i, clearColor);
        }
    }

    for (int row = 0; row < rows; ++row) {
        // mock data for testing instead of hex(addr, 4)
        Util::String line = Util::String("$") + hex(addr, 4) + Util::String(": ");
        for (int col = 0; col < columns; ++col) {
            // mock data for testing instead of hex(bus.bRead(&bus, addr, 1), 2)
            line += hex(nes_bus_Read(nesBus.busBase, addr, 1), 2) + Util::String(" ");
            addr++;
        }
        drawText(line, x, y + row * 10, textColor, stringDrawer); // Adjust spacing as needed
    }
}

void drawCPU(int x, int y, Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
    Util::Graphic::Color activeColor(0, 255, 0);   // Green for active flags
    Util::Graphic::Color inactiveColor(255, 0, 0); // Red for inactive flags
    Util::Graphic::Color textColor(255, 255, 255);     // White for text

    // clear CPU area
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 80; ++j) {
            pixelDrawer->drawPixel(x + i, y + j, clearColor);
        }
    }

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
    drawText(Util::String("A: $") + hex(nesBus.cpu->A, 2) + "  [" + Util::String::format("%d", nesBus.cpu->A) + "]", x, y + 30, textColor, stringDrawer);
    drawText(Util::String("X: $") + hex(nesBus.cpu->X, 2) + "  [" + Util::String::format("%d", nesBus.cpu->X) + "]", x, y + 40, textColor, stringDrawer);
    drawText(Util::String("Y: $") + hex(nesBus.cpu->Y, 2) + "  [" + Util::String::format("%d", nesBus.cpu->Y) + "]", x, y + 50, textColor, stringDrawer);
    drawText(Util::String("Stack P: $") + hex(nesBus.cpu->SP, 4), x, y + 60, textColor, stringDrawer);
}

void drawInstructions(int x, int y, int nLines, Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
    Util::Graphic::Color textColor(255, 255, 255); // White color for other instructions
    Util::Graphic::Color highlightColor(255, 255, 0); // Yellow color for the current instruction

     //clear instruction area
     for (int i = 0; i < 250; ++i) {
         for (int j = 0; j < 270; ++j) {
             pixelDrawer->drawPixel(x + i, y + j, clearColor);
         }
     }

    auto currentPC = nesBus.cpu->PC;
    int lineY = (nLines >> 1) * 10 + y;

    if(currentPC != instructions.entries[MAP_LENGTH-1].address) {
        drawText(instructions.entries[currentPC].instruction,
        x, lineY, highlightColor, stringDrawer);
        while(lineY < (nLines * 10) + y) {
            if(currentPC++ != instructions.entries[MAP_LENGTH-1].address && instructions.entries[currentPC].address  != 0 ) {
                lineY += 10;
                drawText(instructions.entries[currentPC].instruction,
                x, lineY, textColor, stringDrawer);
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
                x, lineY, textColor, stringDrawer);
            }
        }
    }
}

void drawImage(int x, int y, int width, int height, image* img, Util::Graphic::PixelDrawer* pixelDrawer, int scale = 1){

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            pixel p = getImagePixel(img, j, i);
            Util::Graphic::Color color(p.r, p.g, p.b, p.a);
            //Util::Graphic::Color color(img->data[i * width + j].r, img->data[i * width + j].g, img->data[i * width + j].b, img->data[i * width + j].a);
            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    pixelDrawer->drawPixel(x + j * scale + dx, y + i * scale + dy, color);
                }
            }
        }
    }
}

void update(Util::Graphic::StringDrawer* stringDrawer, Util::Graphic::PixelDrawer* pixelDrawer){
    // Draw
    //drawRAM( 8, 2, 0x0000, 16, 16, stringDrawer, pixelDrawer);
    //drawRAM( 8, 172, 0x8000, 16, 16, stringDrawer, pixelDrawer);
    drawCPU( 540, 2, stringDrawer, pixelDrawer);

    // x 454, y 72
    drawInstructions(540, 72, 26, stringDrawer, pixelDrawer);

    // visualize pattern table
    drawImage(540, 350, 128, 128, getPatternTableImage(nesBus.ppu, 0, palette), pixelDrawer, 1);
    drawImage(672, 350, 128, 128, getPatternTableImage(nesBus.ppu, 1, palette), pixelDrawer, 1);

    // draw screen
    drawImage(0, 20, 256, 240, getScreenImage(&nesBus), pixelDrawer, 2);

    // draw nametable ids
    for(uint8_t y = 0; y < 30; y++){
        for(uint8_t x = 0; x < 32; x++){
            drawText(hex(nesBus.ppu->nametable[0][y * 32 + x], 2), x * 16, y * 16, Util::Graphic::Colors::WHITE, stringDrawer);
        }
    }
};

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


// main loop
int main(int argc, char ** argv){
    cpu6502 cpu;
    ppu2C02 ppu;

    cInit(&cpu);
    ppuInit(&ppu);

    bInit(&nesBus, &b, &cpu, &ppu);
    setBus(&nesBus);

    cartridgeInit(&cart, "user/nes/donkey kong.nes");
    insertCartridge(&nesBus, &cart);

    //disassembly
    instructions = disassemble(0x0000, 0xFFFF, nesBus.busBase);

    // reset cpu
    reset(nesBus.cpu);

    Util::Io::File lfbFile("/device/lfb");

    lfbFile.controlFile(Util::Graphic::LinearFrameBuffer::SET_RESOLUTION, Util::Array<uint32_t>({VIDEO_WIDTH, VIDEO_HEIGHT}));

    lfb = new Util::Graphic::LinearFrameBuffer(lfbFile);

    auto pixelDrawer = Util::Graphic::PixelDrawer(*lfb);
    auto stringDrawer = Util::Graphic::StringDrawer(pixelDrawer);
    Util::Graphic::Ansi::prepareGraphicalApplication(true);
    Util::Io::File::setAccessMode(Util::Io::STANDARD_INPUT, Util::Io::File::NON_BLOCKING);
    Util::Io::KeyDecoder keyDecoder(new Util::Io::DeLayout());

    clearScreen(pixelDrawer);

    float elapsedTime = 0;
    clock_t start, end;

    update(&stringDrawer, &pixelDrawer);
    while (true) {
        elapsedTime = (float)(end - start) / CLOCKS_PER_SEC;
        start = clock();

        auto keyCode = Util::System::in.read();
        Util::Io::Key key = keyDecoder.getCurrentKey();
        if(running){
            run(keyDecoder, elapsedTime, keyCode, key, pixelDrawer, stringDrawer);
        }
        else {
            processAdvanceInputs(keyDecoder, keyCode, key, pixelDrawer, stringDrawer);
        }

        if(processGeneralInput(key, pixelDrawer, stringDrawer)){
            return true;
        }

        end = clock();
    }
    // clean up
    delete lfb;
    bDestroy(&nesBus);


    return 0;
}

void clearScreen(const Util::Graphic::PixelDrawer &pixelDrawer) {
    for (int y = 0; y < 700; ++y) {
        for (int x = 0; x < 800; ++x) {
            pixelDrawer.drawPixel(x, y, clearColor);
        }
    }
}

bool processGeneralInput(const Util::Io::Key &key, Util::Graphic::PixelDrawer &pixelDrawer,
                         Util::Graphic::StringDrawer &stringDrawer) {
    if(key.isPressed() && key.getScancode() == Util::Io::Key::ENTER){
        running = true;
    }
    if(key.isPressed() && key.getScancode() == Util::Io::Key::R){
        reset(nesBus.cpu);
        update(&stringDrawer, &pixelDrawer);
    }
    if(key.isPressed() && key.getScancode() == Util::Io::Key::Q){
        (++palette) &= 0x07;
    }
    if(key.isPressed() && key.getScancode() == Util::Io::Key::ESC){
        return true;
    }
}

void processAdvanceInputs(Util::Io::KeyDecoder &keyDecoder, int16_t keyCode, const Util::Io::Key &key,
                          Util::Graphic::PixelDrawer &pixelDrawer, Util::Graphic::StringDrawer &stringDrawer) {
    if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
        if (key.isPressed() && key.getScancode() == Util::Io::Key::C) {
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            update(&stringDrawer, &pixelDrawer);
        }
        if (key.isPressed() && key.getScancode() == Util::Io::Key::F) {
            do {
                busClock(&nesBus);
            } while (!frameComplete(&nesBus));
            do {
                busClock(&nesBus);
            } while (!complete(nesBus.cpu));
            setFrameComplete(&nesBus, false);
            update(&stringDrawer, &pixelDrawer);
        }
    }
}

void run(Util::Io::KeyDecoder &keyDecoder, float elapsedTime, int16_t keyCode, const Util::Io::Key &key,
         Util::Graphic::PixelDrawer &pixelDrawer, Util::Graphic::StringDrawer &stringDrawer) {
    if (keyCode != -1 && keyDecoder.parseScancode(keyCode)) {
        if (key.isPressed() && key.getScancode() == Util::Io::Key::SPACE) {
            running = false;
        }
    }
    if(timeLeft > 0.0f){
        timeLeft -= elapsedTime;
    } else {
        timeLeft += (1.0f / 60.0f) - elapsedTime;
        do (busClock(&nesBus)); while (!frameComplete(&nesBus));
        setFrameComplete(&nesBus, false);
        update(&stringDrawer, &pixelDrawer);
    }
}


