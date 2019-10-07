#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <iterator>
#include <iostream>

#include "gtest/gtest.h"

#include "galasm/galasm2.h"


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
};

TEST_F(GalasmFixture, GenerateCounter_PLD) {
    int N = fileSize("Counter.pld");

    std::vector<unsigned char> v(N, 0x00);
    int rc = readFileToBuffer("Counter.pld", N, v.data());

    rc = assemblePldMemory("Countero.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(rc >= 0) << rc << "\n";

    EXPECT_TRUE(compareTwoFiles("Counter.jed", "Countero.jed"));
}

TEST_F(GalasmFixture, GAL20RA10_PLD) {
    int N = fileSize("GAL20RA10.pld");

    std::vector<unsigned char> v(N, 0x00);
    int rc = readFileToBuffer("GAL20RA10.pld", N, v.data());

    rc = assemblePldMemory("X20RA10o.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(rc >= 0) << rc << "\n";

    EXPECT_TRUE(compareTwoFiles("GAL20RA10.jed", "X20RA10o.jed"));
}

TEST_F(GalasmFixture, GAL22V10_PLD) {
    int N = fileSize("GAL22V10.pld");

    std::vector<unsigned char> v(N, 0x00);
    int rc = readFileToBuffer("GAL22V10.pld", N, v.data());

    rc = assemblePldMemory("Y22V10o.pld", v.data(), v.size(), 0, 0, 0, 0, 0);
    EXPECT_TRUE(rc >= 0) << rc << "\n";

    EXPECT_TRUE(compareTwoFiles("GAL22V10.jed", "Y22.jed"));
}




//  GAL22V10.pld
//  Gatter.pld
//  Tristate.pld
//
