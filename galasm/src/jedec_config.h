#ifndef JEDEC_CONF_H
#define JEDEC_CONF_H

#include <stdbool.h>

/*
 * This structure is used to store GALasm's configuration
 */
typedef struct {
    int gal_type;
    bool JedecSecBit; /* set security bit in JEDEC? */
    bool JedecFuseChk;
    bool GenFuse; /* generate fuse file?        */
    bool GenChip; /* generate chip file?        */
    bool GenPin;  /* generate pin file?         */
    /* calc. fuse checksum?       */
    /* azummo: if false, file checksum will be generated */
} Config_t;

#endif
