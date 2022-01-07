################################################################################
# Makefile
################################################################################
PROJECT_ROOT=../../../
OPT_INC = ${PROJECT_ROOT}/common/make/common_spin.mk
-include ${OPT_INC}

################################################################################
# Key paths and settings
################################################################################
ifeq ($(wildcard ${OPT_INC}),)
CC = gcc
CFLAGS = -g -o
LDFLAGS = -g
ODIR  = .obj/build${D}
SDIR  = .
MKDIR = mkdir -p
PLATFORM = $(shell uname)
ifeq ($(PLATFORM),Darwin)
OS = mac
endif
endif

OUTPUTNAME = ROI_Control${D}
OUTDIR = .

################################################################################
# Dependencies
################################################################################
# Spinnaker deps
SPINNAKER_LIB = -L../../../spinnaker/lib -lSpinnaker${D} -lSpinnaker_C${D} ${SPIN_DEPS}

################################################################################
# Master inc/lib/obj/dep settings
################################################################################
_OBJ = ROI_Control.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
INC = -I../../../spinnaker/include/spinc
ifneq ($(OS),mac)
INC += -I/opt/spinnaker/include/spinc
LIB += -Wl,-Bdynamic ${SPINNAKER_LIB}
LIB += -Wl,-rpath-link=../../lib
else
INC += -I/usr/local/include/spinnaker/spinc
LIB += -rpath ../../lib/
LIB += ${SPINNAKER_LIB}
endif

################################################################################
# Rules/recipes
################################################################################
# Final binary
${OUTPUTNAME}: ${OBJ}
	${CC} ${CFLAGS} ${OUTPUTNAME} ${OBJ} ${LIB}
	# mv ${OUTPUTNAME} ${OUTDIR}

# Intermediate object files
${OBJ}: ${ODIR}/%.o : ${SDIR}/%.c
	@${MKDIR} ${ODIR}
	${CC} ${INC} -Wall -g -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME}
	@echo "all cleaned up!"
