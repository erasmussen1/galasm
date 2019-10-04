#include <iostream>
#include <string>

#include <getopt.h>

#include <galasm/galasm2.h>

#include "ProjectVersion.h"


struct Options {
    std::string filename;
    bool showVersion{false};
    bool showUsage{false};
    bool showOptions{false};

    bool GenFuse{true};
    bool GenChip{true};
    bool GenPin{true};
    bool JedecSecBit{false};
    bool JedecFuseChk{false};

    void printOptions(void) {
        std::cout << "PLD filename: " << filename << "\n";
    }
};

static void printVersion(void) {
    std::cout << "GALasm2, Portable GAL Assembler\n"
                 "Original sources Copyright (c) 1991-96 Christian Habermann\n"
                 "  Version " Package_VERSION_MAJOR "." Package_VERSION_MINOR "." Package_VERSION_PATCH "\n"
                 "  Branch  " GIT_BRANCH
                 "\n"
                 "  Build date: " __DATE__ " " __TIME__
                 "\n"
                 "\n";
}

static void printUsage(void) {
    std::cout << "Usage:\n"
                 "GALasm [-scfpa] -i <filename>\n"
                 "\n"
                 "    -s Enable security fuse\n"
                 "    -c Do not create the .chp file\n"
                 "    -f Do not create the .fus file\n"
                 "    -p Do not create the .pin file\n"
                 "    -a Restrict checksum to the fuse array only\n\n";
}

static void parseCmdline(int argc, char** argv, Options& opts) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "fFcCpPsSaAi:ovhu")) != -1) {
        switch (opt) {
            case 'f':
            case 'F':
                opts.GenFuse = false;
                break;
            case 'c':
            case 'C':
                opts.GenChip = false;
                break;
            case 'p':
            case 'P':
                opts.GenPin = false;
                break;
            case 's':
            case 'S':
                opts.JedecSecBit = true;
                break;
            case 'a':
            case 'A':
                opts.JedecFuseChk = true;
                break;
            case 'i':
                opts.filename = std::string(optarg);
                break;
            case 'o':
                opts.showOptions = true;
                break;
            case 'v':
                opts.showVersion = true;
                break;
            case 'h':
            case 'u':
            default:
                opts.showUsage = true;
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    Options opts;

    parseCmdline(argc, argv, opts);

    if (opts.showOptions) {
       opts.printOptions();
    }

    if (opts.showVersion) {
        printVersion();
    }

    if (opts.showUsage) {
        printUsage();
        return -1;
    }

    int rc = assemblePldFile(opts.filename.c_str(),
                             (int)opts.GenFuse,
                             (int)opts.GenChip,
                             (int)opts.GenPin,
                             (int)opts.JedecSecBit,
                             (int)opts.JedecFuseChk);
    if (rc) {
        std::cout << "Assembling failed.\n";
        return rc;
    }

    std::cout << "Assembling successfully completed.\n";
    return 0;
}

