#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include "board.hpp"
#include "common.hpp"
#include "x11_handling.hpp"

namespace HackMatch {
namespace X11Handling {
namespace {
const int PIXEL_FUZZ = 3;  // If your bot has trouble seeing the blocks, increase the fuzz.  Maximum practical value is around 22.
const char *EXAPUNKS_WINDOW_NAME = "EXAPUNKS";
const auto KEY_DELAY = std::chrono::milliseconds(17);
const int ASSUMED_WINDOW_WIDTH = 1600;
const int ASSUMED_WINDOW_HEIGHT = 900;
const int ASSUMED_XIMAGE_BYTE_ORDER = LSBFirst;
const int ASSUMED_XIMAGE_BITMAP_UNIT = 32;
const int ASSUMED_XIMAGE_BITMAP_BIT_ORDER = LSBFirst;
const int ASSUMED_XIMAGE_BITMAP_PAD = 32;
const int ASSUMED_XIMAGE_DEPTH = 24;
const int ASSUMED_XIMAGE_BYTES_PER_LINE_FULL_WINDOW = 6400;
const int ASSUMED_XIMAGE_BITS_PER_PIXEL = 32;
const int ASSUMED_XIMAGE_RED_MASK = 16711680;
const int ASSUMED_XIMAGE_GREEN_MASK = 65280;
const int ASSUMED_XIMAGE_BLUE_MASK = 255;
const int ASSUMED_XIMAGE_PIXEL_PAD = 0;
const int ITEM_SIZE = 60;
const int BOARD_PIXEL_WIDTH = Board::MAX_COLS * ITEM_SIZE;
const int BOARD_PIXEL_HEIGHT_ITEMS = 526; // eyeballed, ~10 pixels safety from red haze. should be smaller than Board::MAX_ROWS * ITEM_SIZE;
static_assert(BOARD_PIXEL_HEIGHT_ITEMS < Board::MAX_ROWS * ITEM_SIZE);
const int BOARD_PIXEL_HEIGHT = 643;
static_assert(BOARD_PIXEL_HEIGHT_ITEMS < BOARD_PIXEL_HEIGHT);
const int BOARD_X_OFFSET = 367;
const int BOARD_Y_OFFSET = 126;
const int BYTES_PER_PIXEL = ASSUMED_XIMAGE_BITS_PER_PIXEL/8;
const int PHAGE_HELD_Y_OFFSET = 630;
const int PHAGE_PINK_DATA_X_OFFSET = 392 - BOARD_X_OFFSET;
const int PHAGE_PINK_DATA_Y_OFFSET = 741 - BOARD_Y_OFFSET;
const int PHAGE_SILVER_DATA_X_OFFSET = 385 - BOARD_X_OFFSET;
const int PHAGE_SILVER_DATA_Y_OFFSET = 694 - BOARD_Y_OFFSET;

constexpr uint32_t rgbToPixel(int r, int g, int b) {
    return (r << 16) + (g << 8) + b;
}
// 21 right, 11 up from first partial pixel of chip pin
const int PIXEL_X_OFFSET = 21;
const uint32_t YELLOW_PIXEL = rgbToPixel(235, 163, 24);
const uint32_t GREEN_PIXEL = rgbToPixel(18, 186, 156);
const uint32_t RED_PIXEL = rgbToPixel(220, 22, 49);
const uint32_t PINK_PIXEL = rgbToPixel(251, 22, 184);
const uint32_t BLUE_PIXEL = rgbToPixel(32, 57, 130);
const uint32_t YELLOW_BOMB_PIXEL = rgbToPixel(29, 27, 8);
const uint32_t GREEN_BOMB_PIXEL = rgbToPixel(3, 39, 45);
const uint32_t RED_BOMB_PIXEL = rgbToPixel(66, 9, 15);
const uint32_t PINK_BOMB_PIXEL = rgbToPixel(59, 2, 50);
const uint32_t BLUE_BOMB_PIXEL = rgbToPixel(9, 5, 51);

// BGR0 byte-wise
const uint8_t YELLOW_DATA[] = {24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0, 24, 163, 235, 0};
const uint8_t GREEN_DATA[] = {156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0, 156, 186, 18, 0};
const uint8_t RED_DATA[] = {49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0, 49, 22, 220, 0};
const uint8_t PINK_DATA[] = {184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0, 184, 22, 251, 0};
const uint8_t BLUE_DATA[] = {130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0, 130, 57, 32, 0};
//const uint8_t YELLOW_BOMB_DATA[] = {0, 29, 27, 8, 0, 29, 27, 8, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7, 0, 29, 27, 7};
//const uint8_t BLUE_BOMB_DATA[] = {0, 9, 5, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51, 0, 9, 4, 51};

const uint8_t PHAGE_SILVER_DATA[] = {255, 255, 228, 0, 255, 255, 228, 0, 255, 255, 229, 0, 255, 255, 229, 0, 255, 255, 229, 0, 255, 255, 228, 0};
const uint8_t PHAGE_PINK_DATA[] = {122, 14, 178, 0, 148, 8, 221, 0, 149, 4, 222, 0, 150, 0, 224, 0, 150, 0, 224, 0, 150, 0, 224, 0, 150, 0, 224, 0, 149, 4, 222, 0};


template <typename T>
class XFreeWrapper {
    T& t;
public:
    XFreeWrapper(T& t) : t(t) {}
    ~XFreeWrapper() {XFree(t);}
};

class XDestroyImageWrapper {
    XImage *xImage;
public:
    XDestroyImageWrapper(XImage *xImage) : xImage(xImage) {}
    ~XDestroyImageWrapper() {XDestroyImage(xImage);}
};

std::optional<Window> getExapunksWindowImpl(Display *display, Window curWindow) {
    Window rootReturn;
    Window parentReturn;
    Window *childrenReturn;
    unsigned int nChildrenReturn;
    Status status = XQueryTree(display, curWindow, &rootReturn, &parentReturn, &childrenReturn, &nChildrenReturn);
    if (status == 0) {
        throw std::runtime_error("failed to XQueryTree");
    }
    XFreeWrapper wrap{childrenReturn};
    for (unsigned int i=0; i<nChildrenReturn; ++i) {
        char *windowName;
        status = XFetchName(display, childrenReturn[i], &windowName);
        if (status != 0) {
            XFreeWrapper wrap{windowName};
            if (!strcmp(EXAPUNKS_WINDOW_NAME, windowName)) {
                return {childrenReturn[i]};
            }
        }
        auto ret = getExapunksWindowImpl(display, childrenReturn[i]);
        if (ret) {
            return ret;
        }
    }
    return {};
}

void sendKey(Display* display, KeyCode keyCode) {
    XTestFakeKeyEvent(display, keyCode, True, 0);
    XSync(display, False);
    std::this_thread::sleep_for(KEY_DELAY);
    XTestFakeKeyEvent(display, keyCode, False, 0);
    XSync(display, False);
    std::this_thread::sleep_for(KEY_DELAY);
}

XImage* screenShotGame(Display* display, Window window) {
    XImage *xImage = XGetImage(display, window, BOARD_X_OFFSET, BOARD_Y_OFFSET, BOARD_PIXEL_WIDTH, BOARD_PIXEL_HEIGHT, AllPlanes, ZPixmap);
    if (xImage == nullptr) {
        throw std::runtime_error("failed to XGetimage");
    }
    return xImage;
}

// pixelcmp returns 0 if the two pixels are similar enough.
int pixelcmp(uint32_t p1, uint32_t p2) {
    int r = ((p1 & ASSUMED_XIMAGE_RED_MASK)>>16) - ((p2 & ASSUMED_XIMAGE_RED_MASK)>>16);
    int g = ((p1 & ASSUMED_XIMAGE_GREEN_MASK)>>8) - ((p2 & ASSUMED_XIMAGE_GREEN_MASK)>>8);
    int b = (p1 & ASSUMED_XIMAGE_BLUE_MASK) - (p2 & ASSUMED_XIMAGE_BLUE_MASK);
    if (r<0) r = -r;
    if (g<0) g = -g;
    if (b<0) b = -b;
    return (r+g+b > PIXEL_FUZZ);
}

// imgcmp is a replacement for memcmp which compares sequences of pixels.
// Returns 0 if all pixels are pairwise similar.
int imgcmp(const void *p1, const void *p2, size_t len) {
    const uint32_t *i1 = (const uint32_t *)p1;
    const uint32_t *i2 = (const uint32_t *)p2;
    size_t i = len/sizeof (uint32_t);
    while (i--) {
        if (pixelcmp(i1++[0], i2++[0]) != 0) {
            return 1;
        }
    }
    return 0;
}

uint8_t dataOffsettedToItem(char* data) {
    uint32_t pixel;
    memcpy(&pixel, data, sizeof(pixel));
    switch (pixel) {
    case YELLOW_PIXEL:
        if (memcmp(data, YELLOW_DATA, sizeof(YELLOW_DATA))==0) return Board::YELLOW;
        break;
    case GREEN_PIXEL:
        if (memcmp(data, GREEN_DATA, sizeof(GREEN_DATA))==0) return Board::GREEN;
        break;
    case RED_PIXEL:
        if (memcmp(data, RED_DATA, sizeof(RED_DATA))==0) return Board::RED;
        break;
    case PINK_PIXEL:
        if (memcmp(data, PINK_DATA, sizeof(PINK_DATA))==0) return Board::PINK;
        break;
    case BLUE_PIXEL:
        if (memcmp(data, BLUE_DATA, sizeof(BLUE_DATA))==0) return Board::BLUE;
        break;
    case YELLOW_BOMB_PIXEL:
        return Board::YELLOW_BOMB;
    case GREEN_BOMB_PIXEL:
        return Board::GREEN_BOMB;
    case RED_BOMB_PIXEL:
        return Board::RED_BOMB;
    case PINK_BOMB_PIXEL:
        return Board::PINK_BOMB;
    case BLUE_BOMB_PIXEL:
        return Board::BLUE_BOMB;
    }
    return Board::EMPTY;
}

constexpr std::size_t pixelCoordToDataOffset(int x, int y) {
    return BOARD_PIXEL_WIDTH*BYTES_PER_PIXEL*y + BYTES_PER_PIXEL*x;
}

std::optional<int> findGameYOffset(char* data) {
    for (int y=BOARD_PIXEL_HEIGHT_ITEMS; y-->0; ) {
        for (int i=0; i<Board::MAX_COLS; ++i) {
            const std::size_t dataOffset = pixelCoordToDataOffset(i*ITEM_SIZE + PIXEL_X_OFFSET, y);
            const uint8_t item = dataOffsettedToItem(data+dataOffset);
            switch (item) {
            case Board::YELLOW:
            case Board::GREEN:
            case Board::RED:
            case Board::PINK:
            case Board::BLUE:
                return {y % ITEM_SIZE};
            }
        }
    }
    std::cout << "no items on screen\n";
    return {};
}

std::optional<uint8_t> findPhageColumn(char* data) {
    for (uint8_t col=0; col<Board::MAX_COLS; ++col) {
        const std::size_t offset = pixelCoordToDataOffset(col*ITEM_SIZE + PHAGE_SILVER_DATA_X_OFFSET, PHAGE_SILVER_DATA_Y_OFFSET);
        if (memcmp(data+offset, PHAGE_SILVER_DATA, sizeof(PHAGE_SILVER_DATA)) == 0) {
            return {col};
        }
    }
    std::cout << "failed to find phage, probably crouched\n";
    return {};
}

uint8_t findHeld(char* data, const uint8_t phageCol) {
    const std::size_t heldOffset = pixelCoordToDataOffset(phageCol*ITEM_SIZE + PIXEL_X_OFFSET, PHAGE_HELD_Y_OFFSET);
    const uint8_t held = dataOffsettedToItem(data+heldOffset);
    const std::size_t pinkOffset = pixelCoordToDataOffset(phageCol*ITEM_SIZE + PHAGE_PINK_DATA_X_OFFSET, PHAGE_PINK_DATA_Y_OFFSET);
    const bool foundPink = 0 == memcmp(data+pinkOffset, PHAGE_PINK_DATA, sizeof(PHAGE_PINK_DATA));
    assert(foundPink == (held == Board::EMPTY));
    return held;
}

}

Window getExapunksWindow(Display *display) {
    Window rootWindow = XDefaultRootWindow(display);
    auto ret = getExapunksWindowImpl(display, rootWindow);
    if (ret) {
        return *ret;
    }
    throw std::runtime_error("failed to get exapunks window");
}

void validateAssumptions(Display *display, Window window) {
    XWindowAttributes xWindowAttributes;
    Status status = XGetWindowAttributes(display, window, &xWindowAttributes);
    if (status == 0) {
        throw std::runtime_error("failed to XGetWindowattributes");
    }
    bool error = false;
    if (xWindowAttributes.width != ASSUMED_WINDOW_WIDTH) {
        std::cerr << "window width is: " << xWindowAttributes.width << ", but was assumed to be: " << ASSUMED_WINDOW_WIDTH << '\n';
        error = true;
    }
    if (xWindowAttributes.height != ASSUMED_WINDOW_HEIGHT) {
        std::cerr << "window height is: " << xWindowAttributes.height << ", but was assumed to be: " << ASSUMED_WINDOW_HEIGHT << '\n';
        error = true;
    }
    if (error) {
        throw std::runtime_error("bad window size");
    }

    XImage *xImage = XGetImage(display, window, 0, 0, ASSUMED_WINDOW_WIDTH, ASSUMED_WINDOW_HEIGHT, AllPlanes, ZPixmap);
    if (xImage == nullptr) {
        throw std::runtime_error("failed to XGetimage");
    }
    XDestroyImageWrapper wrap{xImage};
    if (xImage->byte_order != ASSUMED_XIMAGE_BYTE_ORDER) {
        std::cerr << "byte_order is: " << xImage->byte_order << ", but was assumed to be: " << ASSUMED_XIMAGE_BYTE_ORDER << '\n';
        error = true;
    }
    if (xImage->bitmap_unit != ASSUMED_XIMAGE_BITMAP_UNIT) {
        std::cerr << "bitmap_unit is: " << xImage->bitmap_unit << ", but was assumed to be: " << ASSUMED_XIMAGE_BITMAP_UNIT << '\n';
        error = true;
    }
    if (xImage->bitmap_bit_order != ASSUMED_XIMAGE_BITMAP_BIT_ORDER) {
        std::cerr << "bitmap_bit_order is: " << xImage->bitmap_bit_order << ", but was assumed to be: " << ASSUMED_XIMAGE_BITMAP_BIT_ORDER << '\n';
        error = true;
    }
    if (xImage->bitmap_pad != ASSUMED_XIMAGE_BITMAP_PAD) {
        std::cerr << "bitmap_pad is: " << xImage->bitmap_pad << ", but was assumed to be: " << ASSUMED_XIMAGE_BITMAP_PAD << '\n';
        error = true;
    }
    if (xImage->depth != ASSUMED_XIMAGE_DEPTH) {
        std::cerr << "depth is: " << xImage->depth << ", but was assumed to be: " << ASSUMED_XIMAGE_DEPTH << '\n';
        error = true;
    }
    if (xImage->bytes_per_line != ASSUMED_XIMAGE_BYTES_PER_LINE_FULL_WINDOW) {
        std::cerr << "bytes_per_line is: " << xImage->bytes_per_line << ", but was assumed to be: " << ASSUMED_XIMAGE_BYTES_PER_LINE_FULL_WINDOW << '\n';
        error = true;
    }
    if (xImage->bits_per_pixel != ASSUMED_XIMAGE_BITS_PER_PIXEL) {
        std::cerr << "bits_per_pixel is: " << xImage->bits_per_pixel << ", but was assumed to be: " << ASSUMED_XIMAGE_BITS_PER_PIXEL << '\n';
        error = true;
    }
    if (xImage->red_mask != ASSUMED_XIMAGE_RED_MASK) {
        std::cerr << "red_mask is: " << xImage->red_mask << ", but was assumed to be: " << ASSUMED_XIMAGE_RED_MASK << '\n';
        error = true;
    }
    if (xImage->green_mask != ASSUMED_XIMAGE_GREEN_MASK) {
        std::cerr << "green_mask is: " << xImage->green_mask << ", but was assumed to be: " << ASSUMED_XIMAGE_GREEN_MASK << '\n';
        error = true;
    }
    if (xImage->blue_mask != ASSUMED_XIMAGE_BLUE_MASK) {
        std::cerr << "blue_mask is: " << xImage->blue_mask << ", but was assumed to be: " << ASSUMED_XIMAGE_BLUE_MASK << '\n';
        error = true;
    }

    uint32_t testPixel;
    memcpy(&testPixel, xImage->data, sizeof(testPixel));
    const int pixelPad = testPixel >> 24;
    if (pixelPad != ASSUMED_XIMAGE_PIXEL_PAD) {
        std::cerr << "pixelPad: " << pixelPad << ", but was assumed to be: " << ASSUMED_XIMAGE_PIXEL_PAD << '\n';
        error = true;
    }
    if (error) {
        throw std::runtime_error("bad XImage format");
    }

    for (int y=0; y<30; ++y) {
        for (int x=0; x<30; ++x) {
            const unsigned long correctPixel = XGetPixel(xImage, x, y);
            const int correctRed = correctPixel >> 16;
            const int correctGreen = (correctPixel & ASSUMED_XIMAGE_GREEN_MASK) >> 8;
            const int correctBlue = (correctPixel & ASSUMED_XIMAGE_BLUE_MASK);

            const std::size_t dataOffset = ASSUMED_XIMAGE_BYTES_PER_LINE_FULL_WINDOW * y + (ASSUMED_XIMAGE_BITS_PER_PIXEL/8) * x;
            uint32_t testPixel;
            memcpy(&testPixel, xImage->data + dataOffset, sizeof(testPixel));
            const int testRed = (testPixel & xImage->red_mask) >> 16;
            const int testGreen = (testPixel & xImage->green_mask) >> 8;
            const int testBlue = testPixel & xImage->blue_mask;
            if (correctRed != testRed) {
                std::cerr << "failed red pixel check. x: " << x << " y: " << y << " correct: " << correctRed << " test: " << testRed << '\n';
                error = true;
            }
            if (correctGreen != testGreen) {
                std::cerr << "failed green pixel check. x: " << x << " y: " << y << " correct: " << correctGreen << " test: " << testGreen << '\n';
                error = true;
            }
            if (correctBlue != testBlue) {
                std::cerr << "failed blue pixel check. x: " << x << " y: " << y << " correct: " << correctBlue << " test: " << testBlue << '\n';
                error = true;
            }
            if (error) {
                throw std::runtime_error("failed pixel check");
            }
        }
    }
}

void activateWindow(Display *display, Window window) {
    XSetInputFocus(display, window, RevertToNone, CurrentTime);
    XRaiseWindow(display, window);
    XSync(display, False);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

std::optional<PhageAndBoard> loadPhageAndBoardFromWindow(Display *display, Window window) {
    Timer timer{"loadPhageAndBoardFromWindow time"};
    XImage *xImage = screenShotGame(display, window);
    XDestroyImageWrapper wrap{xImage};
    const std::optional<int> yOffset = findGameYOffset(xImage->data);
    if (!yOffset) {
        return {};
    }
    PhageAndBoard phageAndBoard{};
    for (int i=0; i<Board::MAX_COLS; ++i) {
        const int x = i*ITEM_SIZE + PIXEL_X_OFFSET;
        for (int j=0; j<Board::MAX_ROWS; ++j) {
            const int y = j*ITEM_SIZE + *yOffset;
            const std::size_t dataOffset = pixelCoordToDataOffset(x, y);
            phageAndBoard.board.items[i][j] = dataOffsettedToItem(xImage->data+dataOffset);
            if (phageAndBoard.board.items[i][j] == Board::EMPTY) break;
            ++phageAndBoard.board.counts[i];
        }
    }
    const auto wrappedPhageColumn = findPhageColumn(xImage->data);
    if (!wrappedPhageColumn) {
        return {};
    }
    phageAndBoard.phageCol = *wrappedPhageColumn;
    phageAndBoard.board.held = findHeld(xImage->data, phageAndBoard.phageCol);
    return {phageAndBoard};
}

void moveLeft(Display* display) {
    KeyCode keyCodeS = XKeysymToKeycode(display, XK_s);
    sendKey(display, keyCodeS);
}

void moveRight(Display* display) {
    KeyCode keyCodeF = XKeysymToKeycode(display, XK_f);
    sendKey(display, keyCodeF);
}
void swap(Display* display) {
    KeyCode keyCodeK = XKeysymToKeycode(display, XK_k);
    sendKey(display, keyCodeK);
}

void tractorBeam(Display *display) {
    KeyCode keyCodeJ = XKeysymToKeycode(display, XK_j);
    sendKey(display, keyCodeJ);
}
}}
