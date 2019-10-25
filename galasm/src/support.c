#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "localize.h"
#include "support.h"


/**
 * FileSize()
 *
 * input:   *filename   filename
 *
 * output:  -1   file does not exist
 *          -2   not enough memory for FileInfoBlock
 *          >0   length of the specified file
 *
 * remarks:
 * returns length of a file
 *
 */
int FileSize(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fclose(fp);

    return size;
}

/**
 * ReadFile()
 *
 * input:   *filename       name of file
 *          filesize        length of file [bytes]
 *          *filebuff       where to store the specified file
 *
 * output:  TRUE:  file loaded
 *          FALSE: failed to load
 *
 * remarks:
 *   loads the specified file into memory
 *
 */
bool ReadFile(const char* filename, int filesize, uint8_t* buffer) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }

    int actlen = fread(buffer, 1, filesize, fp);
    fclose(fp);

    if (actlen != filesize) {
        return false;
    }

    return true;
}

/**
 * The functions:
 *    AddByte(), AddString(), IncPointer(), DecPointer()
 *    and FreeBuffer() are useful for handling chained lists
 */

/**
 * AddByte()
 *
 * input:   *buff   pointer to buffer structure
 *          code    byte to add to the list
 *
 * output:  0:   o.k.
 *           !=0: error occured (e.g. not enough free memory)
 *
 * remarks: This function does set a byte in a buffer. If the end of the buffer
 * is reached, a new buffer is added to the chained list of buffers.
 */
int AddByte(ActBuffer_t* buff, uint8_t code) {

    /* is the current address within the buffer? */
    if ((buff->Entry) < (buff->BuffEnd)) {
        *buff->Entry++ = code; /* the fill in the byte if not, then add a new buffer */
        return 0;
    }

    Buffer_t* mybuff = (Buffer_t*)calloc(sizeof(Buffer_t), 1);
    if (!mybuff) {
        ErrorReq(2);
        return (-1);
    }

    buff->ThisBuff->Next = mybuff; /* new buffer is current buffer */
    mybuff->Prev = buff->ThisBuff; /* previous is old buffer       */
    buff->ThisBuff = mybuff;       /* current buffer is new buffer */
    buff->Entry = (uint8_t*)(&mybuff->Entries[0]);
    buff->BuffEnd = (uint8_t*)mybuff + (long)sizeof(Buffer_t);
    *buff->Entry++ = code;

    return (0);
}

/**
 * AddString()
 *
 * input:   *buff    pointer to buffer structure
 *          strnptr  pointer to string to be added to the buffer
 *
 * output:  0:   o.k.
 *          !=0: error occured (e.g. not enough free memory)
 *
 * remarks: This function does add a string to a buffer. If the end of the
 *          buffer is reached, a new buffer is added to the chained list of
 *          buffers.
 *
 */
int AddString(ActBuffer_t* buff, uint8_t* strnptr) {
    while (*strnptr) {
        if (AddByte(buff, *strnptr++))
            return (-1);
    }
    return (0);
}

/**
 * IncPointer()
 *
 * input:   *buff   pointer to buffer structure
 *
 * output:  none
 *
 * remarks:
 *   This function does increment the pointer of the current buffer
 *   structure. If the pointer points to the end of the buffer, the
 *   pointer will be set to the beginning of the next entry of the
 *   chained list.
 *
 *   ATTENTION: there is no test whether the absolut end of the chained
 *   list is reached or not!
 */
void IncPointer(ActBuffer_t* buff) {
    buff->Entry++;

    if (buff->Entry == buff->BuffEnd) {
        buff->ThisBuff = buff->ThisBuff->Next;
        buff->Entry = (uint8_t*)(&buff->ThisBuff->Entries[0]);
        buff->BuffEnd = (uint8_t*)buff->ThisBuff + (long)sizeof(Buffer_t);
    }
}

/**
 ** DecPointer()
 *
 * input:   *buff   pointer to buffer structure
 *
 * output:  none
 *
 * remarks:
 *   This function does decrement the pointer of the current buffer
 *   structure. If the pointer points to the start of the buffer, the
 *   pointer will be set to the beginning of the previous entry of the
 *   chained list.
 *
 *   ATTENTION: there is no test whether the absolut start of the chained
 *   list is reached or not!
 */
void DecPointer(ActBuffer_t* buff) {
    buff->Entry--;

    /* start of buffer reached? */
    if (buff->Entry < &buff->ThisBuff->Entries[0]) {
        buff->ThisBuff = buff->ThisBuff->Prev;
        buff->BuffEnd = (uint8_t*)buff->ThisBuff + (long)sizeof(Buffer_t);
        buff->Entry = (uint8_t*)((buff->BuffEnd) - 1L);
    }
}

/**
 * FreeBuffer()
 *
 * input:   *buff   pointer to buffer structure
 *
 * output:  none
 *
 * remarks:
 *    This function does free memory which is allocated
 *    by the chained list.
 *
 */
void FreeBuffer(Buffer_t* buff) {
    Buffer_t* nextbuff;

    while (buff) {
        nextbuff = buff->Next;

        free(buff);

        buff = nextbuff;
    }
}
