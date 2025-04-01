
# Platform portability between WSL and Ubuntu.

ifneq (${WSL_DISTRO_NAME},)
	# Running under Windows Subsystem for Linux, so we can run the compiler directly.
	CDRIVE = /mnt/c
else
	# Not running under Windows, so assume we're using WINE to run the compiler.
	CDRIVE = ${HOME}/.wine/drive_c
	WINE = wine
endif

WIN_CDRIVE = C:
BOOSTCBIN = ${CDRIVE}/Program Files (x86)/SourceBoost
CC = $(WINE) "${BOOSTCBIN}/boostc_pic18.exe"
LN = $(WINE) "${BOOSTCBIN}/boostlink_picmicro.exe"


# Implicit rules for compiling and linking using BoostC.

.SUFFIXES:
.SUFFIXES: .c .o .h

%.o : %.c
	${CC} ${CFLAGS} $< -o $@

%.hex : %.o
	${LN} ${LDFLAGS} -p $(@F) $(^F) ${LDLIBS}

