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

    bool GenFuse{false};
    bool GenChip{false};
    bool GenPin{false};
    bool JedecSecBit{false};
    bool JedecFuseChk{false};
    bool verbose{false};
    bool unixNewline{false};

    void printOptions(void) {
        std::cout << "PLD filename: " << filename << "\n";
    }
};


static void printVersion(void) {
    std::cout << PROJECT_NAME
        ", Portable GAL Assembler\n"
        "Original sources base of Christian Habermann's code\n"
        "  Version " PROJECT_VERSION "." GIT_COMMIT_HASH
        "\n"
        "  Branch  " GIT_BRANCH
        "\n"
        "  Build date: " __DATE__ " " __TIME__
        "\n"
        "\n";
}

static void printUsage(void) {
    std::cout << "Usage:\n"
                 "GALasm [FCPSAoxVvhu] -f <filename>\n"
                 "\n"
                 "    -S   Enable security fuse\n"
                 "    -C   Generate .chp file\n"
                 "    -F   Generate .fus file\n"
                 "    -P   Generate the .pin file\n"
                 "    -A   Restrict checksum to the fuse array only\n"
                 "    -x   Unix style newline (jed file)\n"
                 "    -V   Verbose output\n"
                 "    -v   Show version\n"
                 "    -h   Show help menu\n"
                 "    -f   PLD input filename\n"
                 "\n\n";
}

static void parseCmdline(int argc, char** argv, Options& opts) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "FCPSAf:oxVvhu")) != -1) {
        switch (opt) {
            case 'F':
                opts.GenFuse = true;
                break;
            case 'C':
                opts.GenChip = true;
                break;
            case 'P':
                opts.GenPin = true;
                break;
            case 'S':
                opts.JedecSecBit = true;
                break;
            case 'A':
                opts.JedecFuseChk = true;
                break;
            case 'f':
                opts.filename = std::string(optarg);
                break;
            case 'o':
                opts.showOptions = true;
                break;
            case 'x':
                opts.unixNewline = true;
                break;
            case 'V':
                opts.verbose = true;
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

    if (opts.filename.empty()) {
        return -1;
    }

    int rc = assemblePldFile(opts.filename.c_str(),
                             (int)opts.GenFuse,
                             (int)opts.GenChip,
                             (int)opts.GenPin,
                             (int)opts.JedecSecBit,
                             (int)opts.JedecFuseChk,
                             (int)opts.verbose,
                             (int)opts.unixNewline);
    if (rc) {
        std::cout << "Assembling failed.\n";
        return rc;
    }

    std::cout << "Assembling successfully completed.\n";
    return 0;
}
