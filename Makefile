# Copyright (C) 2017 Chris Tarazi
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc
CCFLAGS = -g -std=gnu11 -Wall -Wextra -pedantic
SRC = src
INC = include
BUILD = build
INCLUDES = $(INC)/args.h $(INC)/bmp.h $(INC)/helper.h $(INC)/stegan.h
OBJS = $(BUILD)/main.o $(BUILD)/args.o $(BUILD)/bmp.o $(BUILD)/helper.o \
	$(BUILD)/stegan.o
EXE = steg

all: $(EXE)

steg: $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) -o $(EXE)

$(BUILD)/main.o: $(SRC)/main.c $(INCLUDES)
	$(CC) $(CCFLAGS) -c $(SRC)/main.c -o $(BUILD)/main.o

$(BUILD)/args.o: $(SRC)/args.c
	$(CC) $(CCFLAGS) -c $(SRC)/args.c -o $(BUILD)/args.o

$(BUILD)/bmp.o: $(SRC)/bmp.c
	$(CC) $(CCFLAGS) -c $(SRC)/bmp.c -o $(BUILD)/bmp.o

$(BUILD)/helper.o: $(SRC)/helper.c
	$(CC) $(CCFLAGS) -c $(SRC)/helper.c -o $(BUILD)/helper.o

$(BUILD)/stegan.o: $(SRC)/stegan.c
	$(CC) $(CCFLAGS) -c $(SRC)/stegan.c -o $(BUILD)/stegan.o

clean:
	rm $(OBJS) $(EXE)
