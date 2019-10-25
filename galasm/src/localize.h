#ifndef LOCALIZE_H
#define LOCALIZE_H

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

void ErrorReq(int errornum);

int AsmError(int errornum, int pinnum, int lineNumber);

#ifdef __cplusplus
}
#endif

#endif  // SUPPORT_H
