#ifndef JEDEC_CONF_H
#define JEDEC_CONF_H

#include <stdbool.h>

/*
 * This structure is used to store GALasm's configuration
 */
typedef struct {
    bool verbose;
    int gal_type;
    bool JedecSecBit; /* set security bit in JEDEC?      */
    bool JedecFuseChk;
    bool GenFuse; /* generate fuse file?                 */
    bool GenChip; /* generate chip file?                 */
    bool GenPin;  /* generate pin file?                  */
    /* calc. fuse checksum?                              */
    /* azummo: if false, file checksum will be generated */

    int num_of_olmcs; /* number of OLMCs                 */
    int num_of_pins;  /* number of pins                  */
    int num_of_col;   /* number of col.                  */
    char name[32];
    uint8_t PinNames[24 + 1][10];  // TODO - Find why PinNames[24][10] has memory leaks
} Config_t;

#endif
