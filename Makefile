CC=g++
CFLAGS=-Wall -D MG_ENABLE_MBEDTLS=0
OPTIM=-O3

#Project specifics and info
PROJECT_NAME=mgrest
ALL_OUT=out/mongoose.o out/frozen.o out/muapp.o out/mubyte.o out/munet.o

#Boost related dependencies
INCLUDES_BOOST=-I C:/Dev/CLibs/boost_1_75_0
LIBBOOST=

#GTest related dependencies
GTEST_ROOT=C:/Dev/CLibs/googletest-master/googletest
INCLUDES_GTEST=-I '${GTEST_ROOT}/include'
LIBGTEST=-lgtest -L '${GTEST_ROOT}/build'
LIBGTEST_MAIN=-lgtest_main

#GMock related dependencies
GMOCK_ROOT=C:/Dev/CLibs/googletest-master/googlemock
INCLUDES_GMOCK=-I '${GMOCK_ROOT}/include'
LIBGMOCK=-lgmock -L '${GMOCK_ROOT}/build'
LIBGMOCK_MAIN=-lgmock_main

#List your tests' source files in the TESTS_SOURCES variable
TESTS_SOURCES=tests/json_unit.cpp tests/mg_unit.cpp
#Choose the test suite to run {GTEST, GMOCK}
TEST_SUITE=GMOCK

#Doxygen executable
DOXYGEN=C:\Dev\Doxygen\doxygen.exe

INCLUDES_SSL=
LIBSSL=#-lmbedtls -lmbedcrypto -lmbedx509 -L lib

INCLUDES_LOG=
LIBTHREAD=-lpthread

#OS Specifics
#Specify your actual os here -> this can be overriden at making (e.g 'make mgrest OS=LIN' to make for linux environment)
OS=WIN
ifeq (${OS}, WIN)
#Windows here
RM=del /f
LIBSOCK=-lwsock32 -lWs2_32
EXX=.exe
else ifeq (${OS}, LINUX)#Define Linux dll and special flags here
#Linux here
RM=rm
MAIN=${EXEC_NAME}
LIBSOCK=
else
endif
INCLUDES=-I include ${INCLUDES_LOG}
ALLTESTS=alltests${EXX}

%${EXX}:
	echo TEST
	echo $@

#Targets
.PHONY : keygen ping doc check distribution install_gmock install_gtest
keygen: bin/keygen${EXX}
ping: bin/ping${EXX}

bin/%${EXX}: src/%.cpp ${ALL_OUT}
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ $< ${ALL_OUT} ${LIBSSL} ${LIBSOCK}
bin/%${EXX}: examples/%.cpp ${ALL_OUT}
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ $< ${ALL_OUT} ${LIBSSL} ${LIBSOCK}

out/%.o: src/%.cpp include/%.hpp
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ -c $<

out/%.o: src/%.c include/%.h
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ -c $<

out/%.o: src/%.cpp include/cesanta/%.hpp
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ -c $<

out/%.o: src/%.c include/cesanta/%.h
	${CC} ${CFLAGS} ${OPTIM} ${INCLUDES} -o $@ -c $<

check: alltests
	cd bin && ./${ALLTESTS}
doc: Doxyfile ${ALL_OUT}
	${DOXYGEN} Doxyfile 
distribution: alltests doc
	ar -rs lib/libmgrest.a ${ALL_OUT}

#Project structure rules
structure:
	-mkdir bin
	-mkdir out
	-mkdir src
	-mkdir include
	-mkdir tests
	-mkdir doc
	-mkdir lib
clean:
	-${RM} out/*.o
	-${RM} bin/*.o

#Test management
alltests: bin/${ALLTESTS}
bin/${ALLTESTS}: ${TESTS_SOURCES} ${ALL_OUT}
ifeq (${TEST_SUITE}, GTEST)
	${CC} ${CFLAGS} ${INCLUDES} ${INCLUDES_GTEST} ${LIBGTEST_MAIN} ${LIBGTEST} -o bin/alltests ${TESTS_SOURCES} ${ALL_OUT} ${LIBSOCK} ${LIBSSL} ${LIBTHREAD}
else ifeq (${TEST_SUITE}, GMOCK)
	${CC} ${CFLAGS} ${INCLUDES} ${INCLUDES_GTEST} ${INCLUDES_GMOCK} ${LIBGMOCK_MAIN} ${LIBGTEST} ${LIBGMOCK} -o bin/alltests ${TESTS_SOURCES} ${ALL_OUT} ${LIBSOCK} ${LIBSSL} ${LIBTHREAD}
endif
#Gtest static libraries building
install_gtest:
	-cd '${GTEST_ROOT}' && mkdir build
	cd '${GTEST_ROOT} && ${CC} -c src/gtest-all.cc src/gtest_main.cc -I include -I .
	cd '${GTEST_ROOT}' && ar rs build/libgtest.a gtest-all.o
	cd '${GTEST_ROOT}' && ar rs build/libgtest_main.a gtest_main.o
	-cd '${GTEST_ROOT}' && ${RM} *.o
install_gmock:
	-cd '${GMOCK_ROOT}' && mkdir build
	cd '${GMOCK_ROOT}' && ${CC} -c src/gmock-all.cc src/gmock_main.cc -I include -I . ${INCLUDES_GTEST}
	cd '${GMOCK_ROOT}' && ar rs build/libgmock.a gmock-all.o
	cd '${GMOCK_ROOT}' && ar rs build/libgmock_main.a gmock_main.o
	-cd '${GMOCK_ROOT}' && ${RM} *.o
