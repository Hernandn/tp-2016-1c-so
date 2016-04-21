RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=build/%.o)

# Clean and compile .so
all: build/mllibs.so

create-dirs:
	mkdir -p build/sockets

build/mllibs.so: create-dirs $(OBJS)
	$(CC) -shared -o "build/mllibs.so" $(OBJS)

build/sockets/%.o: sockets/%.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# Add debug parameters and compile
debug: CC += -DDEBUG -g
debug: all

# Clean build files
clean:
	$(RM) build

install: all
	mkdir -p /usr/include/mllibs
	cp -u build/mllibs.so /usr/lib
	cp --parents -u $(H_SRCS) /usr/include/mllibs

uninstall:
	rm -f /usr/lib/mllibs.so
	rm -rf /usr/include/mllibs

.PHONY: all create-dirs clean install uninstall
