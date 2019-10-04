#!/bin/bash
#
#
#
set -o nounset
# set -x


WORKING_DIR="$(cd $(dirname $0) && cd .. && pwd)"

# Script version
SCRIPT_VERSION="1.00.02.00"

# default out of source directory
buildDir="build"

# Keep the gtest colors when run from ctest
export GTEST_COLOR=1

NP=1
if [[ -e /proc/cpuinfo ]]
then
    NP=$(cat /proc/cpuinfo | grep ^processor | wc -l)
    NP=$(( $NP * 2 ))
fi

die()
{
    echo "======================================"
    echo " $*"
    echo "======================================"
    exit 1
}

makeBuildDir()
{
    [[ $1 -eq 1 ]] && [[ -d $buildDir ]] && rm -rf $buildDir
    mkdir -p $buildDir
}

generateBuildSystem()
{
    echo "======================"
    for a in $*
    do
        echo $a
    done
    echo "======================"

    pushd $buildDir
    cmake $* ..
    [[ $? -ne 0 ]] && die "ERR: failed to generate cmake "

    popd
}

buildApp()
{
    pushd $buildDir

    make -j $NP
    [[ $? -ne 0 ]] && die "ERR: failed to build project $(pwd)"

    popd

}

buildPackage()
{
    pushd $buildDir

    make package

    popd

    return 0
}

runUnitTest()
{
    pushd $buildDir
    ctest -VV
    [[ $? -ne 0 ]] && die "ERR: unit-tests failed $(pwd)"
    popd
}

usage()
{
cat <<LAB_USAGE
$(basename "$0")  v$SCRIPT_VERSION

usage:
    -G    Run all unittest
    -p    Build install package
    -r    Release build

    -h    Show this help


Advanced:
    -d    Delete build directory
    -c    Generate build system
    -e    Build project
    -b    Build benchmarks
    -g    Run all unit-tests
    -C    Build with Clang compiler
    -s    Build with sanitizer flags on


LAB_USAGE
}


generateBuildSystemFlag=0
runUnitTestFlag=0
buildPackageFlag=0

clearBuildFlag=0
checkSoftware=0
buildOnlyFlag=0
buildForBenchmarkFlag=0
clangToolchainFlag=0
toolchain=" "
buildType=" "
sanitizer=" "
fuzzer=" "
benchmark=" "
unittest=" "

while getopts "dcegbCsfrGphu" opt
do
    case $opt in
        d)
            clearBuildFlag=1
            ;;
        c)
            generateBuildSystemFlag=1
            ;;
        e)
            buildOnlyFlag=1
            ;;
        g)
            runUnitTestFlag=1
            unittest="-DBUILD_UNITTEST=ON"
            ;;
        b)
            buildForBenchmarkFlag=1
            benchmark="-DBUILD_BENCHMARK=ON"
            ;;
        C)
            toolchain="-DCMAKE_TOOLCHAIN_FILE=../toolchain_clang.cmake"
            ;;
        s)
            sanitizer="-DENABLE_SANITIZER=ON"
            ;;
        f)
            fuzzer="-DBUILD_FUZZER=ON"
            ;;
        r)
            buildType="-DRELEASE_BUILD=ON"
            ;;
        G)
            clearBuildFlag=1
            generateBuildSystemFlag=1
            clangToolchainFlag=1
            buildOnlyFlag=1
            runUnitTestFlag=1
            unittest="-DBUILD_UNITTEST=ON"
            ;;
        p)
            buildPackageFlag=1
            ;;
        h|u|?)
            usage
            exit 1
            ;;
    esac
done



pushd $WORKING_DIR

if [[ $# -eq 0 ]]
then
    clearBuildFlag=1
    generateBuildSystemFlag=1
    buildOnlyFlag=1
fi

makeBuildDir $clearBuildFlag

[[ $generateBuildSystemFlag -ne 0 ]] && generateBuildSystem "${toolchain} ${sanitizer} ${unittest} ${benchmark} ${fuzzer}"
[[ $buildOnlyFlag -ne 0 ]] && buildApp
[[ $runUnitTestFlag -ne 0 ]] && runUnitTest
[[ ${buildPackageFlag} -ne 0 ]] && buildPackage

popd
