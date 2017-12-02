
//
// MakeMacRom - Helps build a custom ROM file for an emulated 68k Mac
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

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

// RomWriteError - thrown when an invalid write occurs, when building a Mac .rom file 
struct RomWriteError : std::exception {
    std::string _errorMessage;

    RomWriteError(const char * errorName, size_t errorAt, size_t writeStart, size_t writeSize) {
        std::ostringstream os;
        os << std::hex;
        os << errorName << " at 0x" << (errorAt)
           << ", via 0x" << writeSize << " byte write"
           << " starting at 0x" << writeStart;
        _errorMessage = os.str();
    }

    virtual const char* what() const noexcept {
        return _errorMessage.c_str();
    }
};

// RomWriter - utility class for writing binary data to a file, without overlaps
//
// Attempts to overwrite data, or write data past the end of _data's size, will
// result in an exception being thrown, of type RomWriteError.
struct RomWriter {
    // ROM data to-be
    std::vector<uint8_t> _data;

    // Record of already-written bytes; same size as _data; each
    // element is 0 if unwritten, or 1 if written.  All elements are initially
    // 0.  Elements are set to 1 as writes occur to _data.
    std::vector<uint8_t> _writtenBytes;

    // Constructor
    RomWriter(size_t sizeInBytes) :
        _data(sizeInBytes, 0x00),
        _writtenBytes(sizeInBytes, 0x00)
    {
    }

    // Basic accessors
    uint8_t * data()            { return _data.data(); }
    uint8_t * writtenBytes()    { return _writtenBytes.data(); }
    size_t size() const         { return _data.size(); }

    // Write the contents of a separate file, to _data
    void WriteFromFile(size_t offset, const char * fileName) {
        // Open file, or bail
        FILE * file = fopen(fileName, "r");
        if ( ! file) {
            fprintf(stderr, "ERROR: Unable to open code file for reading. Reason: %s\n",
                strerror(errno));
            std::exit(1);
        }

        // Get size of file
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Ensure valid write, bailing if not
        CheckWrite(offset, fileSize);

        // Copy file to data
        printf("INFO: writing 0x%x bytes at 0x%x, from \"%s\"\n",
            (int)fileSize,
            (int)offset,
            fileName
        );
        fread((data() + offset), fileSize, 1, file);

        // Read is done.  Clean up.
        fclose(file);
    }

    // Writes a set of bytes to _data
    void WriteBytes(size_t offset, std::initializer_list<uint8_t> bytes) {
        CheckWrite(offset, bytes.size());
        memcpy(data() + offset, bytes.begin(), bytes.size());
        memset(writtenBytes() + offset, 1, bytes.size());
    }

    // Called before every write, this will lead to program-error if a
    // double-write is detected, on a specific byte within _data.
    void CheckWrite(size_t offset, size_t numBytes) {
        if ((offset + numBytes) > size()) {
            throw RomWriteError("write past end of ROM", size(), offset, numBytes);
        }

        for (size_t i = 0; i < numBytes; ++i) {
            if ((writtenBytes() + offset)[i] != 0) {
                throw RomWriteError("double-write", offset + i, offset, numBytes);
            }
        }
    }
};

int main(int argc, char ** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s OUTPUT_FILE INPUT_COMPILED_CODE\n\n",
            argc > 0 ? argv[0] : "makerom");
        return 1;
    }

    FILE * outFile = fopen(argv[1], "w");
    if ( ! outFile) {
        fprintf(stderr, "ERROR: Unable to open output file for writing. Reason: %s\n",
            strerror(errno));
        return 1;
    }

    try {
        // Setup for a 512 KB .rom file
        RomWriter writer(1024 * 512);

        // Write: initial PC (Program Counter) value
        fprintf(stderr, "INFO: writing initial PC (Program Counter)\n");
        writer.WriteBytes(0x4, {0x00, 0x40, 0x00, 0x2a});

        // Write: compiled code
        writer.WriteFromFile(0x2a, argv[2]);

        // Finish writing ROM to disk
        fwrite(writer.data(), writer.size(), 1, outFile);
        fclose(outFile);
        fprintf(stderr, "INFO: Rom successfully written to: %s\n", argv[1]);
    }
    catch (const std::exception & ex) {
        fprintf(stderr, "ERROR: %s\n", ex.what());
        return 1;
    }

    return 0;
}
