#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdbool.h>

#include "galasm_types.h"

#ifdef __cplusplus
extern "C" {
#endif


int FileSize(const char* filename);
bool ReadFile(const char* filename, int filesize, uint8_t* filebuff);
int AddByte(ActBuffer_t* buff, uint8_t code);
int AddString(ActBuffer_t* buff, uint8_t* strnptr);
void IncPointer(ActBuffer_t* buff);
void DecPointer(ActBuffer_t* buff);
void FreeBuffer(Buffer_t* buff);

#ifdef __cplusplus
}
#endif

#endif  // SUPPORT_H
