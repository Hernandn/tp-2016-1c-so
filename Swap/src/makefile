CC=gcc
TARGET=Swap
HEADERS=$(wildcard *.h)
OBJECTS=$(patsubst %.c, %.o, $(wildcard *.c))
LIBS=-lmllibs -lcommons -lparser-ansisop
TARGET_PATH=$(shell pwd)

all: $(TARGET)
default: all

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

.PHONY: default all clean