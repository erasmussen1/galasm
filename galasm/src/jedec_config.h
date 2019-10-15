#ifndef JEDEC_CONF_H
#define JEDEC_CONF_H

#include <stdbool.h>

/*
 * This structure is used to store GALasm's configuration
 */
typedef struct {
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
} Config_t;

#endif
