CC=g++
CFLAGS=-Wall
OPTIM=-O3

#Project specifics and info
PROJECT_NAME=mgrest
EXEC_NAME=keygen_musrv
ALL_OUT=out/mongoose.o out/frozen.o out/muapp.o

#Boost related dependencies
INCLUDES_BOOST=-I C:/Dev/CLibs/boost_1_75_0
LIBBOOST=

#GTest related dependencies
GTEST_ROOT=C:/Dev/CLibs/googletest-master/googletest
INCLUDES_GTEST=-I ${GTEST_ROOT}/include
LIBGTEST=-lgtest -L ${GTEST_ROOT}/build
LIBGTEST_MAIN=-lgtest_main

#GMock related dependencies
GMOCK_ROOT=C:/Dev/CLibs/googletest-master/googlemock
INCLUDES_GMOCK=-I ${GMOCK_ROOT}/include
LIBGMOCK=-lgmock -L ${GMOCK_ROOT}/build
LIBGMOCK_MAIN=-lgmock_main

#List your tests' source files in the TESTS_SOURCES variable
TESTS_SOURCES=
#Choose the test suite to run {GTEST, GMOCK}
TEST_SUITE=GTEST

INCLUDES_SSL=
LIBSSL=-lssl -lcrypto

INCLUDES_LOG=
LIBTHREAD=-lpthread

#OS Specifics
#Defaults for linux-based os
RM=rm
MAIN=${EXEC_NAME}
ALLTESTS=alltests
LIBSOCK=
#Specify your actual os here -> this can be overriden at making (e.g 'make mgrest OS=LIN' to make for linux environment)
OS=WIN
ifeq (${OS}, WIN)
#Windows here
RM=del /f
MAIN=${EXEC_NAME}.exe
LIBSOCK=-lwsock32 -lWs2_32
ALLTESTS=alltests.exe
else ifeq (${OS}, LINUX)//Define Linux dll and special flags here
#Linux here
else
endif
INCLUDES=-I include ${INCLUDES_LOG}

#Targets

${EXEC_NAME}: bin/${MAIN}
bin/${MAIN}: src/${EXEC_NAME}.cpp ${ALL_OUT}
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o bin/${EXEC_NAME} src/${EXEC_NAME}.cpp ${ALL_OUT} ${LIBSOCK}
out/muapp.o: src/muapp.cpp include/muapp.hpp
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o out/muapp.o -c src/muapp.cpp
out/mongoose.o: src/mongoose.c include/mongoose.h
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o out/mongoose.o -c src/mongoose.c
	
out/frozen.o: src/frozen.c include/frozen.h
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o out/frozen.o -c src/frozen.c

#Project structure rules
structure:
	-mkdir bin
	-mkdir out
	-mkdir src
	-mkdir include
	-mkdir tests
clean:
	-${RM} out/*.o
	-${RM} bin/*.o

#Test management
alltests: tests/${ALLTESTS}
tests/${ALLTESTS}: ${TESTS_SOURCES}
ifeq (${TEST_SUITE}, GTEST)
	${CC} ${CFLAGS} ${INCLUDES} ${INCLUDES_GTEST} ${LIBGTEST_MAIN} ${LIBGTEST} -o tests/alltests ${TESTS_SOURCES} ${ALL_OUT}
else ifeq (${TEST_SUITE}, GMOCK)
	${CC} ${CFLAGS} ${INCLUDES} ${INCLUDES_GTEST} ${INCLUDES_GMOCK} ${LIBGMOCK_MAIN} ${LIBGTEST} ${LIBGMOCK} -o tests/alltests ${TESTS_SOURCES} ${ALL_OUT}
endif
#Gtest static libraries building
install_gtest:
	-cd ${GTEST_ROOT} && mkdir build
	cd ${GTEST_ROOT} && ${CC} -c src/gtest-all.cc src/gtest_main.cc -I include -I .
	cd ${GTEST_ROOT} && ar rs build/libgtest.a gtest-all.o
	cd ${GTEST_ROOT} && ar rs build/libgtest_main.a gtest_main.o
	-cd ${GTEST_ROOT} && ${RM} *.o
install_gmock:
	-cd ${GMOCK_ROOT} && mkdir build
	cd ${GMOCK_ROOT} && ${CC} -c src/gmock-all.cc src/gmock_main.cc -I include -I . ${INCLUDES_GTEST}
	cd ${GMOCK_ROOT} && ar rs build/libgmock.a gmock-all.o
	cd ${GMOCK_ROOT} && ar rs build/libgmock_main.a gmock_main.o
	-cd ${GMOCK_ROOT} && ${RM} *.o