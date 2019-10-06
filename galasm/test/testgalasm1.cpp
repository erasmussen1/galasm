#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <iterator>
#include <iostream>

#include "gtest/gtest.h"

#include "galasm/galasm2.h"


struct MatrixFixture : public testing::Test {
    MatrixFixture() {
        //
    }

    ~MatrixFixture() {
        //
    }

    static void readfileToVectore(const std::string& filename, std::vector<unsigned char>& vec) {
        std::ifstream file(filename);
        std::ostringstream ss;
        ss << file.rdbuf();

        const std::string& s = ss.str();
        vec.clear();

        std::copy(s.begin(), s.end(), std::back_inserter(vec));
    }
};


TEST(MatrixFixture, GenerateCounterPLD_new) {
    std::vector<unsigned char> v;
    MatrixFixture::readfileToVectore("data/Counter.pld", v);

    int rc = assemblePldMemory("Counter.pld", v.data(), v.size(), 0, 0, 0, 1, 1);
    EXPECT_TRUE(!rc) ;

    std::vector<unsigned char> generatedOutput;
    MatrixFixture::readfileToVectore("Cou.jed", generatedOutput);

    std::vector<unsigned char> expectedOutput;
    MatrixFixture::readfileToVectore("data/Counter.jed", expectedOutput);

    bool result = std::equal(expectedOutput.begin(), expectedOutput.end(), generatedOutput.begin());

    if (result) {
        std::cout << "Both vectors are equal\n";
    } else {
        std::cout << "Both vectors are not equal\n";
    }

    for(auto & x : expectedOutput) {
        std::cout << x;
    }

    std::cout << "=================================";
    for(auto & x : generatedOutput) {
        std::cout << x;
    }
}

