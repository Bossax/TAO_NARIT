# Makefile for handling contents of Nüvü Cameras software examples directory
# 'make'                               builds all examples
# 'make cleano'                        removes all object files created during builds
# 'make clean'                         removes all object files and executables created during builds


# Handle spaces à la < https://stackoverflow.com/questions/9838384/can-gnu-make-handle-filenames-with-spaces >
s+ = $(subst \ ,+,$1)
+s = $(subst +,\ ,$1)

OWNED = $(call s+,$(shell find ./* -mindepth 1 -depth -path "*/*/[Mm]akefile" | xargs -I{} echo "{}"| sed "s/\ /\\\ /g"))
TARG_DIRS = $(subst Makefile,,$(subst makefile,,${OWNED}))


# RULES
.PHONY: all $(TARG_DIRS)

all: $(TARG_DIRS)

$(TARG_DIRS) : $(call +s,${OWNED})
	@make -C $(call +s,$@)


.PHONY: clean cleano

cleano:
	find ./* -mindepth 1 -depth -path "*/*/[Mm]akefile" -execdir make cleano \; # Doesn't clean executables!
	
clean:
	find ./* -mindepth 1 -depth -path "*/*/[Mm]akefile" -execdir make clean \; # Implies cleano


