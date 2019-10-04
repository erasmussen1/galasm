#include <iostream>
#include <string>

// #include <galsam/galasm.h>

#include "ProjectVersion.h"


void version() {
    std::cout << GIT_BRANCH " " GIT_COMMIT_HASH "\n"
              << "Version " Package_VERSION_MAJOR "." Package_VERSION_MINOR
                 "." Package_VERSION_PATCH "\n\n";
}

int main(int argc, char* argv[]) {
    version();

    std::cout << "Hello world!\n";

    return 0;
}

#if 0
int main(int argc, char* argv[]) {
    int rc;
    char* p;

    struct Config cfg;

    cfg.GenFuse = TRUE;
    cfg.GenChip = TRUE;
    cfg.GenPin = TRUE;
    cfg.JedecSecBit = FALSE;
    cfg.JedecFuseChk = FALSE;

    p = argv[1];

    printf(
        "GALasm 2.1, Portable GAL Assembler\n"
        "Copyright (c) 1998-2003 Alessandro Zummo. All Rights Reserved\n"
        "Original sources Copyright (c) 1991-96 Christian Habermann\n\n");

    while (argc > 1 && (p[0] == '-' || (isalpha(p[1]) && (argc != 2)))) {
        switch (p[1]) {
            case 's':
            case 'S':
                cfg.JedecSecBit = TRUE;
                break;

            case 'c':
            case 'C':
                cfg.GenChip = FALSE;
                break;

            case 'f':
            case 'F':
                cfg.GenFuse = FALSE;
                break;

            case 'p':
            case 'P':
                cfg.GenPin = FALSE;
                break;

            case 'a':
            case 'A':
                cfg.JedecFuseChk = TRUE;
                break;

            case 'h':
            case 'H':
            case '?':
                printf("Usage:\nGALasm [-scfpa] <filename>\n");
                printf(
                    "-s Enable security fuse\n"
                    "-c Do not create the .chp file\n"
                    "-f Do not create the .fus file\n"
                    "-p Do not create the .pin file\n"
                    "-a Restrict checksum to the fuse array only\n");
                return (0);

            case '-':
            case '\0':
                argc--;
                argv++;
                goto opt_done;

            default:
                goto usage;
        }


        if (!isalpha(p[2])) {
            argc--;
            argv++;

            p = argv[1];
        } else
            p++;
    }

opt_done:
    if (argc != 2) {
    usage:
        printf("Usage:\nGALasm [-scfpa] <filename>\n");
        printf("Type GALasm -h for help\n");
        return (5);
    }

    rc = AssemblePldFile(argv[1], &cfg);

    if (rc != 0)
        printf("Assembling failed.\n");
    else
        printf("Assembling successfully completed.\n");

    return (rc);
}
#endif
