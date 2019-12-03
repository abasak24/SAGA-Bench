# See LICENSE.txt for license details.

C = g++
CFLAGS = -O2 -Wall -Wextra -std=c++11 -fpermissive

CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -pedantic -std=c++11 -fopenmp

STC_PREFIX := s_
DYN_PREFIX := d_

STC_DIR := src/static
DYN_DIR := src/dynamic
UTL_DIR := src/common
OBJ_DIR := obj
BIN_DIR := bin

STC_SRC := $(wildcard $(STC_DIR)/*.cc)
STC_SRC += $(wildcard $(UTL_DIR)/*.cc)
STC_HDR := $(wildcard $(STC_DIR)/*.h)
STC_HDR += $(wildcard $(UTL_DIR)/*.h)

STC_BIN := $(addprefix $(BIN_DIR)/$(STC_PREFIX),$(notdir $(basename $(wildcard $(STC_DIR)/*.cc))))

DYN_SRC := $(wildcard $(DYN_DIR)/*.cc)
DYN_SRC += $(wildcard $(DYN_DIR)/*.c)
DYN_SRC += $(wildcard $(UTL_DIR)/*.cc)
DYN_HDR := $(wildcard $(DYN_DIR)/*.h)
DYN_HDR += $(wildcard $(UTL_DIR)/*.h)

DYN_OBJ := $(addprefix $(OBJ_DIR)/$(DYN_PREFIX),$(notdir $(patsubst %.c,%.o,$(wildcard $(DYN_DIR)/*.c))))
DYN_OBJ += $(addprefix $(OBJ_DIR)/$(DYN_PREFIX),$(notdir $(patsubst %.cc,%.o,$(wildcard $(DYN_DIR)/*.cc))))

.PHONY : all
all : $(BIN_DIR)/errorExtractor frontEnd $(STC_BIN)

$(BIN_DIR)/errorExtractor : errorExtractor.cc
	$(CXX) $(CXXFLAGS) $< -o $@

frontEnd : $(DYN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/$(DYN_PREFIX)%.o : $(DYN_DIR)/%.cc $(DYN_HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/$(DYN_PREFIX)%.o : $(DYN_DIR)/%.c $(DYN_HDR)
	$(C) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(STC_PREFIX)% : $(STC_DIR)/%.cc $(STC_HDR)
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY : clean

clean:
	rm -f frontEnd
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*
