CFLAGS  += -O3 -march=znver2 -mno-bmi -mno-bmi2
CFLAGS  += -fno-ident -I.

MODULES  = src/calc_magic.o
MODULES += src/fast_modulo.o
MODULES += src/fast_division.o
MODULES += src/fast_divisible.o

LDLIBS  += -l:division.a -s
LDFLAGS += -L.

default: division.a

all: default benchmark

division.a: $(MODULES)
	ar rcs $@ $^
	ranlib $@

benchmark: benchmark.o division.a

clean:
	rm -f $(MODULES) division.a benchmark benchmark.o
