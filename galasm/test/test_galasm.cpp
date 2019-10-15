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

#include "../src/galasm.h"

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

bool compareTwoFiles(const std::string& filename1, const std::string& filename2) {
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    return rangeEqual(begin1, end, begin2, end);
}

int fileSize(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);

    fclose(fp);

    return size;
}

bool readFileToBuffer(const char* filename, int filesize, unsigned char* filebuff) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }

    int actlen = fread(filebuff, 1, filesize, fp);
    fclose(fp);

    return (actlen == filesize);
}

struct GalasmFixture : public testing::Test {
    GalasmFixture() {
        //
    }

    ~GalasmFixture() {
        //
    }

    int N{0};
    bool res{false};
    int rc{0};
};

TEST_F(GalasmFixture, GAL20RA10_PLD) {
    N = fileSize("GAL20RA10.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("GAL20RA10.pld", N, v.data());

    rc = assemblePldMemory("X20RA10_out.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("GAL20RA10.jed", "X20RA10_out.jed"));
}

TEST_F(GalasmFixture, GAL22V10_PLD) {
    N = fileSize("GAL22V10.pld");

    std::vector<unsigned char> v(N+10, 0x00);
    res  = readFileToBuffer("GAL22V10.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = assemblePldMemory("GAL22V10_out.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("GAL22V10.jed", "GAL22V10_out.jed"));
}

TEST_F(GalasmFixture, GenerateCounter_PLD) {
    N = fileSize("Counter.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("Counter.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = assemblePldMemory("Counter_out.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Counter.jed", "Counter_out.jed"));
}

TEST_F(GalasmFixture, Gatter_PLD) {
    N = fileSize("Gatter.pld");

    std::vector<unsigned char> v(N, 0x00);
    res  = readFileToBuffer("Gatter.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = assemblePldMemory("Gatter_out.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Gatter.jed", "Gatter_out.jed"));
}

TEST_F(GalasmFixture, Tristate_PLD) {
    N = fileSize("Tristate.pld");

    std::vector<unsigned char> v(N, 0x00);
    res = readFileToBuffer("Tristate.pld", N, v.data());
    EXPECT_TRUE(res);

    rc = assemblePldMemory("Tristate_out.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(!rc);

    EXPECT_TRUE(compareTwoFiles("Tristate.jed", "Tristate_out.jed"));
}

TEST_F(GalasmFixture, Filenames) {
    char *inFilename = strdup("GAL22V10.pld");
    char *outFilename = strdup("GAL22V10.xxx          ");

    int rc = GetBaseName(inFilename, "jed", outFilename);
    EXPECT_TRUE(!rc) ;

    EXPECT_EQ("GAL22V10.jed", std::string(outFilename));

    free(inFilename);
    free(outFilename);
}


