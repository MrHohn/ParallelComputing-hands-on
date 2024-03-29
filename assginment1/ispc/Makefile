# check and define 32-bit or 64-bit machine
ARCH=$(shell getconf LONG_BIT)

CXX=clang++
CXXCFLAGS=-Iobjs/ -Icommon -O2 -Wall -g -mavx
CXXLFLAGS=-lpthread
ISPC=ispc
ISPCFLAGS_32=-O2 --arch=x86 --target=avx1-i32x8
ISPCFLAGS_64=-O2 --arch=x86-64 --target=avx1-i32x8
TASKSYS_SOURCE=common/tasksys.cpp

default: sqrt

.PHONY: dirs clean
.PRECIOUS: objs/sqrt.h

dirs:
	/bin/mkdir -p objs/

clean:
	/bin/rm -rf objs *~ sqrt

sqrt: dirs  objs/sqrt.o objs/sqrt_ispc.o objs/sqrt_serial.o objs/tasksys.o objs/sqrt_avx.o
	$(CXX) $(CXXCFLAGS) -o $@ objs/sqrt.o objs/sqrt_ispc.o objs/sqrt_serial.o objs/sqrt_avx.o objs/tasksys.o $(CXXLFLAGS)

objs/sqrt.o: sqrt.cpp objs/sqrt_ispc.h 
	$(CXX) $(CXXCFLAGS) -c -o $@ $<

objs/sqrt_serial.o: sqrt_serial.cpp 
	$(CXX) $(CXXCFLAGS) -c -o $@ $<

objs/sqrt_avx.o: sqrt_avx.cpp 
	$(CXX) $(CXXCFLAGS) -c -o $@ $<

objs/tasksys.o: $(TASKSYS_SOURCE)
	$(CXX) $(CXXCFLAGS) -c -o $@ $<

objs/%_ispc.h objs/%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS_$(ARCH)) $< -o objs/$*_ispc.o -h objs/$*_ispc.h
