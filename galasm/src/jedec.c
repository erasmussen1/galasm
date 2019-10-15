/******************************************************************************
** JEDEC.c
*******************************************************************************
**
** description:
**
** This file contains some functions to save GAL data in JEDEC format.
**
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jedec.h"
#include "galasm.h"

static size_t WriteOutput(void* buf, size_t size, size_t nmemb, FILE* out);

void initJedec(JedecStruct_t* jedec) {
    memset(jedec, 0x00, sizeof(*jedec));
    memset(jedec->GALLogic, 1, sizeof(jedec->GALLogic));
}

void setMode(JedecStruct_t* jedec, int modus) {
    switch (modus) {
        default:
        case MODE1:
            /* set SYN and AC0 bit */
            jedec->GALSYN = 1;
            jedec->GALAC0 = 0;
            break;

        case MODE2:
            /* set SYN, AC0 for mode 2 */
            jedec->GALSYN = 1;
            jedec->GALAC0 = 1;
            break;

        case MODE3:
            /* set SYN and AC0 for mode 3 */
            jedec->GALSYN = 0;
            jedec->GALAC0 = 1;
            break;
    }
}

/******************************************************************************
** FileChecksum()
*******************************************************************************
** input:   buff    ActBuffer structure of file buffer
**
** output:          16 bit checksum of the JEDEC structure
**
** remarks: This function calculates the JEDEC file checksum. The start and
**          the end of the area for which the checksum should be calculated
**          must be marked by <STX> and <ETX>!.
******************************************************************************/
int FileChecksum(ActBuffer_t buff) {
    int checksum = 0;

    /* search for <STX> */
    while (*buff.Entry != 0x2) {
        IncPointer(&buff);
    }

    /* search for <ETX> and */
    while (*buff.Entry != 0x3) {
        checksum += *buff.Entry;

        IncPointer(&buff);
    }

    checksum += 0x3; /* add <ETX> too */

    return checksum;
}


/******************************************************************************
** FuseChecksum()
*******************************************************************************
** input:   galtype     type of GAL
**
** output:              16 bit checksum of the JEDEC structure
**
** remarks: This function does calculate the fuse checksum of the JEDEC
**          structure.
******************************************************************************/
int FuseChecksum(JedecStruct_t* jedec, int galtype) {
    uint8_t *ptr = jedec->GALLogic - 1L;
    uint8_t *ptrXOR = jedec->GALXOR;
    uint8_t *ptrS1 = jedec->GALS1;

    int n = 0;
    int checksum = 0;
    int byte = 0;

    for (;;) {
        if (galtype == GAL16V8) {
            if (n == XOR16) {
                ptr = jedec->GALXOR;
            } else {
                if (n == XOR16 + 8) {
                    ptr = jedec->GALSig;
                } else {
                    if (n == NUMOFFUSES16)
                        break;
                    else
                        ptr++;
                }
            }
        }

        if (galtype == GAL20V8) {
            if (n == XOR20) {
                ptr = jedec->GALXOR;
            } else {
                if (n == XOR20 + 8) {
                    ptr = jedec->GALSig;
                } else {
                    if (n == NUMOFFUSES20)
                        break;
                    else
                        ptr++;
                }
            }
        }

        if (galtype == GAL22V10) {
            if (n >= XOR22V10 && n < XOR22V10 + 20) {
                if (!(n % 2))
                    ptr = ptrXOR++;
                else
                    ptr = ptrS1++;
            } else {
                if (n == SIG22V10)
                    ptr = jedec->GALSig - 1L;

                if (n == SIG22V10 + SIG_SIZE)
                    break;
                else
                    ptr++;
            }
        }

        if (galtype == GAL20RA10) {
            if (n == XOR20RA10) {
                ptr = jedec->GALXOR;
            } else {
                if (n == SIG20RA10 + SIG_SIZE)
                    break;
                else
                    ptr++;
            }
        }

        byte |= (*ptr << (n + 8) % 8);

        if (!((n + 9) % 8)) {
            checksum += byte;
            byte = 0;
        }

        n++;
    }

    checksum += byte;

    return (checksum);
}

/**
 * This variables are defined both globally AND locally!
 * All assignments concern to the locale variables!
 */
static void getChipConfiguration(Config_t* cfg, int* MaxFuseAdr, int* RowSize, int* XORSize) {
    switch (cfg->gal_type) {
        case GAL16V8:
            *MaxFuseAdr = MAX_FUSE_ADR16;
            *RowSize = ROW_SIZE_16V8;
            *XORSize = 8;
            break;

        case GAL20V8:
            *MaxFuseAdr = MAX_FUSE_ADR20;
            *RowSize = ROW_SIZE_20V8;
            *XORSize = 8;
            break;

        case GAL22V10:
            *MaxFuseAdr = MAX_FUSE_ADR22V10;
            *RowSize = ROW_SIZE_22V10;
            *XORSize = 10;
            break;

        case GAL20RA10:
            *MaxFuseAdr = MAX_FUSE_ADR20RA10;
            *RowSize = ROW_SIZE_20RA10;
            *XORSize = 10;
            break;
    }
}

/******************************************************************************
** MakeJedecBuff()
*******************************************************************************
** input:   buff     ActBuffer structure for the ram buffer
**          galtype  type of GAL
**			cfg		 pointer to the config structure
**
** output:  0:  o.k.
**         -1:  not enough free memory
**
** remarks: generates the JEDEC file in a ram buffer
******************************************************************************/
int MakeJedecBuff(JedecStruct_t* jedec, Config_t *cfg, ActBuffer_t buff) {
    UBYTE mystrng[255];
    ActBuffer_t buff2;
    int n, m, bitnum, bitnum2, flag;

    int MaxFuseAdr = 0;
    int RowSize = 0;
    int XORSize = 0;

    getChipConfiguration(cfg, &MaxFuseAdr, &RowSize, &XORSize);

    buff2 = buff;

    if (!cfg->JedecFuseChk) {
        if (AddString(&buff, (UBYTE*)"\2\n")) { /* <STX> */
            return (-1);
        }
    }

    /*** make header of JEDEC file ***/
    if (AddString(&buff, (UBYTE*)"Used Program:   GALasm 2.1\n"))
        return (-1);

    if (AddString(&buff, (UBYTE*)"GAL-Assembler:  GALasm 2.1\n"))
        return (-1);

    int galtype = cfg->gal_type;
    if (galtype == GAL16V8) {
        if (AddString(&buff, (UBYTE*)"Device:         GAL16V8\n\n"))
            return (-1);
    }

    if (galtype == GAL20V8) {
        if (AddString(&buff, (UBYTE*)"Device:         GAL20V8\n\n"))
            return (-1);
    }

    if (galtype == GAL20RA10) {
        if (AddString(&buff, (UBYTE*)"Device:         GAL20RA10\n\n"))
            return (-1);
    }

    if (galtype == GAL22V10) {
        if (AddString(&buff, (UBYTE*)"Device:         GAL22V10\n\n"))
            return (-1);
    }

    /* Default value of fuses */
    if (AddString(&buff, (UBYTE*)"*F0\n")) {
        return (-1);
    }

    /* Security-Bit */
    if (cfg->JedecSecBit) {
        if (AddString(&buff, (UBYTE*)"*G1\n")) {
            return (-1);
        }
    } else {
        if (AddString(&buff, (UBYTE*)"*G0\n")) {
            return (-1);
        }
    }

    if (galtype == GAL16V8) /* number of fuses */
        if (AddString(&buff, (UBYTE*)"*QF2194\n"))
            return (-1);

    if (galtype == GAL20V8)
        if (AddString(&buff, (UBYTE*)"*QF2706\n"))
            return (-1);

    if (galtype == GAL20RA10)
        if (AddString(&buff, (UBYTE*)"*QF3274\n"))
            return (-1);

    if (galtype == GAL22V10)
        if (AddString(&buff, (UBYTE*)"*QF5892\n"))
            return (-1);

    /*** make fuse-matrix ***/
    bitnum = bitnum2 = flag = 0;

    for (m = 0; m < RowSize; m++) {
        flag = 0;

        bitnum2 = bitnum;

        for (n = 0; n <= MaxFuseAdr; n++) {
            if (jedec->GALLogic[bitnum2]) {
                flag = 1;
                break;
            }

            bitnum2++;
        }

        if (flag) {
            sprintf((char*)&mystrng[0], "*L%04d ", bitnum);

            if (AddString(&buff, (UBYTE*)&mystrng[0]))
                return (-1);

            for (n = 0; n <= MaxFuseAdr; n++) {
                if (AddByte(&buff, (uint8_t)(jedec->GALLogic[bitnum] + '0')))
                    return (-1);
                bitnum++;
            }

            if (AddByte(&buff, (UBYTE)'\n'))
                return (-1);
        } else {
            bitnum = bitnum2;
        }
    }

    if (!flag) {
        bitnum = bitnum2;
    }

    /*** XOR-Bits ***/
    sprintf((char*)&mystrng[0], "*L%04d ", bitnum); /* add fuse adr. */
    if (AddString(&buff, (UBYTE*)&mystrng[0]))
        return (-1);

    for (n = 0; n < XORSize; n++) {
        if (AddByte(&buff, (uint8_t)(jedec->GALXOR[n] + '0')))
            return (-1);
        bitnum++;

        if (galtype == GAL22V10) { /*** S1 of 22V10 ***/
            if (AddByte(&buff, (uint8_t)(jedec->GALS1[n] + '0')))
                return (-1);
            bitnum++;
        }
    }

    if (AddByte(&buff, (UBYTE)'\n')) {
        return (-1);
    }

    /*** Signature ***/
    sprintf((char*)&mystrng[0], "*L%04d ", bitnum);

    if (AddString(&buff, (UBYTE*)&mystrng[0]))
        return (-1);

    for (n = 0; n < SIG_SIZE; n++) {
        if (AddByte(&buff, (uint8_t)(jedec->GALSig[n] + '0'))) {
            return (-1);
        }
        bitnum++;
    }

    if (AddByte(&buff, (UBYTE)'\n'))
        return (-1);


    if ((galtype == GAL16V8) || (galtype == GAL20V8)) {

        /*** AC1-Bits ***/
        sprintf((char*)&mystrng[0], "*L%04d ", bitnum);
        if (AddString(&buff, (UBYTE*)&mystrng[0]))
            return (-1);

        for (n = 0; n < AC1_SIZE; n++) {
            if (AddByte(&buff, (uint8_t)(jedec->GALAC1[n] + '0')))
                return (-1);
            bitnum++;
        }

        if (AddByte(&buff, (UBYTE)'\n'))
            return (-1);


        /*** PT-Bits ***/
        sprintf((char*)&mystrng[0], "*L%04d ", bitnum);
        if (AddString(&buff, (UBYTE*)&mystrng[0]))
            return (-1);

        for (n = 0; n < PT_SIZE; n++) {
            if (AddByte(&buff, (uint8_t)(jedec->GALPT[n] + '0')))
                return (-1);
            bitnum++;
        }

        if (AddByte(&buff, (UBYTE)'\n')) {
            return (-1);
        }

        /*** SYN-Bit ***/
        sprintf((char*)&mystrng[0], "*L%04d ", bitnum);

        if (AddString(&buff, (UBYTE*)&mystrng[0]))
            return (-1);

        if (AddByte(&buff, (uint8_t)(jedec->GALSYN + '0')))
            return (-1);

        if (AddByte(&buff, (UBYTE)'\n'))
            return (-1);

        bitnum++;

        /*** AC0-Bit ***/
        sprintf((char*)&mystrng[0], "*L%04d ", bitnum);

        if (AddString(&buff, (UBYTE*)&mystrng[0]))
            return (-1);

        if (AddByte(&buff, (uint8_t)(jedec->GALAC0 + '0')))
            return (-1);

        if (AddByte(&buff, (UBYTE)'\n'))
            return (-1);
    }


    // if (cfg->JedecFuseChk) {
    sprintf((char*)&mystrng[0], "*C%04x\n", FuseChecksum(jedec, cfg->gal_type));

    if (AddString(&buff, (UBYTE*)&mystrng[0])) {
        return (-1);
    }
    // }

    if (AddString(&buff, (UBYTE*)"*\n")) { /* closing '*' */
        return (-1);
    }

    if (!cfg->JedecFuseChk) {
        if (AddByte(&buff, (UBYTE)0x3)) { /* <ETX> */
            return (-1);
        }

        sprintf((char*)&mystrng[0], "%04x\n", FileChecksum(buff2));

        if (AddString(&buff, (UBYTE*)&mystrng[0])) {
            return (-1);
        }
    }

    return (0);
}

/******************************************************************************
** WriteJedecFile()
*******************************************************************************
** input:   galtype     type of GAL
**          cfg			configuration structure
**
** output:  none
**
** remarks: generats the JEDEC file out of the JEDEC structure
******************************************************************************/
void WriteJedecFile(char* filename, JedecStruct_t* jedec, Config_t* cfg) {
    ActBuffer_t mybuff;
    Buffer_t* first_buff;
    UBYTE *filebuffer, *filebuffer2;

    if (!(first_buff = (Buffer_t*)calloc(sizeof(Buffer_t), 1))) {
        ErrorReq(2); /* out of memory? */
        return;
    }

    mybuff.ThisBuff = first_buff;
    mybuff.Entry = (UBYTE*)(&first_buff->Entries[0]);
    mybuff.BuffEnd = (UBYTE*)first_buff + (long)sizeof(Buffer_t);

    /* put JEDEC in ram-buffer */
    if (MakeJedecBuff(jedec, cfg, mybuff)) {
        FreeBuffer(first_buff);
        ErrorReq(2);
        return;
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        FreeBuffer(first_buff); /* error?, then cancel*/
        ErrorReq(13);           /* can't open file */
        return;
    }

    for (;;) {
        filebuffer = filebuffer2 = mybuff.Entry;

        while (filebuffer2 < mybuff.BuffEnd) { /* get size of buffer */
            if (!*filebuffer2) {
                break;
            }
            filebuffer2++;
        }

        /* save buffer */
        /* DHH - 24-Oct-2012: ensure lines are terminated with CRLF */
        long result = WriteOutput(filebuffer, (size_t)1, (size_t)(filebuffer2 - filebuffer), fp);
        if (result != (filebuffer2 - filebuffer)) { /* write error? */
            fclose(fp);
            FreeBuffer(first_buff);
            ErrorReq(13);
            return;
        }

        if (!mybuff.ThisBuff->Next) {
            break;
        }

        mybuff.ThisBuff = mybuff.ThisBuff->Next;
        mybuff.Entry = (UBYTE*)(&mybuff.ThisBuff->Entries[0]);
        mybuff.BuffEnd = (UBYTE*)mybuff.ThisBuff + (long)sizeof(Buffer_t);
    }

    fclose(fp);

    FreeBuffer(first_buff);
}

/*
 * DHH - 24-Oct-2012
 * This function works like fwrite, but outputs newlines as CRLF.
 * My GAL programmer (Wellon VP-190) doesn't like JEDEC files
 * with bare newlines in them.
 */
static size_t WriteOutput(void* buf_, size_t size, size_t nmemb, FILE* out) {
    unsigned char* buf = (unsigned char*)buf_;
    size_t i;

    for (i = 0; i < size * nmemb; i++) {
        unsigned char byte = buf[i];
        if (byte == '\n') {
            fwrite("\r\n", 1, 2, out);
        } else {
            fwrite(&byte, 1, 1, out);
        }
    }

    return nmemb;
}
