TARGET		= Xstation86
OBJS_TARGET	= Xstation86.o graphic.o audio.o sdl.o callback.o libgpu.o libini.o \
			e8086/disasm.o e8086/e8086.o e8086/e80186.o e8086/e80286r.o e8086/ea.o \
			e8086/flags.o e8086/opcodes.o e8086/pqueue.o

CFLAGS += -O2 `sdl-config --cflags` -fpermissive
LIBS += `sdl-config --libs` -lm -lc -lstdc++

include Makefile.in
