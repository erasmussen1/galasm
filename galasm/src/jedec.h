#ifndef JEDEC_H
#define JEDEC_H

#include <stdint.h>

#include "galasm_types.h"


/*#
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

void initJedec(JedecStruct_t* jedec);
void setMode(JedecStruct_t* jedec, int modus);
int FuseChecksum(JedecStruct_t* jedec, int galtype);


#endif

