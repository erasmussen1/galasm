#include <stdlib.h>
#include <string.h>

#include <galasm/galasm2.h>
#include "support.h"
#include "galasm.h"


/**
 * This is called from main
 */
int assemblePldFile(const char* filename,
                    int GenFuse,
                    int GenChip,
                    int GenPin,
                    int JedecSecBit,
                    int JedecFuseChk,
                    bool verbose,
                    bool unixNewline) {
    Config_t cfg;
    cfg.verbose = verbose;
    cfg.unixNewline = unixNewline;
    cfg.GenFuse = GenFuse;
    cfg.GenChip = GenChip;
    cfg.GenPin = GenPin;
    cfg.JedecSecBit = JedecSecBit;
    cfg.JedecFuseChk = JedecFuseChk;

    int fileSize = FileSize(filename);

    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (!buffer) {
        return -1;
    }

    bool ok = ReadFile(filename, fileSize, buffer);
    if (!ok) {
        free(buffer);
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
                      int JedecFuseChk,
                      bool verbose,
                      bool unixNewline) {
    Config_t cfg;
    cfg.verbose = verbose;
    cfg.unixNewline = unixNewline;
    cfg.GenFuse = GenFuse;
    cfg.GenChip = GenChip;
    cfg.GenPin = GenPin;
    cfg.JedecSecBit = JedecSecBit;
    cfg.JedecFuseChk = JedecFuseChk;

    return AssemblePldFile(filename, fbuff2, size2, &cfg);
}
