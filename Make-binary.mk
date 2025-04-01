
# Building the binary, and detecting changes in include files.

MODULES = $(SRCS:.c=)
OBJS = $(SRCS:.c=.o)

${BINARY}: $(OBJS)
.PHONY: binary
binary: depend ${BINARY}

.PHONY: depend
depend: .depend

.depend: $(SRCS)
	rm -f "$@"
	${CC} ${CFLAGS} $(SRCS) -m  >/dev/null  # save dependencies, per-file
	@for M in $(MODULES); do \
	  echo $$M.o : `cat $$M.pp.dep | sed -r 's/\"[^"]+\"//g' | tr \\\\ / 2>/dev/null` >> "$@" ; \
	  echo "" >> "$@" ; \
	  rm -f $$M.pp.dep ; done

-include .depend
