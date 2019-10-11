#include <stdlib.h>
#include <string.h>

#include <galasm/galasm2.h>

#include "galasm.h"


int assemblePldFile(const char* filename,
                    int GenFuse,
                    int GenChip,
                    int GenPin,
                    int JedecSecBit,
                    int JedecFuseChk) {  // This version is called from main
    Config_t cfg;
    cfg.GenFuse = GenFuse;
    cfg.GenChip = GenChip;
    cfg.GenPin = GenPin;
    cfg.JedecSecBit = JedecSecBit;
    cfg.JedecFuseChk = JedecFuseChk;

    int fileSize = FileSize(filename);

    unsigned char* buffer = (unsigned char* )malloc(fileSize);
    if (!buffer ) {
        ErrorReq(2);
        return -1;
    }

    bool ok = ReadFile(filename, fileSize, buffer);
    if (!ok) {
        free(buffer);
        ErrorReq(3);
        return -1;
    }

    int rc = AssemblePldFile(filename, buffer, fileSize, &cfg);
    free(buffer);

    return rc;
}

int assemblePldMemory(const char* filename,
                      unsigned char* fbuff2,
                      int size2,
                      int GenFuse,
                      int GenChip,
                      int GenPin,
                      int JedecSecBit,
                      int JedecFuseChk) {
    Config_t cfg;
    cfg.GenFuse = GenFuse;
    cfg.GenChip = GenChip;
    cfg.GenPin = GenPin;
    cfg.JedecSecBit = JedecSecBit;
    cfg.JedecFuseChk = JedecFuseChk;

    return AssemblePldFile(filename, fbuff2, size2, &cfg);
}

