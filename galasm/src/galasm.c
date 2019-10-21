#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jedec.h"
#include "galasm.h"


#define SUFFIX_NON 0 /* possible suffixes */
#define SUFFIX_T 1
#define SUFFIX_R 2
#define SUFFIX_E 3
#define SUFFIX_CLK 4
#define SUFFIX_APRST 5
#define SUFFIX_ARST 6

#define DUMMY_OLMC11 24
#define DUMMY_OLMC12 25


/******************************** variables **********************************/
/* Diese Arrays geben an, in welche Spalte der ent-  */
/* sprechende Pin eingekoppelt (bzw. rückgekoppelt)  */
/* wird. Für die invertierende Einkopplung ist 1 zu  */
/* addieren, um die entsprechende Spalte zu erhalten */
/* -1 heißt: keine Einkopplung auf Matrix vorhanden  */


/* A possible translation (courtesy of Babelfish)
 * These arrays indicate, into which column the appropriate pin
 * becomes linked (and/or jerk-coupled). For inverting linking
 * is to be added 1, in order to receive the appropriate column -1 hei_t
 * : no linking on matrix available.
 */

/* Possible interpretation (by me)
 * These arrays maps the pins to the fuse matrix. Each integer
 * represents the number of the linked column or -1 if that pin
 * dosn't link to any column . For inverted signals, the right column
 * number can be calculated by adding one to the given number.
 */

/* GAL16V8 */
int PinToFuse16Mode1[20] = {2,  0,  4,  8,  12, 16, 20, 24, 28, -1,
                            30, 26, 22, 18, -1, -1, 14, 10, 6,  -1};

int PinToFuse16Mode2[20] = {2,  0,  4,  8,  12, 16, 20, 24, 28, -1,
                            30, -1, 26, 22, 18, 14, 10, 6,  -1, -1};

int PinToFuse16Mode3[20] = {-1, 0,  4,  8,  12, 16, 20, 24, 28, -1,
                            -1, 30, 26, 22, 18, 14, 10, 6,  2,  -1};

/* GAL20V8 */
int PinToFuse20Mode1[24] = {2,  0,  4,  8,  12, 16, 20, 24, 28, 32, 36, -1,
                            38, 34, 30, 26, 22, -1, -1, 18, 14, 10, 6,  -1};

int PinToFuse20Mode2[24] = {2,  0,  4,  8,  12, 16, 20, 24, 28, 32, 36, -1,
                            38, 34, -1, 30, 26, 22, 18, 14, 10, -1, 6,  -1};

int PinToFuse20Mode3[24] = {-1, 0,  4,  8,  12, 16, 20, 24, 28, 32, 36, -1,
                            -1, 38, 34, 30, 26, 22, 18, 14, 10, 6,  2,  -1};

/* GAL22V10 */
int PinToFuse22V10[24] = {0,  4,  8,  12, 16, 20, 24, 28, 32, 36, 40, -1,
                          42, 38, 34, 30, 26, 22, 18, 14, 10, 6,  2,  -1};


/* GAL20RA10 */
int PinToFuse20RA10[24] = {-1, 0,  4,  8,  12, 16, 20, 24, 28, 32, 36, -1,
                           -1, 38, 34, 30, 26, 22, 18, 14, 10, 6,  2,  -1};


/* These arrays show which row is connected to which OLMC */
int ToOLMC[8] = {56, 48, 40, 32, 24, 16, 8, 0};

int ToOLMC22V10[12] = {122, 111, 98, 83, 66, 49, 34, 21, 10, 1, 0, 131};

int ToOLMC20RA10[10] = {72, 64, 56, 48, 40, 32, 24, 16, 8, 0};

/* this array shows the size of the */
/* 22V10-OLMCs ( AR and SP = 1 )    */

int OLMCSize22V10[12] = {9, 11, 13, 15, 17, 17, 15, 13, 11, 9, 1, 1};

/* The last two entries of the 22V10 arrays are for the   */
/* AR and SP rows of the 22V10 GAL. This rows are not     */
/* connected to an OLMC. But to keep the assembler as     */
/* simple as possible, I introduced  two "OLMCs" for the  */
/* AR and SP. The assembler treads them like real OLMCs.  */
/* So don't become confused when OLMC number 11 and 12    */
/* are used as inputs, outputs, pins... by the assembler. */
/* These two OLMCs are just dummy-OLMCs.                  */

UBYTE PinDecNeg[24];
UBYTE ModeErrorStr[] = "Mode  x:  Pin xx";
// uint8_t* pinnames;
int modus;
int linenum;
UBYTE* actptr;
UBYTE* buffend;

JedecStruct_t Jedec;
Pin_t actPin;
GAL_OLMC_t OLMC[12];


int GetBaseName(const char* filename, const char* ext, char* newfilename) {
    strcpy(newfilename, filename);

    char* base = newfilename;

    newfilename = base + strlen(filename);

    while (newfilename > base && *newfilename != '.') {
        --newfilename;
    }

    newfilename++;
    for (int i = 0; ext[i] != '\0'; i++) {
        *newfilename++ = ext[i];
    }
    *newfilename = '\0';

    newfilename = base;

    return 0;
}

static void outputToFiles(const char* file, JedecStruct_t* jedec, Config_t* cfg) {
    char* jedFilename = strdup(file);

    int rc = GetBaseName(file, "jed", jedFilename);
    if (!rc) {
        WriteJedecFile(jedFilename, jedec, cfg);
    }
    free(jedFilename);

    if (cfg->GenFuse) {
        char* fusFilename = strdup(file);
        int rc = GetBaseName(file, "fus", fusFilename);
        if (!rc) {
            WriteFuseFile(fusFilename, jedec, cfg);
        }
        free(fusFilename);
    }

    if (cfg->GenPin) {
        char* pinFilename = strdup(file);
        int rc = GetBaseName(file, "pin", pinFilename);
        if (!rc) {
            WritePinFile(pinFilename, cfg);
        }
        free(pinFilename);
    }

    if (cfg->GenChip) {
        char* chpFilename = strdup(file);
        int rc = GetBaseName(file, "chp", chpFilename);
        if (!rc) {
            WriteChipFile(chpFilename, cfg);
        }
        free(chpFilename);
    }
}

static int getOlmcPinNumber(const int gal_type, const uint8_t pinNumber) {
    int n = -1;

    switch (gal_type) {
        case GAL16V8:
            n = pinNumber - 12;
            break;

        case GAL20V8:
            n = pinNumber - 15;
            break;

        case GAL22V10:
        case GAL20RA10:
            n = pinNumber - 14;
            break;

        default:
            break;
    }

    return n;
}

/**
 * get OLMC number
 */
static int getOLMCnumber(Config_t* cfg, Pin_t* actPin) {
    return getOlmcPinNumber(cfg->gal_type, actPin->p_Pin);
}

static int isEndOfChipNameBlank(unsigned char* ch, long offset) {
    if ((*(ch + offset) != ' ') && (*(ch + offset) != '\n') && (*(ch + offset) != '\t')) {
        return (-1);
    }

    return 0;
}

/**
 * Only ' ', newline and tab are valid after the GAL's name
 * Any other char will produce an error
 */
static int getGalTypeFromBuffer(unsigned char* actptr, Config_t* cfg) {

    cfg->num_of_olmcs = 0;
    cfg->num_of_pins = 0;
    cfg->num_of_col = 0;
    cfg->gal_type = UNKNOWN;
    strcpy(cfg->name, "UNKNOWN");

    if (strncmp((char*)actptr, "GAL16V8", (size_t)7) == 0) {
        cfg->num_of_olmcs = 8;
        cfg->num_of_pins = 20;
        cfg->num_of_col = MAX_FUSE_ADR16 + 1;
        cfg->gal_type = GAL16V8;
        strcpy(cfg->name, "GAL16V8");

        return isEndOfChipNameBlank(actptr, strlen(cfg->name));
    }

    if (strncmp((char*)actptr, "GAL20V8", (size_t)7) == 0) {
        cfg->num_of_olmcs = 8;
        cfg->num_of_pins = 24;
        cfg->num_of_col = MAX_FUSE_ADR20 + 1;
        cfg->gal_type = GAL20V8;
        strcpy(cfg->name, "GAL20V8");

        return isEndOfChipNameBlank(actptr, strlen(cfg->name));
    }

    if (strncmp((char*)actptr, "GAL20RA10", (size_t)9) == 0) {
        cfg->num_of_olmcs = 10;
        cfg->num_of_pins = 24;
        cfg->num_of_col = MAX_FUSE_ADR20RA10 + 1;
        cfg->gal_type = GAL20RA10;
        strcpy(cfg->name, "GAL20RA10");

        return isEndOfChipNameBlank(actptr, strlen(cfg->name));
    }

    if (strncmp((char*)actptr, "GAL22V10", (size_t)8) == 0) {
        cfg->num_of_olmcs = 10;
        cfg->num_of_pins = 24;
        cfg->num_of_col = MAX_FUSE_ADR22V10 + 1;
        cfg->gal_type = GAL22V10;
        strcpy(cfg->name, "GAL22V10");

        return isEndOfChipNameBlank(actptr, strlen(cfg->name));
    }

    return -1;
}

/**
 * Clear OLMC structure
 */
static void clearOLMC(GAL_OLMC_t* olmc) {
    for (int n = 0; n < 12; n++) {
        olmc[n].Active = 0;
        olmc[n].PinType = 0;
        olmc[n].TriCon = 0;
        olmc[n].Clock = 0;
        olmc[n].ARST = 0;
        olmc[n].APRST = 0;
        olmc[n].FeedBack = NO;
    }
}

static void initPins(uint8_t* pins) {
    for (int n = 0; n < 24; n++) {
        pins[n] = 0;
    }
}

static void initPinNames(Config_t* cfg) {
    for (int i = 0; i < 32; i++) {
        cfg->name[i] = '\0';
    }

    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 10; ++j) {
            cfg->PinNames[i][j] = '\0';
        }
        cfg->PinNames[i][9] = '\0';
    }
}

#if 0
static void printPinNames(FILE* fp, Config_t* cfg) {
    fprintf(fp, "Device: %s\n", cfg->name);
    for (int i = 0; i < cfg->num_of_pins; i++) {
        fprintf(fp, "%d %s\n", i, cfg->PinNames[i]);
    }
}
#endif

/**
 * set unused CLK, ARST and APRST equal 0
 */
static int setNotUsedPins(GAL_OLMC_t* olmc, JedecStruct_t* jedec, Config_t* cfg) {

    for (int n = 0; n < cfg->num_of_olmcs; n++) {
        if (olmc[n].PinType == NOTUSED) {
            continue;
        }

        /* register output needs clock definition */
        if (olmc[n].PinType == REGOUT && !olmc[n].Clock) {
            AsmError(41, n + 14);
            return (-1);
        }

        if (!olmc[n].Clock) {
            int l = (ToOLMC20RA10[n] + 1) * cfg->num_of_col;

            for (int k = l; k < l + cfg->num_of_col; k++) {
                jedec->GALLogic[k] = 0;
            }
        }

        if (olmc[n].PinType == REGOUT) {
            if (!olmc[n].ARST) {
                int l = (ToOLMC20RA10[n] + 2) * cfg->num_of_col;

                for (int k = l; k < l + cfg->num_of_col; k++) {
                    jedec->GALLogic[k] = 0;
                }
            }

            if (!olmc[n].APRST) {
                int l = (ToOLMC20RA10[n] + 3) * cfg->num_of_col;

                for (int k = l; k < l + cfg->num_of_col; k++) {
                    jedec->GALLogic[k] = 0;
                }
            }
        }
    }

    return 0;
}

/**
 * get first row of the  OLMC and the
 * number of rows which are available
 */
static void getRowsOfOlmc(Config_t* cfg, int n, int* l, int* i) {
    switch (cfg->gal_type) {
        case GAL16V8:
        case GAL20V8:
            *l = ToOLMC[n];
            *i = 8;
            break;

        case GAL22V10:
            *l = ToOLMC22V10[n];
            *i = OLMCSize22V10[n];
            break;

        case GAL20RA10:
            *l = ToOLMC20RA10[n];
            *i = 8;
            break;
    }
}

/**
 * set fuse matrix of unused OLMCs and of OLMCs
 * which are programmed as input equal 0
 */
static int setFuseMatrixOfOlmc(GAL_OLMC_t* olmc, JedecStruct_t* jedec, Config_t* cfg) {

    for (int n = 0; n < cfg->num_of_olmcs; n++) {
        if (olmc[n].PinType == NOTUSED || olmc[n].PinType == INPUT) {
            int l = 0;
            int i = 0;

            getRowsOfOlmc(cfg, n, &l, &i);

            l = l * cfg->num_of_col;
            int m = l + i * cfg->num_of_col;

            for (int k = l; k < m; k++) {
                jedec->GALLogic[k] = 0;
            }
        }
    }

    if (cfg->gal_type == GAL22V10) {
        if (!olmc[10].PinType) {
            for (int n = 0; n < cfg->num_of_col; n++) {
                jedec->GALLogic[n] = 0;
            }
        }

        /* set row of SP equal 0 */
        if (!olmc[11].PinType) {
            for (int n = 5764; n < 5764 + cfg->num_of_col; n++) {
                jedec->GALLogic[n] = 0;
            }
        }
    }

    if (cfg->gal_type == GAL20RA10) {
        int rc = setNotUsedPins(olmc, jedec, cfg);
        if (rc) {
            return -1;
        }
    }

    return 0;
}

static void getRowOffset(Config_t* cfg,
                         GAL_OLMC_t* olmc,
                         const int actOLMC,
                         const int modus,
                         const int suffix,
                         int* row_offset) {
    switch (cfg->gal_type) {
        case GAL16V8:
        case GAL20V8:
            if (suffix == SUFFIX_E) /* when tristate control use */
                *row_offset = 0;    /* first row (=> offset = 0) */
            else if (!*row_offset)  /*is offset of rows still equal 0?*/
                if (modus != MODE1 && olmc[actOLMC].PinType != REGOUT)
                    *row_offset = 1; /* then init. row-offset */
            break;

        case GAL22V10:
            if (suffix == SUFFIX_E) /* enable is the first row of the OLMC */
                *row_offset = 0;
            else {
                if (actOLMC == 10 || actOLMC == 11)
                    *row_offset = 0;   /* AR, SP?, then no offset */
                else if (!*row_offset) /* output starts at the     */
                    *row_offset = 1;   /* second row => offset = 1 */
            }
            break;

        case GAL20RA10:
            switch (suffix) {
                case SUFFIX_E: /* enable is the first row of the OLMC */
                    *row_offset = 0;
                    break;

                case SUFFIX_CLK: /* Clock is the second row of the OLMC */
                    *row_offset = 1;
                    break;

                case SUFFIX_ARST: /* AReset is the third row of the OLMC */
                    *row_offset = 2;
                    break;

                case SUFFIX_APRST: /* APreset is the fourth row of the OLMC  */
                    *row_offset = 3;
                    break;

                default: /* output equation starts at the fifth row */
                    if (*row_offset <= 3)
                        *row_offset = 4;
            }
    }
}

/**
 * SetAND()
 *
 * input:   row         row in which the AND should be set
 *          pinnum      pin which should be ANDed
 *          negation    0: pin without negation
 *                      1: pin with negation sign (/)
 * output:  none
 *
 * remarks: sets an AND (=0) in the fuse matrix
 */
void SetAND(int row, int pinnum, int negation, JedecStruct_t* jedec, Config_t* cfg) {
    int column = 0;

    switch (cfg->gal_type) {
        case GAL16V8:
            if (modus == MODE1)
                column = PinToFuse16Mode1[pinnum - 1];
            if (modus == MODE2)
                column = PinToFuse16Mode2[pinnum - 1];
            if (modus == MODE3)
                column = PinToFuse16Mode3[pinnum - 1];
            break;

        case GAL20V8:
            if (modus == MODE1)
                column = PinToFuse20Mode1[pinnum - 1];
            if (modus == MODE2)
                column = PinToFuse20Mode2[pinnum - 1];
            if (modus == MODE3)
                column = PinToFuse20Mode3[pinnum - 1];
            break;

        case GAL22V10:
            column = PinToFuse22V10[pinnum - 1];

            /* is it a registered OLMC pin?   */
            /* yes, then correct the negation */
            if ((pinnum >= 14 && pinnum <= 23) && !jedec->GALS1[23 - pinnum]) {
                negation = negation ? 0 : 1;
            }
            break;

        case GAL20RA10:
            column = PinToFuse20RA10[pinnum - 1];
            break;
    }

    jedec->GALLogic[row * cfg->num_of_col + column + negation] = 0;
}

/**
 * GetNextLine()
 *
 * input:
 *   none
 *
 * output:
 *   0: line found, actptr points to this line
 *   1: end of file reached
 *
 * remarks:
 *   gets pointer to next line
 */
int GetNextLine(void) {
    while (1) {
        if (*actptr == '\n') {
            actptr++;
            linenum++;
            break;
        }

        if (actptr > buffend) {
            return (1);
        }

        actptr++;
    }

    return 0;
}

/*
 * GetNextChar()
 *
 * input:   none
 *
 * output:  0: character found, actptr points to it
 *          1: no character found
 *
 * remarks: searchs the next character which is no comment, space, TAB, LF
 */
int GetNextChar(void) {
    while (1) {
        switch (*actptr) {
            case '\n':
                actptr++;
                linenum++;
                break;

            case ' ':
            case '\t':
                actptr++;
                break;

            case ';':
                if (GetNextLine()) {
                    return (0);
                }
                break;

            default:
                /* was there a character? */
                if (*actptr > ' ' && *actptr <= '~') {
                    return (0);
                } else {
                    actptr++;
                }
        }

        if (actptr > buffend) {
            return (1);
        }
    }

    return 0;
}

/*
 * IsPinName()
 *
 * input:   *pinnames   pointer to the pinnames array
 *          numofpins   number of pins (20 or 24, depends on the type of GAL)
 *
 *  global: actptr          pointer to the first character of the pinname
 *          actPin.p_Pin:   number of pin or NC_PIN; 0: no pin
 *          actPin.p_Neg:   pinname with '/' = 1;  without '/' = 0
 *
 * output:  none
 *
 * remarks: This function tests whether actptr points to a pinname or not
 */
void IsPinName(UBYTE* pinnames, int numofpins) {
    actPin.p_Neg = 0; /* install structure for pin */
    actPin.p_Pin = 0;

    if (IsNEG(*actptr)) { /* negation? */
        actptr++;
        actPin.p_Neg = 1;
    }

    /* get length of pin name */
    int n = 0;
    UBYTE* oldactptr = actptr;
    while (isalpha(*actptr) || isdigit(*actptr)) {
        actptr++;
        n++;
    }

    if (!n) {
        return;
    }

    if ((n == 2) && !strncmp((char*)oldactptr, "NC", (size_t)2)) {
        actPin.p_Pin = NC_PIN; /* NC pin*/
        return;
    }

    /* examine whole list of pin names */
    int i = 0;
    for (int k = 0; k < numofpins; k++) {
        i = 0;

        if (IsNEG(*(pinnames + k * 10))) {
            i = 1;
        }

        /* are the string sizes equal? */
        if (n != (int)strlen((char*)(pinnames + k * 10 + i))) {
            continue;
        }

        /* yes, then compare these strings */
        if (!(strncmp((char*)oldactptr, (char*)(pinnames + k * 10 + i), (size_t)n))) {
            actPin.p_Pin = k + 1;
            break;
        }
    }
}



/******************************************************************************
** Is_AR_SP()
*******************************************************************************
** input:   *ptr    pointer to the first character of the pinname
**
** output:  none
**          global  actPin.p_Pin: 23: AR, 24: SP, 0: no AR, SP
**                  actPin.p_Neg: pinname with '/' = 1;  without '/' = 0
**
** remarks: This function tests whether actptr points to a AR or SP
******************************************************************************/
void Is_AR_SP(UBYTE* ptr) {
    /* install structure for pin */
    actPin.p_Neg = 0;
    actPin.p_Pin = 0;

    if (IsNEG(*ptr)) {
        ptr++;
        actPin.p_Neg = 1;
    }

    /* get length of pin name */
    UBYTE* oldptr = ptr;

    int n = 0;
    while (isalpha(*ptr) || isdigit(*ptr)) {
        ptr++;
        n++;
    }

    /* assign AR to "OLMC 11" ("pin 24") and */
    /* assign SP to "OLMC 12" ("pin 25")     */
    if (!n) {
        return;
    }

    if ((n == 2) && !strncmp((char*)oldptr, "AR", (size_t)2)) {
        actPin.p_Pin = DUMMY_OLMC11;
    }

    if ((n == 2) && !strncmp((char*)oldptr, "SP", (size_t)2)) {
        actPin.p_Pin = DUMMY_OLMC12;
    }
}


/******************************************************************************
** IsOR()
*******************************************************************************
** input:   none
**
** output:  1: chr is a OR
**          0: chr is no OR
**
** remarks: checks whether or not chr is a OR sign or not
******************************************************************************/
int IsOR(char chr) {
    if (chr == '+' || chr == '#') {
        return (1);
    }

    return (0);
}

/******************************************************************************
** IsAND()
*******************************************************************************
** input:   none
**
** output:  1: chr is a AND
**          0: chr is no AND
**
** remarks: checks whether or not chr is a AND sign or not
******************************************************************************/
int IsAND(char chr) {
    if (chr == '*' || chr == '&') {
        return (1);
    }

    return (0);
}

/******************************************************************************
** IsNEG()
*******************************************************************************
** input:   none
**
** output:  1: chr is a negation sign
**          0: chr is no negation sign
**
** remarks: checks whether or not chr is a negation sign or not
******************************************************************************/
int IsNEG(char chr) {
    if (chr == '/' || chr == '!') {
        return (1);
    }

    return (0);
}

/******************************************************************************
** WriteChipFile(char *filename, int gal_type)
*******************************************************************************
** input:   gal type
**			filename
**
** output:  none
**
** remarks: make chip file
******************************************************************************/
void WriteChipFile(char* filename, Config_t* cfg) {
    FILE* fp = fopen(filename, (char*)"w");
    if (!fp) {
        ErrorReq(13);
        return;
    }

    fprintf(fp, "\n\n");
    WriteSpaces(fp, 31);
    fprintf(fp, " %s\n\n", cfg->name);

    WriteSpaces(fp, 26);
    fprintf(fp, "-------\\___/-------\n");

    uint8_t* pinnames = &cfg->PinNames[0][0];

    for (int n = 0; n < cfg->num_of_pins / 2; n++) {
        WriteSpaces(fp, 25 - (int)strlen((char*)(pinnames + n * 10)));

        fprintf(fp,
                "%s | %2d           %2d | %s\n",
                pinnames + n * 10,
                n + 1,
                cfg->num_of_pins - n,
                pinnames + (cfg->num_of_pins - n - 1) * 10);

        if (n < cfg->num_of_pins / 2 - 1) {
            WriteSpaces(fp, 26);
            fprintf(fp, "|                 |\n");
        }
    }

    WriteSpaces(fp, 26);
    fprintf(fp, "-------------------\n");

    if (fclose(fp) == EOF) {
        ErrorReq(8);
        return;
    }
}

void WritePinFile(char* filename, Config_t* cfg) {
    uint8_t* pinnames = &cfg->PinNames[0][0];
    int num_of_pins = cfg->num_of_pins;
    int gal_type = cfg->gal_type;

    FILE* fp = fopen(filename, (char*)"w");
    if (!fp) {
        ErrorReq(13);
        return;
    }

    fprintf(fp, "\n\n");
    fprintf(fp, " Pin # | Name     | Pin Type\n");
    fprintf(fp, "-----------------------------\n");

    int flag = 0;
    for (int n = 1; n <= num_of_pins; n++) {
        fprintf(fp, "  %2d   | ", n);
        fprintf(fp, "%s", pinnames + (n - 1) * 10);
        WriteSpaces(fp, 9 - (int)strlen((char*)(pinnames + (n - 1) * 10)));

        flag = 0;
        if (n == num_of_pins / 2) {
            fprintf(fp, "| GND\n");
            flag = 1;
        }

        if (n == num_of_pins) {
            fprintf(fp, "| VCC\n\n");
            flag = 1;
        }

        if (gal_type == GAL16V8 || gal_type == GAL20V8) {
            if (modus == MODE3 && n == 1) {
                fprintf(fp, "| Clock\n");
                flag = 1;
            }

            if (modus == MODE3) {
                if (gal_type == GAL16V8 && n == 11) {
                    fprintf(fp, "| /OE\n");
                    flag = 1;
                }

                if (gal_type == GAL20V8 && n == 13) {
                    fprintf(fp, "| /OE\n");
                    flag = 1;
                }
            }
        }

        if (gal_type == GAL22V10 && n == 1) {
            fprintf(fp, "| Clock/Input\n");
            flag = 1;
        }

        if ((gal_type == GAL16V8 && n >= 12 && n <= 19) ||
            (gal_type == GAL20V8 && n >= 15 && n <= 22) ||
            (gal_type == GAL20RA10 && n >= 14 && n <= 23) ||
            (gal_type == GAL22V10 && n >= 14 && n <= 23)) {

            // n is pinNumber
            int k = getOlmcPinNumber(gal_type, n);

            if (OLMC[k].PinType != INPUT)
                if (OLMC[k].PinType)
                    fprintf(fp, "| Output\n");
                else
                    fprintf(fp, "| NC\n");
            else
                fprintf(fp, "| Input\n");
        } else {
            if (!flag)
                fprintf(fp, "| Input\n");
        }
    }

    if (fclose(fp) == EOF) {
        ErrorReq(8);
        return;
    }
}

/******************************************************************************
** WriteRow()
*******************************************************************************
** input:   *fp     pointer to the file handle
**          row     number of row which should be written
**
** output:  none
**
** remarks: writes a row of an OLMC to the file characterized by the file
**          handle fp
******************************************************************************/
void WriteRow(FILE* fp, int row, int num_of_col) {
    fprintf(fp, "\n%3d ", row); /* print row number */

    for (int col = 0; col < num_of_col; col++) { /* print fuses of a row */
        if (!((col) % 4)) {
            fprintf(fp, " ");
        }

        if (Jedec.GALLogic[row * num_of_col + col]) {
            fprintf(fp, "-");
        } else {
            fprintf(fp, "x");
        }
    }
}

void WriteFuseFile(char* filename, JedecStruct_t* jedec, Config_t* cfg) {
    int gal_type = cfg->gal_type;
    int num_of_col = cfg->num_of_col;
    int numofOLMCs = cfg->num_of_olmcs;
    int pin = cfg->num_of_pins - 1;
    uint8_t* pinnames = &cfg->PinNames[0][0];

    {
        /*
         * This might be a bug in the original code.
         * (missing break in switch statments)
         *
         * Leave this but in, until it is confirmed
         * that it is a bug
         */
        num_of_col = MAX_FUSE_ADR22V10 + 1;
    }

    FILE* fp = fopen(filename, (char*)"w");
    if (!fp) {
        ErrorReq(13);
        return;
    }

    int row = 0;
    for (int olmc = 0; olmc < numofOLMCs; olmc++) {

        /* AR when 22V10 */
        if (gal_type == GAL22V10 && olmc == 0) {
            fprintf(fp, "\n\nAR");
            WriteRow(fp, row, num_of_col);
            row++;
        }

        fprintf(fp, "\n\nPin %2d = ", pin); /* print pin */
        fprintf(fp, "%s", pinnames + (pin - 1) * 10);

        WriteSpaces(fp, 13 - (int)strlen((char*)(pinnames + (pin - 1) * 10)));

        if (gal_type == GAL16V8)
            fprintf(fp, "XOR = %1d   AC1 = %1d", jedec->GALXOR[19 - pin], jedec->GALAC1[19 - pin]);
        else if (gal_type == GAL20V8)
            fprintf(fp, "XOR = %1d   AC1 = %1d", jedec->GALXOR[23 - pin], jedec->GALAC1[23 - pin]);
        else if (gal_type == GAL22V10)
            fprintf(fp, "S0 = %1d   S1 = %1d", jedec->GALXOR[23 - pin], jedec->GALAC1[23 - pin]);
        else if (gal_type == GAL20RA10)
            fprintf(fp, "S0 = %1d", jedec->GALXOR[23 - pin]);

        /* get number of rows of an OLMC */
        int numofrows = 8;
        if (gal_type == GAL22V10) {
            numofrows = OLMCSize22V10[olmc];
        }

        /* print all fuses of an OLMC */
        for (int n = 0; n < numofrows; n++) {
            WriteRow(fp, row, num_of_col);
            row++;
        }

        /* SP when 22V10 */
        if (gal_type == GAL22V10 && olmc == 9) {
            fprintf(fp, "\n\nSP");
            WriteRow(fp, row, num_of_col);
        }

        pin--;
    }

    fprintf(fp, "\n\n");

    if (fclose(fp) == EOF) {
        ErrorReq(8); /* can't close file */
        return;
    }
}

/******************************************************************************
** WriteSpaces()
*******************************************************************************
** input:   *fp         pointer to the file handle of the file
**          numof       number of spaces to be written to the file
**
** output:  none
**
** remarks: write "numof" spaces to the file characterized by *fp
******************************************************************************/
void WriteSpaces(FILE* fp, int numof) {
    for (int n = 0; n < numof; n++) {
        fprintf(fp, " ");
    }
}

/******************************************************************************
** AsmError()
*******************************************************************************
** input:   errornum    number of error to be printed
**          pinnum      = 0: print "Error in line linnum:" ...
**                      > 0: print "Pin pinnum:" ...
**
** output:  none
**
** remarks: print error messages of the GAL-assembler and free
**          the memory allocated by the file buffer
******************************************************************************/
void AsmError(int errornum, int pinnum) {
    if (!pinnum)
        printf("Error in line %d: ", linenum);
    else
        printf("Error, pin %d: ", pinnum);

    printf("%s\n", AsmErrorArray[errornum]);
}

/**
 * int AssemblePldFile(char *file)
 *
 * input:   file  The file to be assembled
 *
 * output:  0:    successful
 *          else: error
 *
 * remarks: This function does assemble a *.pld file.
 */
int AssemblePldFile(
    const char* file, unsigned char* fbuff, int fsize, Config_t* cfg, bool verbose) {
    UBYTE chr;
    UBYTE *bool_start, *oldptr;
    char prevOp;
    char suffix_strn[MAX_SUFFIX_SIZE];
    int i = 0, j, k, l = 0;
    int max_chr, pass, pin_num, bool_linenum;
    int actOLMC, row_offset, newline, oldline;
    int suffix, start_row, max_row;

    if (!fbuff) {
        ErrorReq(2);
        return -2;
    }

    actptr = fbuff;
    buffend = fbuff + fsize;
    linenum = 1;

    initPinNames(cfg);

    initJedec(&Jedec);

    clearOLMC(OLMC);

    int rc = getGalTypeFromBuffer(actptr, cfg);
    if (rc) {
        AsmError(1, 0);
        return rc;
    }

    /*** get the leading 8 bytes of the second line as signature ***/
    if (GetNextLine()) {
        AsmError(2, 0);
        return (-1);
    }

    /* store signature in the JEDEC structure */
    int n = 0;
    int m = 0;

    /* end of signature: after eight characters, CR or TAB */
    while ((*actptr != '\n') && (*actptr != '\t') && (n < 8)) {
        chr = *actptr;

        for (m = 0; m < 8; m++) {
            Jedec.GALSig[n * 8 + m] = (chr >> (7 - m)) & 0x1;
        }

        actptr++;
        n++;

        if (actptr > buffend) {
            AsmError(2, 0);
            return (-1);
        }
    }

    /*** get name of pins ***/
    /* clear flags for negations in the pin declaration */
    initPins(PinDecNeg);

    /* assembler: pin names in PinNames */
    uint8_t* pinnames = &cfg->PinNames[0][0];

    GetNextLine();

    for (n = 0; n < cfg->num_of_pins; n++) {
        if (GetNextChar()) {
            AsmError(2, 0);
            return (-1);
        }

        m = 0;
        chr = *actptr;

        /* is there a negation? */
        if (IsNEG(chr)) {
            max_chr = 10;
            PinDecNeg[n] = 1;
        } else {
            max_chr = 9;
        }

        if (!(isalpha(chr) || isdigit(chr) || IsNEG(chr))) {
            AsmError(5, 0);
            return (-1);
        }

        k = 0;
        while (isalpha(chr) || isdigit(chr) || IsNEG(chr)) {
            /* check position of '/' */
            if (IsNEG(chr) && k != 0) {
                AsmError(10, 0);
                return (-1);
            }

            k = 1;
            actptr++;

            if (IsNEG(chr) && (!(isalpha(*actptr) || isdigit(*actptr)))) {
                AsmError(3, 0);
                return (-1);
            }

            *(pinnames + n * 10 + m) = chr;
            m++;

            chr = *actptr;

            if (m == max_chr) {
                AsmError(4, 0);
                return (-1);
            }
        }
        *(pinnames + n * 10 + m) = '\0';

        /* pin name twice? */
        for (l = 0; l < n; l++) {
            if (strcmp((char*)pinnames + l * 10, "NC")) {
                i = j = 0;

                if (IsNEG(*(pinnames + l * 10))) { /* skip negation sign */
                    i = 1;
                }

                if (IsNEG(*(pinnames + n * 10))) {
                    j = 1;
                }

                if (!strcmp((char*)(pinnames + l * 10 + i), (char*)(pinnames + n * 10 + j))) {
                    AsmError(9, 0); /* pin name defined twice */
                    return (-1);
                }
            }
        }

        /* is GND at the GND-pin? */
        if (!strcmp((char*)(pinnames + n * 10), "GND")) {
            if (n + 1 != cfg->num_of_pins / 2) {
                AsmError(6, 0);
                return (-1);
            }
        }

        if (n + 1 == cfg->num_of_pins / 2) {
            if (strcmp((char*)(pinnames + n * 10), "GND")) {
                AsmError(8, 0);
                return (-1);
            }
        }

        /* is VCC at the VCC pin? */
        if (!strcmp((char*)(pinnames + n * 10), "VCC")) {
            if (n + 1 != cfg->num_of_pins) {
                AsmError(6, 0);
                return (-1);
            }
        }

        if (n + 1 == cfg->num_of_pins) {
            if (strcmp((char*)(pinnames + n * 10), "VCC")) {
                AsmError(7, 0);
                return (-1);
            }
        }

        /* AR and SP are key words for 22V10 */
        /* they are not allowed in the pin declaration */
        if (cfg->gal_type == GAL22V10) {
            if (!strcmp((char*)(pinnames + n * 10), "AR")) {
                AsmError(18, 0);
                return (-1);
            }

            if (!strcmp((char*)(pinnames + n * 10), "SP")) {
                AsmError(18, 0);
                return (-1);
            }
        }
    }

    // printPinNames(stdout, cfg);

    /*
     * Boolean Equations evaluate:
     *  The Boolean Equations is twice examined.
     *  With the first run the OLMC pins are evaluated and the OLMC
     *  structure is filled.
     *  With the help of this structure the correct mode (1, 2 or 3)
     *  will be calculated. With the second run the Fuse matrix is
     *  then provided.
     */
    if (GetNextChar()) {
        AsmError(2, 0);
        return (-1);
    }

    if (!strncmp((char*)actptr, "DESCRIPTION", (size_t)11)) {
        AsmError(33, 0);
        return (-1);
    }

    bool_start = actptr;    /* set pointer to the beginning of    */
    bool_linenum = linenum; /* the equations and save line number */

    /* this is a two-pass-assembler */
    for (pass = 0; pass < 2; pass++) {
        if (verbose) {
            printf("Assembler Phase %d for \"%s\"\n", (pass + 1), file);
        }

        /* 2nd pass? => make ACW and get the mode for 16V8, 20V8 GALs  */
        if (pass) {
            modus = 0;
            if (cfg->gal_type == GAL16V8 || cfg->gal_type == GAL20V8) {
                for (n = 0; n < 8; n++) {
                    /* is there a registered OLMC?, then GAL's mode is mode 3 */
                    if (OLMC[n].PinType == REGOUT) {
                        modus = MODE3;
                        setMode(&Jedec, modus);
                        break;
                    }
                }

                if (!modus) {
                    for (n = 0; n < 8; n++) {
                        /* is there a tristate OLMC?, then GAL's */
                        if (OLMC[n].PinType == TRIOUT) {
                            modus = MODE2;
                            setMode(&Jedec, modus);
                            break;
                        }
                    }
                }

                if (!modus) {
                    /* if there is a violation of mode 1, */
                    /* then use automatically mode 2      */
                    for (n = 0; n < 8; n++) {
                        if (OLMC[n].PinType == INPUT) {
                            if (cfg->gal_type == GAL16V8) {
                                pin_num = n + 12;

                                if (pin_num == 15 || pin_num == 16) {
                                    modus = MODE2;
                                    setMode(&Jedec, modus);
                                    break;
                                }
                            }

                            if (cfg->gal_type == GAL20V8) {
                                pin_num = n + 15;

                                if (pin_num == 18 || pin_num == 19) {
                                    modus = MODE2;
                                    setMode(&Jedec, modus);
                                    break;
                                }
                            }
                        }

                        if (OLMC[n].PinType == COM_TRI_OUT && OLMC[n].FeedBack) {
                            modus = MODE2;
                            setMode(&Jedec, modus);
                            break;
                        }
                    }
                }

                /* if there is still no mode defined, use mode 1 */
                if (!modus) {
                    modus = MODE1;
                    setMode(&Jedec, modus);
                }

                /* If GAL's mode is mode 1, use all OLMCs which type is   */
                /* not defined explicitly as combinational outputs.       */
                /* If GAL's mode is mode 2 or 3, use all OLMCs which type */
                /* is not defined explicitly as tristate output which is  */
                /* always enabled */

                for (n = 0; n < 8; n++) {
                    if (OLMC[n].PinType == COM_TRI_OUT) {
                        if (modus == MODE1)
                            OLMC[n].PinType = COMOUT;
                        else {
                            OLMC[n].PinType = TRIOUT;
                            OLMC[n].TriCon = TRI_VCC;
                        }
                    }
                }

                /* make ACW; (SYN and AC0 are defined already) */
                for (n = 0; n < PT_SIZE; n++) { /* set product term disable */
                    Jedec.GALPT[n] = 1;
                }

                /* get AC1 bits */
                for (n = 0; n < AC1_SIZE; n++) {
                    if (OLMC[n].PinType == INPUT || OLMC[n].PinType == TRIOUT) {
                        Jedec.GALAC1[AC1_SIZE - 1 - n] = 1;
                    }
                }

                /* get XOR bits */
                for (n = 0; n < XOR_SIZE; n++) {
                    if (((OLMC[n].PinType == COMOUT) || (OLMC[n].PinType == TRIOUT) ||
                         (OLMC[n].PinType == REGOUT)) &&
                        (OLMC[n].Active == ACTIVE_HIGH))
                        Jedec.GALXOR[XOR_SIZE - 1 - n] = 1;
                }
            }

            if (cfg->gal_type == GAL22V10) {
                for (n = 0; n < 10; n++) {
                    if (OLMC[n].PinType == COM_TRI_OUT) /* output can be */
                        OLMC[n].PinType = TRIOUT;       /* tristate or   */
                                                        /* register      */

                    if (((OLMC[n].PinType == COMOUT) || (OLMC[n].PinType == TRIOUT) ||
                         (OLMC[n].PinType == REGOUT)) &&
                        (OLMC[n].Active == ACTIVE_HIGH))
                        Jedec.GALXOR[9 - n] = 1;

                    /* get AC1 bits (S1) */
                    if (OLMC[n].PinType == INPUT || OLMC[n].PinType == TRIOUT)
                        Jedec.GALS1[9 - n] = 1;
                }
            }

            if (cfg->gal_type == GAL20RA10) {
                /* get XOR bits (S0) */
                for (n = 0; n < 10; n++) {
                    if (OLMC[n].PinType == COM_TRI_OUT) /* output can be */
                        OLMC[n].PinType = TRIOUT;       /* tristate or   */
                                                        /* register      */

                    if (((OLMC[n].PinType == COMOUT) || (OLMC[n].PinType == TRIOUT) ||
                         (OLMC[n].PinType == REGOUT)) &&
                        (OLMC[n].Active == ACTIVE_HIGH))
                        Jedec.GALXOR[9 - n] = 1;
                }
            }
        }

        if (pass && verbose) {
            printf("GAL %s; Operation mode %d; Security fuse %s\n",
                   cfg->name,
                   modus,
                   cfg->JedecSecBit ? "on" : "off");
        }

        actptr = bool_start;
        linenum = bool_linenum;
        newline = linenum;
        goto label1;

    loop1:
        if (GetNextChar()) {
            AsmError(2, 0);
            return (-1);
        }

        suffix = SUFFIX_NON;

        if (*actptr == '.') {
            actptr++;

            /* no suffix allowed at AR and SP */
            if (cfg->gal_type == GAL22V10 && (actPin.p_Pin == 24 || actPin.p_Pin == 25)) {
                AsmError(39, 0);
                return (-1);
            }

            /* copy suffix string into suffix array */
            n = 0;
            while (isalpha(*actptr)) {
                if (n < MAX_SUFFIX_SIZE)
                    suffix_strn[n++] = *actptr++;
                else {
                    AsmError(13, 0);
                    return (-1);
                }
            }
            suffix_strn[n] = '\0';

            if (suffix_strn[0] == 'T')
                suffix = SUFFIX_T;
            else if (suffix_strn[0] == 'R')
                suffix = SUFFIX_R;
            else if (suffix_strn[0] == 'E')
                suffix = SUFFIX_E;
            else if (!strcmp(&suffix_strn[0], "CLK"))
                suffix = SUFFIX_CLK;
            else if (!strcmp(&suffix_strn[0], "ARST"))
                suffix = SUFFIX_ARST;
            else if (!strcmp(&suffix_strn[0], "APRST"))
                suffix = SUFFIX_APRST;
            else {
                AsmError(13, 0);
                return (-1);
            }

            /* check whether suffix is allowed or not */
            if (cfg->gal_type != GAL20RA10) {
                switch (suffix) {
                    case SUFFIX_CLK:
                        AsmError(34, 0); /* no .CLK allowed */
                        return (-1);
                        break;

                    case SUFFIX_ARST:
                        AsmError(35, 0); /* .ARST is not allowed */
                        return (-1);
                        break;

                    case SUFFIX_APRST:
                        AsmError(36, 0); /* .APRST is not allowed */
                        return (-1);
                        break;
                }
            }

            if (GetNextChar()) {
                AsmError(2, 0);
                return (-1);
            }
        }

        /* save offset of OLMC */
        actOLMC = (int)actPin.p_Pin;
        if (cfg->gal_type == GAL16V8) {
            actOLMC -= 12;
        } else if (cfg->gal_type == GAL20V8) {
            actOLMC -= 15;
        } else {
            actOLMC -= 14;
        }

        /* offset for OR at OLMC previous operator */
        row_offset = 0;
        prevOp = 0;

        if (!pass) {
            /* is pin a OLMC pin? */
            if (((actPin.p_Pin >= 12) && (actPin.p_Pin <= 19)) ||
                ((actPin.p_Pin >= 15) && (actPin.p_Pin <= 22)) ||
                ((actPin.p_Pin >= 14) && (actPin.p_Pin <= DUMMY_OLMC12)) ||
                ((actPin.p_Pin >= 14) && (actPin.p_Pin <= 23))) {

                /* get OLMC number */
                n = getOLMCnumber(cfg, &actPin);

                switch (suffix) {
                    case SUFFIX_R: /* output definition */
                    case SUFFIX_T:
                    case SUFFIX_NON:
                        if (!OLMC[n].PinType || OLMC[n].PinType == INPUT) {
                            /* get pin's activation */
                            if (actPin.p_Neg) {
                                OLMC[n].Active = ACTIVE_LOW;
                            } else {
                                OLMC[n].Active = ACTIVE_HIGH;
                            }

                            if (suffix == SUFFIX_T) {
                                OLMC[n].PinType = TRIOUT;
                            }

                            if (suffix == SUFFIX_R) {
                                OLMC[n].PinType = REGOUT;
                            }

                            /* type of output is not defined explicitly  */
                            if (suffix == SUFFIX_NON) {
                                OLMC[n].PinType = COM_TRI_OUT;
                            }
                        } else {
                            /* AR or SP is defined twice */
                            if (cfg->gal_type == GAL22V10 && (n == 10 || n == 11)) {
                                AsmError(40, 0);
                                return (-1);
                            } else {
                                /* pin is defined twice as output */
                                AsmError(16, 0);
                                return (-1);
                            }
                        }
                        break;

                    case SUFFIX_E:
                        /* negation of the trisate control is not allowed */
                        if (actPin.p_Neg) {
                            AsmError(19, 0);
                            return (-1);
                        }

                        /* tri. control twice? yes, then error */
                        if (OLMC[n].TriCon) {
                            AsmError(22, 0);
                            return (-1);
                        }

                        /* set the flag that there is a tri. control equation */
                        OLMC[n].TriCon = TRICON;

                        /* the sequence must be output followed by the tri. control */
                        if (!OLMC[n].PinType || OLMC[n].PinType == INPUT) {
                            AsmError(17, 0);
                            return (-1);
                        }

                        /* GAL16V8/20V8: tristate control for reg. output is not allowed */
                        if (OLMC[n].PinType == REGOUT &&
                            (cfg->gal_type == GAL16V8 || cfg->gal_type == GAL20V8)) {
                            AsmError(23, 0);
                            return (-1);
                        }

                        /* no tristate .T? then error  */
                        if (OLMC[n].PinType == COM_TRI_OUT) {
                            AsmError(24, 0);
                            return (-1);
                        }
                        break;

                    case SUFFIX_CLK:
                        if (actPin.p_Neg) { /* negation of the .CLK control is not allowed */
                            AsmError(19, 0);
                            return (-1);
                        }

                        if (OLMC[n].PinType == NOTUSED) {
                            AsmError(42, 0); /* sequence must be: output */
                            return (-1);     /* def., .CLK definition */
                        }

                        if (OLMC[n].Clock) { /* is .CLK defined twice? */
                            AsmError(45, 0); /* yes, then error */
                            return (-1);
                        }

                        OLMC[n].Clock = 1; /* set flag that there is */
                                           /* a .CLK equation        */
                        if (OLMC[n].PinType != REGOUT) {
                            AsmError(48, 0); /* no .CLK allowed when     */
                            return (-1);     /* output is not registered */
                        }
                        break;

                    case SUFFIX_ARST:
                        if (actPin.p_Neg) {  /* negation of the .ARST  */
                            AsmError(19, 0); /* control is not allowed */
                            return (-1);
                        }

                        if (OLMC[n].PinType == NOTUSED) {
                            AsmError(43, 0); /* sequence must be: output */
                            return (-1);     /* def., .ARST definition   */
                        }

                        if (OLMC[n].ARST) {  /* is .ARST defined twice? */
                            AsmError(46, 0); /* yes, then error         */
                            return (-1);
                        }

                        OLMC[n].ARST = 1; /* set flag that there is */
                                          /* a .ARST equation       */
                        if (OLMC[n].PinType != REGOUT) {
                            AsmError(48, 0); /* no .CLK allowed when     */
                            return (-1);     /* output is not registered */
                        }
                        break;

                    case SUFFIX_APRST:
                        if (actPin.p_Neg) {  /* negation of the .APRST */
                            AsmError(19, 0); /* control is not allowed */
                            return (-1);
                        }

                        if (OLMC[n].PinType == NOTUSED) {
                            AsmError(44, 0); /* sequence must be: output */
                            return (-1);     /* def., .APRST definition  */
                        }

                        if (OLMC[n].APRST) { /* is .APRST defined twice? */
                            AsmError(47, 0); /* yes, then error          */
                            return (-1);
                        }

                        OLMC[n].APRST = 1; /* set flag that there is */
                                           /* a .APRST equation      */
                        if (OLMC[n].PinType != REGOUT) {
                            AsmError(48, 0); /* no .CLK allowed when     */
                            return (-1);     /* output is not registered */
                        }
                        break;
                }
            } else {
                AsmError(15, 0); /* pin can't be programmed */
                return (-1);     /* as output               */
            }
        }

        start_row = max_row = 0;

        switch (cfg->gal_type) { /* get first the row of the */
            case GAL16V8:        /* OLMC and the number of   */
            case GAL20V8:        /* rows which areavailable  */
                start_row = ToOLMC[actOLMC];
                max_row = 8;
                break;

            case GAL22V10:
                start_row = ToOLMC22V10[actOLMC];
                max_row = OLMCSize22V10[actOLMC];
                break;

            case GAL20RA10:
                start_row = ToOLMC20RA10[actOLMC];
                max_row = 8;
                break;
        }

        if (*actptr != '=') {
            AsmError(14, 0);
            return (-1);
        }

    loop2:
        actptr++;

        if (GetNextChar()) {
            AsmError(2, 0);
            return (-1);
        }

        oldptr = actptr; /* save pointer */

        IsPinName(pinnames, cfg->num_of_pins);

        /* AR and SP is not allowed in terms of an equation */
        if (cfg->gal_type == GAL22V10 && !actPin.p_Pin) {
            Is_AR_SP(oldptr);

            if (actPin.p_Pin) {
                AsmError(31, 0);
                return (-1);
            }
        }

        if (!actPin.p_Pin) {
            AsmError(11, 0);
            return (-1);
        }

        if (actPin.p_Pin == NC_PIN) { /* NC used as pin name? */
            AsmError(12, 0);          /* yes, then error */
            return (-1);
        }

        if (IsNEG(*(pinnames + (long)((actPin.p_Pin - 1) * 10)))) {
            actPin.p_Neg = !actPin.p_Neg; /* consider negation in the pin declartion */
        }

        oldline = linenum;

        if (GetNextChar()) {
            AsmError(2, 0);
            return (-1);
        }

        newline = linenum;
        linenum = oldline;

        /* 1st pass */
        if (!pass) {
            if (((cfg->gal_type == GAL16V8) && (actPin.p_Pin >= 12) && (actPin.p_Pin <= 19)) ||
                ((cfg->gal_type == GAL20V8) && (actPin.p_Pin >= 15) && (actPin.p_Pin <= 22)) ||
                ((cfg->gal_type == GAL22V10) && (actPin.p_Pin >= 14) &&
                 (actPin.p_Pin <= DUMMY_OLMC12)) ||
                ((cfg->gal_type == GAL20RA10) && (actPin.p_Pin >= 14) && (actPin.p_Pin <= 23))) {

                n = getOLMCnumber(cfg, &actPin);

                if (!OLMC[n].PinType) {
                    OLMC[n].PinType = INPUT;
                }

                OLMC[n].FeedBack = YES;
            }
        }

        /* in pass 2 we have to make the fuse matrix */
        if (pass) {
            /* get row offset */
            getRowOffset(cfg, OLMC, actOLMC, modus, suffix, &row_offset);

            pin_num = actPin.p_Pin;

            /* is there a valuation of GAL's mode? */
            if (cfg->gal_type == GAL16V8 || cfg->gal_type == GAL20V8) {

                /* valuation of mode 2? */
                if (modus == MODE2) {
                    if (cfg->gal_type == GAL16V8 && (pin_num == 12 || pin_num == 19)) {
                        AsmError(20, 0);
                        return (-1);
                    }

                    if (cfg->gal_type == GAL20V8 && (pin_num == 15 || pin_num == 22)) {
                        AsmError(21, 0);
                        return (-1);
                    }
                }

                /* valuation of mode 3? */
                if (modus == MODE3) {
                    if (cfg->gal_type == GAL16V8 && (pin_num == 1 || pin_num == 11)) {
                        AsmError(26, 0);
                        return (-1);
                    }

                    if (cfg->gal_type == GAL20V8 && (pin_num == 1 || pin_num == 13)) {
                        AsmError(27, 0);
                        return (-1);
                    }
                }
            }

            /* valuation of 20RA10? */
            if (cfg->gal_type == GAL20RA10) {
                if (pin_num == 1) { /* pin 1 is reserved for /PL (preload) */
                    AsmError(37, 0);
                    return (-1);
                }

                if (pin_num == 13) { /* pin 13 is reserved for */
                    AsmError(38, 0); /* /OE (output enable)    */
                    return (-1);
                }
            }

            /* if GND, set row equal 0 */
            if (pin_num == cfg->num_of_pins || pin_num == cfg->num_of_pins / 2) {

                /* /VCC and /GND are not allowed */
                if (actPin.p_Neg) {
                    AsmError(25, 0);
                    return (-1);
                }

                if (!prevOp && !IsAND(*actptr) && !IsOR(*actptr)) {
                    if (pin_num == cfg->num_of_pins / 2) {
                        m = (start_row + row_offset) * cfg->num_of_col;

                        /* set row equal 0 */
                        for (n = m; n < m + cfg->num_of_col; n++) {
                            Jedec.GALLogic[n] = 0;
                        }
                    }
                } else {
                    AsmError(28, 0);
                    return (-1);
                }
            } else {
                if (suffix == SUFFIX_E || suffix == SUFFIX_CLK || suffix == SUFFIX_ARST ||
                    suffix == SUFFIX_APRST ||
                    (cfg->gal_type == GAL22V10 && (actOLMC == 10 || actOLMC == 11))) {

                    if (IsOR(prevOp)) {  /* max. one product term   */
                        AsmError(29, 0); /* for CLK, ARST, APRST, E */
                        return (-1);     /* and 22V10: AR, SP       */
                    }

                    SetAND(start_row + row_offset, pin_num, actPin.p_Neg, &Jedec, cfg);
                } else {
                    /* OR operation? yes, then take the next row */
                    if (IsOR(prevOp)) {
                        row_offset++;

                        if (row_offset == max_row) { /* too many ORs?*/
                            AsmError(30, 0);
                            return (-1);
                        }
                    }
                    /* set ANDs */
                    SetAND(start_row + row_offset, pin_num, actPin.p_Neg, &Jedec, cfg);
                }
            }

            /* are there any more terms? */
            if (!IsOR(*actptr) && !IsAND(*actptr) && suffix != SUFFIX_E && suffix != SUFFIX_CLK &&
                suffix != SUFFIX_ARST && suffix != SUFFIX_APRST) {
                /* no?, then set unused */
                row_offset++; /* rows of the OLMX equal 0 */
                if (row_offset != max_row) {
                    m = (start_row + row_offset) * cfg->num_of_col;

                    for (n = m; n < m + (max_row - row_offset) * cfg->num_of_col; n++) {
                        Jedec.GALLogic[n] = 0;
                    }
                }
            }
        }

        linenum = newline;

        if (IsOR(*actptr) || IsAND(*actptr)) {
            prevOp = *actptr;
            goto loop2;
        }

        if (strncmp((char*)actptr, "DESCRIPTION", (size_t)11)) {

        label1:
            linenum = newline;
            oldptr = actptr;

            IsPinName(pinnames, cfg->num_of_pins);

            if (cfg->gal_type == GAL22V10 && !actPin.p_Pin) { /* no pin name? then  */
                Is_AR_SP(oldptr);                             /* check whether name */
                                                              /* is AR or SP        */
                if (actPin.p_Pin && actPin.p_Neg) {           /* but no negation of */
                    AsmError(32, 0);                          /* AR or SP           */
                    return (-1);
                }
            }

            if (!actPin.p_Pin) {
                AsmError(11, 0);
                return (-1);
            }

            if (actPin.p_Pin == NC_PIN) {
                AsmError(12, 0);
                return (-1);
            }

            if (IsNEG(*(pinnames + (long)((actPin.p_Pin - 1) * 10)))) {
                actPin.p_Neg = !actPin.p_Neg; /* negation at pin declaration */
            }

            goto loop1;
        }
    }

    rc = setFuseMatrixOfOlmc(OLMC, &Jedec, cfg);
    if (rc) {
        return -1;
    }

    /* Now the JEDEC structure is ready */
    outputToFiles(file, &Jedec, cfg);

    return 0;
}

