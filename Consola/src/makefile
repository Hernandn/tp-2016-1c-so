CC=gcc
TARGET=Consola
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

#Defino el entorno necesario para que corra la consola
env:
	ln -sf $(TARGET_PATH)/$(TARGET) /bin
	echo 'export CONF_CONSOLA="$(TARGET_PATH)/consola.conf"' > /etc/profile.d/consola.sh
	chmod -x /etc/profile.d/consola.sh

install: all env

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

.PHONY: default all clean env install
