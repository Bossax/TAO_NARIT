NC_PREFIX = /opt/NuvuCameras
NC_DEFS 	= -I$(NC_PREFIX)/include

NC_LIBS  = -L$(NC_PREFIX)/lib -lnuvu
NC_LIBS += -ltiff -ldl
NC_LIBS += -lncurses
NC_LIBS += -lstdc++ -lm


TAO_PREFIX = $(HOME)/TAO
TAO_DEFS =  -I$(TAO_PREFIX)/base
TAO_LIBS =  -L$(TAO_PREFIX)/base/.libs -ltao


CC  		= gcc
CFLAGS  = -Wall -fopenmp -pipe
CFLAGS += -O2 -g

CPPFLAGS += -I/usr/lib/x86_64-linux-gnu -I/usr/include
CPPFLAGS = -I. $(TAO_DEFS) $(NC_DEFS)
CFPPLAGS += -I${NC_UTILITY}


TAO_NUVU_OBJS = api.o
TAO_NUVU_TESTS = tao_nuvu_test-01


# RULES
.PHONY: all clean dist-clean

all: libtao-nuvu.a $(TAO_NUVU_TESTS)


clean:
	rm -f *~

dist-clean: clean
	rm -f *.o lib*.a $(TAO_NUVU_TESTS)

api.o: api.c tao_nuvu.h												  # implicit rules

libtao-nuvu.a: $(TAO_NUVU_OBJS)						# implicit archive rule
	$(AR) $(ARFLAGS) $@ $^

tao_nuvu_test-01: tao_nuvu_test-01.c tao_nuvu.h libtao-nuvu.a
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ -L. -ltao-nuvu $(TAO_LIBS) $(NC_LIBS)
