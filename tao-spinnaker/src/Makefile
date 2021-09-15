SPINNAKER_PREFIX = /opt/spinnaker
SPINNAKER_DEFS = -I$(SPINNAKER_PREFIX)/include/spinc
SPINNAKER_LIBS = -L$(SPINNAKER_PREFIX)/lib -lSpinnaker_C

TAO_PREFIX = $(HOME)/TAO
TAO_DEFS =  -I$(TAO_PREFIX)/base
TAO_LIBS =  -L$(TAO_PREFIX)/base/.libs -ltao

CC = gcc
CPPFLAGS = -I. $(TAO_DEFS) $(SPINNAKER_DEFS)
CFLAGS = -Wall -Werror -O2 -g

TAO_SPINNAKER_OBJS = api.o
TAO_SPINNAKER_TESTS = tao_spinnaker_test-01

default: all

all: libtao-spinnaker.a $(TAO_SPINNAKER_TESTS)

clean:
	rm -f *~

dist-clean: clean
	rm -f *.o lib*.a $(TAO_SPINNAKER_TESTS)

api.o: api.c tao-spinnaker.h												# implicit rules

libtao-spinnaker.a: $(TAO_SPINNAKER_OBJS)						# implicit archive rule
	$(AR) $(ARFLAGS) $@ $^

tao_spinnaker_test-01: tao_spinnaker_test-01.c tao-spinnaker.h libtao-spinnaker.a
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ -L. -ltao-spinnaker $(TAO_LIBS) $(SPINNAKER_LIBS)

.PHOONY: all default clean dist-clean
