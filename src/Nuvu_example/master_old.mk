CC            = gcc
CFLAGS        = -Wall -fopenmp -pipe
CFLAGS += -O2 
CFLAGS += -I${NC_UTILITY}
CFLAGS += -I/usr/lib/x86_64-linux-gnu -I/usr/include 
CFLAGS += -I${NC_INSTALL}/include 

LIBS  = ${NC_INSTALL}/lib/libnuvu.so 

NC_INSTALL = /opt/NuvuCameras
NC_UTILITY = ../Utility

UNAME = $(shell cat /etc/lsb-release | grep -i distrib_id | sed s/DISTRIB_ID=//)
ifeq ($(UNAME),Ubuntu)
	#Libraries for Ubuntu
	LIBS += /usr/lib/x86_64-linux-gnu/libtiff.so /usr/lib/x86_64-linux-gnu/libdl.so
else
	#Libraries for Scientific Linux & CentOS
	LIBS += /usr/lib64/libtiff.so /usr/lib64/libdl.so
endif

LIBS += -lncurses 
LIBS += -lstdc++ -lm


OBJECTS = $(FILE).o $(UTILITY_OBJS)
HEADERS = $(UTILITY_HDR)

UTILITY_OBJS = $(UTILITY_SRC:.c=.o)
UTILITY_SRC = $(NC_UTILITY)/utility.c
UTILITY_HDR = $(NC_UTILITY)/utility.h


# RULES
.PHONY: all clean cleano

ifeq ($(FILE),)
all:
	@echo "You must specify FILE to build"
else
all: $(FILE)

$(FILE): $(FILE).o $(OBJECTS)
	@echo "Linking : \t$@"
	@$(CC) $(CFLAGS) -o $(FILE)  $(OBJECTS) $(LIBS) 

$(FILE).o: $(FILE).c
	@echo "Compiling : \t$@"
	@$(CC) -c $(CFLAGS) $(FILE).c
endif

.c.o: $(HEADERS)
	@$(CC) $(CFLAGS) -c $< -o $@

cleano:
	rm -f *.o $(UTILITY_OBJS)

clean: cleano
	rm -f $(FILE)
