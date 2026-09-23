#ifndef GALASM_TYPES_H
#define GALASM_TYPES_H

#include <stdint.h>

/* Number of entries per buffer */
#define ENTRY_SIZE 256

/**
 * This structure is used to store some data in
 * a chained list e.g. the coded equations for the optimizer
 */
typedef struct Buffer_t {
    struct Buffer_t* Next;
    struct Buffer_t* Prev;
    uint8_t Entries[ENTRY_SIZE];
} Buffer_t;

/**
 * Used to store results and parameters of functions
 * which deal with chained lists
 */
typedef struct {
    Buffer_t* ThisBuff; /* pointer to current buffer */
    uint8_t* Entry;     /* pointer to data area      */
    uint8_t* BuffEnd;   /* pointer to the end of the buffer */
} ActBuffer_t;

/*
 * This structure is used to store GALasm's configuration
 */
typedef struct {
    bool verbose;
    bool unixNewline;
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


#endif  // GALASM_TYPES_H
