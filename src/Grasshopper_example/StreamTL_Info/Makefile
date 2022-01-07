################################################################################
# Acquisition_C Makefile
################################################################################
PROJECT_ROOT=../../../
OPT_INC = ${PROJECT_ROOT}/common/make/common_spin.mk
-include ${OPT_INC}

################################################################################
# Key paths and settings
################################################################################
ifeq ($(wildcard ${OPT_INC}),)
CC = gcc
ODIR  = .obj/build${D}
SDIR  = .
MKDIR = mkdir -p
PLATFORM = $(shell uname)
ifeq ($(PLATFORM),Darwin)
OS = mac
endif
endif

OUTPUTNAME = StreamTL_Info${D}
OUTDIR = .

################################################################################
# Dependencies
################################################################################
# Spinnaker deps
SPINNAKER_LIB = -L../../../lib -lSpinnaker${D} -lSpinnaker_C${D} ${SPIN_DEPS}

################################################################################
# Master inc/lib/obj/dep settings
################################################################################
_OBJ = StreamTL_Info.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
INC = -I../../../include/spinc
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
	${CC} -g -o ${OUTPUTNAME} ${OBJ} ${LIB}
	# mv ${OUTPUTNAME} ${OUTDIR}

# Intermediate object files
${OBJ}: ${ODIR}/%.o : ${SDIR}/%.c
	@${MKDIR} ${ODIR}
	${CC} ${INC} -Wall -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME}
	@echo "all cleaned up!"
