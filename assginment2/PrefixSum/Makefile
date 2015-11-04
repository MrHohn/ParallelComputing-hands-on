EXECUTABLE := PrefixSum

CU_FILES   := PrefixSum_parallel.cu

CU_DEPS    :=

CC_FILES   := main.cpp PrefixSum_serial.cpp util.cpp

all: $(EXECUTABLE) $(REFERENCE)

LOGS	   := logs

###########################################################

OBJDIR=objs
CXX=g++
CXXFLAGS=-O3 -Wall -m64
LDFLAGS=-lcudart
NVCC=nvcc
NVCCFLAGS=-O3 -arch=sm_20 -m64

OBJS=$(OBJDIR)/main.o  $(OBJDIR)/PrefixSum_serial.o  $(OBJDIR)/util.o  $(OBJDIR)/PrefixSum_parallel.o


.PHONY: dirs clean

default: $(EXECUTABLE)

dirs:
		mkdir -p $(OBJDIR)/

clean:
		rm -rf $(OBJDIR) *.ppm *~ $(EXECUTABLE) $(LOGS)

$(EXECUTABLE): dirs $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/%.o: %.cu
		$(NVCC) $< $(NVCCFLAGS) -c -o $@