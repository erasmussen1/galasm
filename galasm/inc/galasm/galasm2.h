#ifndef GALASM_H
#define GALASM_H

#ifdef __cplusplus
extern "C" {
#endif

int assemblePldFile(
    const char* filename, int GenFuse, int GenChip, int GenPin, int JedecSecBit, int JedecFuseChk);

int assemblePldMemory(const char* filename,
                      unsigned char* buff,
                      int size2,
                      int GenFuse,
                      int GenChip,
                      int GenPin,
                      int JedecSecBit,
                      int JedecFuseChk);


#ifdef __cplusplus
}
#endif

#endif
