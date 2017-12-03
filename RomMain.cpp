
//
// MacRomPicture - Shows a 1-bit, XBM bitmap on an emulated (via pce-macplus) 4 MB Mac Plus.
//
// Written in 2017 by David Lee Ludwig <dludwig@pobox.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//

// Compiled with g++ flags:
//  -Os -o romtest.o -fno-builtin -m68000 -nostdlib -mpcrel -Xlinker --oformat=binary
//
// Kudos to Jon Sharp, for blog posts with details on bare-metal 68k Mac programming:
//   http://jonsharp.net/retrocomputing/bare-metal-macintosh-programming-part-1/
//   http://jonsharp.net/retrocomputing/bare-metal-macintosh-programming-part-2/

#include <stdint.h>
#include <algorithm>

// #include the picture-to-display, via use of a 1-bit XBM bitmap, a file
// format that encodes pictures as plain C code.
#include "Picture.xbm"


//
// Hardware-related variables
//

// ScrnBase: Points to the start of the display-buffer
//   Credits here to http://www.osdata.com/system/physical/memmap.htm#MacPlusvideo
//
// uint8_t * ScrnBase = (uint8_t *) 0xFA700;    // Mac Plus, 1 MB RAM, Main
// uint8_t * ScrnBase = (uint8_t *) 0xF2700;    // Mac Plus, 1 MB RAM, Alternate
// uint8_t * ScrnBase = (uint8_t *) 0x1FA700;   // Mac Plus, 2 MB RAM, Main
// uint8_t * ScrnBase = (uint8_t *) 0x1F2700;   // Mac Plus, 2 MB RAM, Alternate
// uint8_t * ScrnBase = (uint8_t *) 0x27A700;   // Mac Plus, 2.5 MB RAM, Main
// uint8_t * ScrnBase = (uint8_t *) 0x272700;   // Mac Plus, 2.5 MB RAM, Alternate
uint8_t * ScrnBase = (uint8_t *) 0x3FA700;      // Mac Plus, 4 MB RAM, Main
// uint8_t * ScrnBase = (uint8_t *) 0x3F2700;   // Mac Plus, 4 MB RAM, Alternate

// Screen Dimensions
const int ScrnWidth = 512;
const int ScrnHeight = 342;

// VIA1 memory addresses and offsets
uint8_t * const vBase = (uint8_t *) 0xEFE1FE;   // base address to VIA1
const int vDirA = 512 * 3;                      // offset from vBase, to VIA data register A


//
// Utility functions
//

// Reverses order of bits in a given byte
static uint8_t ReverseBits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}


//
// main() function
//

int main(int argc, char **argv) {
    // Setup the 68k CPU's Stack Pointer (register "sp", aka. "a7")
    __asm__ (
        "movea.l #00100000,%%sp\n\t"
        : /* outputs */
        : /* inputs */
        : "sp" /* clobbered regs */
    );

    // Setup VIA1 Data Register A.  Not doing this causes the PCE emulator
    // to not fully setup its internal, RAM-tracking, data structures.
    //
    // This doesn't appear to be needed by PCE, when emulating something
    // more recent than a Mac Plus.
    vBase[vDirA] = 0x3F;

    // Fill screen with white
    for (int i = 0; i < ((ScrnWidth * ScrnHeight) >> 3); ++i) {
        ScrnBase[i] = 0x00;
    }

    // Draw Picture at top-left of screen
    const int drawHeight = std::min(Picture_height, ScrnHeight);
    const int drawWidth = std::min(Picture_width, ScrnWidth);
    for (int y = 0; y < drawHeight; ++y) {
        for (int x = 0; x < (drawWidth >> 3); ++x) {
            // The XBM image's bits are reversed.  This may be an artifact of
            // XBM itself, or by The Gimp's XBM exporter, or how Mac Plus's like
            // its sits.  Any which way, reversing the bits gets the correct
            // value to display.
            const uint8_t b = ReverseBits(Picture_bits[(y * (Picture_width >> 3)) + x]); 
            ScrnBase[(y * (ScrnWidth >> 3)) + x] = b;
        }
    }

    // Sit in a no-op loop, so we don't try to execute random data, past this
    // program.
    while (1) {
        __asm__ (
            "nop\n\t"
        );  
    }

    return 0;
}

