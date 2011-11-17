CC = gcc
RM = rm -f
AR = ar
RANLIB = ranlib

RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

CFLAGS = -Os -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-qual -Winline -I.

PROGS = iwspy
OBJS=iwlib.o
LIBS = -lm
STATIC=libiw.a
XCFLAGS = $(CFLAGS)

IWLIB=$(STATIC)

all:: $(IWLIB) $(PROGS)

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
%.o: %.c wireless.h
	$(CC) $(XCFLAGS) -c $<
%.so: %.c wireless.h
	$(CC) $(XCFLAGS) $(PICFLAG) -c -o $@ $<


iwspy: iwspy.o $(IWLIB)
	$(CC) -o $@ $^ $(LIBS)

clean:
	$(RM_CMD) 

# Compilation of the static library
$(STATIC): $(OBJS:.o=.so)
	$(RM) $@
	$(AR) cru $@ $^
	$(RANLIB) $@

iwlib.so: iwlib.c iwlib.h wireless.h
