
#
# Makefile for CustomMac.rom
#
# Written in 2017 by David Lee Ludwig <dludwig@pobox.com>
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along
# with this software. If not, see
# <http://creativecommons.org/publicdomain/zero/1.0/>.
#

M68K_CXX=m68k-linux-gnu-g++
M68K_OBJDUMP=m68k-linux-gnu-objdump


CustomMac.rom: MakeMacRom RomMain.o
	./MakeMacRom CustomMac.rom RomMain.o

#
# m68k g++ flag-usage may include the following:
#   -Os
#      Optimize for small binaries
#
#   -fno-builtin
#      Don't replace compiled code with normally-equivalent C/C++ std-library
#      calls (which is done on some systems, to help with optimization).
#
#   -m68000
#      Compile to code compatible with 68000 CPUs (and not, say, a 68020 CPU).
#
#   -nostdlib
#      Do not attempt to link to system libraries, which are almost certainly
#      targeting the wrong OS (such as Linux).
#
#   -fno-exceptions
#      Don't use C++ exceptions.  This is probably not strictly needed, however
#      it does result in smaller compiled-code.
#
#   -mpcrel
#      When calling C/C++ functions, jump to addresses relative to the current
#      PC (Program Counter) CPU-register.  This prevents the need for functions
#      to be loaded into any particular address(es) of memory.
#
#   -Xlinker --oformat=binary
#      When linking the code, don't apply any extra metadata regarding the
#      program as a whole (such as ELF metadata, as seen in Linux binaries).
#

RomMain.o: RomMain.cpp Picture.xbm Makefile
	$(M68K_CXX) RomMain.cpp -o RomMain.o -Os -fno-builtin -m68000 -nostdlib -fno-exceptions -mpcrel -Xlinker --oformat=binary

MakeMacRom: MakeMacRom.cpp Makefile
	$(CXX) -std=c++14 MakeMacRom.cpp -o MakeMacRom

disasm: RomMain.o
	$(M68K_OBJDUMP) -D -d -m m68k -b binary RomMain.o

clean:
	$(RM) -f MakeMacRom
	$(RM) -f RomMain.o
	$(RM) -f CustomMac.rom
