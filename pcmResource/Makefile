# See LICENSE.txt for license details.

C = g++
CFLAGS = -O2 -Wall -Wextra -std=c++11 -fpermissive

CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -pedantic -std=c++11 -fopenmp

# Edit this with your pcm repo
PCM_DIR := /home/abasak/pcm

STC_DIR := src/static
DYN_DIR := src/dynamic
UTL_DIR := src/common
OBJ_DIR := obj
BIN_DIR := bin

PCM_HDR := $(wildcard $(PCM_DIR)/*.h)
PCM_SRC := $(wildcard $(PCM_DIR)/*.cpp)
#PCM_OBJ := $(patsubst %.cpp,%.o,$(wildcard $(PCM_DIR)/*.cpp))
PCM_OBJ := msr.o cpucounters.o pci.o mmio.o client_bw.o utils.o
PCM_OBJ := $(addprefix $(PCM_DIR)/,$(PCM_OBJ))

STC_PREFIX := static/
STC_HDR := $(wildcard $(STC_DIR)/*.h)
STC_HDR += $(wildcard $(UTL_DIR)/*.h)
STC_SRC := $(wildcard $(STC_DIR)/*.cc)
STC_SRC += $(wildcard $(UTL_DIR)/*.cc)
STC_BIN := $(addprefix $(BIN_DIR)/$(STC_PREFIX),$(notdir $(basename $(wildcard $(STC_DIR)/*.cc))))

DYN_PREFIX := dynamic/
DYN_HDR := $(wildcard $(DYN_DIR)/*.h)
DYN_HDR += $(wildcard $(UTL_DIR)/*.h)
DYN_SRC := $(wildcard $(DYN_DIR)/*.cc)
DYN_SRC += $(wildcard $(DYN_DIR)/*.c)
DYN_OBJ := $(addprefix $(OBJ_DIR)/$(DYN_PREFIX),$(notdir $(patsubst %.c,%.o,$(wildcard $(DYN_DIR)/*.c))))
DYN_OBJ += $(addprefix $(OBJ_DIR)/$(DYN_PREFIX),$(notdir $(patsubst %.cc,%.o,$(wildcard $(DYN_DIR)/*.cc))))

.PHONY : all
all : $(BIN_DIR)/errorExtractor frontEnd $(STC_BIN)

# Error Extractor -----------------------------------------------
$(BIN_DIR)/errorExtractor : errorExtractor.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Dynamic Suite -------------------------------------------------

# Link
frontEnd : $(DYN_OBJ) $(PCM_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile C++
$(OBJ_DIR)/$(DYN_PREFIX)%.o : $(DYN_DIR)/%.cc $(DYN_HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C
$(OBJ_DIR)/$(DYN_PREFIX)%.o : $(DYN_DIR)/%.c $(DYN_HDR)
	$(C) $(CFLAGS) -c $< -o $@

# Static Suite --------------------------------------------------

# Link
$(BIN_DIR)/$(STC_PREFIX)% : $(OBJ_DIR)/$(STC_PREFIX)%.o $(PCM_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile
$(OBJ_DIR)/$(STC_PREFIX)%.o : $(STC_DIR)/%.cc $(STC_HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Intel PCM -----------------------------------------------------

$(PCM_OBJ) : $(PCM_SRC)
	cd $(PCM_DIR) && make -f $(PCM_DIR)/Makefile