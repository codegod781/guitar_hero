ifneq (${KERNELRELEASE},)
	# KERNELRELEASE defined: we are being compiled as part of the Kernel
	obj-m := vga_framebuffer.o note_reader.o
endif

CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -D_XOPEN_SOURCE=600
LDFLAGS=-lSDL2 -lpthread -lpng -lm

SRCS=game_logic.c sprites.c vga_emulator.c guitar_state.c colors.c
OBJS=$(SRCS:.c=.o)
TARGET=game_logic

KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
PWD := $(shell pwd)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

modules:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules CFLAGS="$(CFLAGS)"

clean:
	rm -f $(OBJS) $(TARGET)
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
	${RM} vga_framebuffer.ko

.PHONY: all clean
