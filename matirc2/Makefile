CC       = gcc
RM       = rm -f

OBJFILES = tcp.o irc.o misc.o main.o screen.o commands.o dprintf.o snprintf.o

CFLAGS   = -ggdb3
LIBS     = -lncurses -L.

all: matirc

matirc: $(OBJFILES)
	$(CC) -o $@ $(OBJFILES) $(LIBS)

clean:
	$(RM) matirc *.o
