#ifndef GALASM_H
#define GALASM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int assemblePldFile(const char* filename,
                    int GenFuse,
                    int GenChip,
                    int GenPin,
                    int JedecSecBit,
                    int JedecFuseChk,
                    bool verbose,
                    bool unixNewline);

#ifdef __cplusplus
}
#endif

#endif
