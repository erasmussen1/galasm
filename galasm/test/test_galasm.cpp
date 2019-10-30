#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <iterator>
#include <iostream>

#include "gtest/gtest.h"

#include "galasm/galasm2.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/galasm_types.h"
#include "../src/galasm.h"
#include "../src/support.h"

#ifdef __cplusplus
}
#endif


template <typename InputIterator1, typename InputIterator2>
bool rangeEqual(InputIterator1 first1,
                InputIterator1 last1,
                InputIterator2 first2,
                InputIterator2 last2) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 != *first2)
            return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

struct GalasmFixture : public testing::Test {
    GalasmFixture() {
        cfg.verbose = false;
        cfg.unixNewline = false;
        cfg.GenFuse = 1;
        cfg.GenChip = 1;
        cfg.GenPin = 1;
        cfg.JedecSecBit = 0;
        cfg.JedecFuseChk = 0;
    }

    ~GalasmFixture() {
        //
    }

    static int fileSize(const char* filename) {
        FILE* fp = fopen(filename, "r");
        if (!fp) {
            return -1;
        }

        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);

        fclose(fp);

        return size;
    }

    static bool readFileToBuffer(const char* filename, int filesize, unsigned char* filebuff) {
        FILE* fp = fopen(filename, "r");
        if (!fp) {
            return false;
        }

        int actlen = fread(filebuff, 1, filesize, fp);
        fclose(fp);

        return (actlen == filesize);
    }

    static bool compareTwoFiles(const std::string& filename1, const std::string& filename2) {
        std::ifstream file1(filename1);
        std::ifstream file2(filename2);

        std::istreambuf_iterator<char> begin1(file1);
        std::istreambuf_iterator<char> begin2(file2);

        std::istreambuf_iterator<char> end;

        return rangeEqual(begin1, end, begin2, end);
    }

    Config_t cfg;
    int N{0};
    bool res{false};
    int rc{0};
};

TEST_F(GalasmFixture, Filenames) {
    char* inFilename = strdup("GAL22V10.pld");
    char* outFilename = strdup("GAL22V10.xxx          ");

    int rc = GetBaseName(inFilename, "jed", outFilename);
    EXPECT_TRUE(!rc);

    EXPECT_EQ("GAL22V10.jed", std::string(outFilename));

    free(inFilename);
    free(outFilename);
}

TEST_F(GalasmFixture, GenerateCounter_PLD) {
    N = fileSize("Counter.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("Counter.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = AssemblePldFile("Counter_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Counter.jed", "Counter_out.jed"));
    EXPECT_TRUE(compareTwoFiles("Counter.chp", "Counter_out.chp"));
    EXPECT_TRUE(compareTwoFiles("Counter.fus", "Counter_out.fus"));
    EXPECT_TRUE(compareTwoFiles("Counter.pin", "Counter_out.pin"));
}

TEST_F(GalasmFixture, GAL20RA10_PLD) {
    N = fileSize("GAL20RA10.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("GAL20RA10.pld", N, v.data());

    rc = AssemblePldFile("GAL20RA10_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("GAL20RA10.jed", "GAL20RA10_out.jed"));
    EXPECT_TRUE(compareTwoFiles("GAL20RA10.chp", "GAL20RA10_out.chp"));
    EXPECT_TRUE(compareTwoFiles("GAL20RA10.fus", "GAL20RA10_out.fus"));
    EXPECT_TRUE(compareTwoFiles("GAL20RA10.pin", "GAL20RA10_out.pin"));
}

TEST_F(GalasmFixture, GAL22V10_PLD) {
    N = fileSize("GAL22V10.pld");

    std::vector<unsigned char> v(N + 10, 0x00);
    res = readFileToBuffer("GAL22V10.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = AssemblePldFile("GAL22V10_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("GAL22V10.jed", "GAL22V10_out.jed"));
    EXPECT_TRUE(compareTwoFiles("GAL22V10.chp", "GAL22V10_out.chp"));
    EXPECT_TRUE(compareTwoFiles("GAL22V10.fus", "GAL22V10_out.fus"));
    EXPECT_TRUE(compareTwoFiles("GAL22V10.pin", "GAL22V10_out.pin"));
}

TEST_F(GalasmFixture, Gatter_PLD) {
    N = fileSize("Gatter.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("Gatter.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = AssemblePldFile("Gatter_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Gatter.jed", "Gatter_out.jed"));
    EXPECT_TRUE(compareTwoFiles("Gatter.chp", "Gatter_out.chp"));
    EXPECT_TRUE(compareTwoFiles("Gatter.fus", "Gatter_out.fus"));
    EXPECT_TRUE(compareTwoFiles("Gatter.pin", "Gatter_out.pin"));
}

TEST_F(GalasmFixture, Tristate_PLD) {
    N = fileSize("Tristate.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("Tristate.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = AssemblePldFile("Tristate_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Tristate.jed", "Tristate_out.jed"));
    EXPECT_TRUE(compareTwoFiles("Tristate.chp", "Tristate_out.chp"));
    EXPECT_TRUE(compareTwoFiles("Tristate.fus", "Tristate_out.fus"));
    EXPECT_TRUE(compareTwoFiles("Tristate.pin", "Tristate_out.pin"));
}

TEST_F(GalasmFixture, SevenSegment_PLD) {
    N = fileSize("7seg.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("7seg.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = AssemblePldFile("7seg_out.pld", v.data(), v.size(), &cfg);
    EXPECT_TRUE(!rc);
}
