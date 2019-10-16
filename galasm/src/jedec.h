#ifndef JEDEC_H
#define JEDEC_H

#include <stdint.h>

#include "jedec_config.h"
#include "galasm_types.h"


/**
 * this structure is used to store the fuses in a
 * kind of JEDEC format
 */
typedef struct {
    uint8_t GALLogic[5808]; /*max. size of fuse matrix */
    uint8_t GALXOR[10];     /* XOR bits                */
    uint8_t GALSig[64];     /* signature               */
    uint8_t GALAC1[8];      /* AC1 bits                */
    uint8_t GALPT[64];      /* product term disable    */
    uint8_t GALSYN;         /* SYN bit                 */
    uint8_t GALAC0;         /* AC0 bit                 */
    uint8_t GALS1[10];      /* S1 bits for 22V10       */
} JedecStruct_t;

#define ENTRY_SIZE 256 /* number of entries per buffer */

/**
 * this structure is used to store some datas in a chained list
 * e.g. the coded equations for the optimizer
 */
typedef struct Buffer_t {
    struct Buffer_t* Next;
    struct Buffer_t* Prev;
    uint8_t Entries[ENTRY_SIZE]; /* data area */
} Buffer_t;

/**
 * used to store results and
 * parameters of functions
 * which deal with chained lists
 */
typedef struct {
    Buffer_t* ThisBuff; /* pointer to current buffer */
    uint8_t* Entry;     /* pointer to data area      */
    uint8_t* BuffEnd;   /* pointer to the end of the buffer */
} ActBuffer_t;

int FileChecksum(ActBuffer_t buff);
void initJedec(JedecStruct_t* jedec);
void setMode(JedecStruct_t* jedec, int modus);
int FuseChecksum(JedecStruct_t* jedec, int galtype);
int MakeJedecBuff(JedecStruct_t* jedec, Config_t* cfg, ActBuffer_t buff);
void WriteJedecFile(char* filename, JedecStruct_t* jedec, Config_t* jedecConf);

#endif
