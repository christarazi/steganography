CC = gcc
CCFLAGS = -g -std=gnu11 -Wall -Wextra -pedantic
SRC = src
INC = include
BUILD = build
INCLUDES = $(INC)/bmp.h $(INC)/helper.h $(INC)/stegan.h
OBJS = $(BUILD)/steg.o $(BUILD)/bmp.o $(BUILD)/helper.o $(BUILD)/stegan.o
EXE = steg

all: $(EXE)

steg: $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) -o $(EXE)

$(BUILD)/steg.o: $(SRC)/steg.c $(INCLUDES)
	$(CC) $(CCFLAGS) -c $(SRC)/steg.c -o $(BUILD)/steg.o

$(BUILD)/bmp.o: $(SRC)/bmp.c
	$(CC) $(CCFLAGS) -c $(SRC)/bmp.c -o $(BUILD)/bmp.o

$(BUILD)/helper.o: $(SRC)/helper.c
	$(CC) $(CCFLAGS) -c $(SRC)/helper.c -o $(BUILD)/helper.o

$(BUILD)/stegan.o: $(SRC)/stegan.c
	$(CC) $(CCFLAGS) -c $(SRC)/stegan.c -o $(BUILD)/stegan.o

clean:
	rm $(OBJS) $(EXE)
