SRCDIR = src
INCDIR = include
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)

CC = gcc

CFLAGS = -g -Wall -I$(INCDIR)

all: $(BINDIR)/chess

$(BINDIR)/chess: $(SRCS)
	$(CC) $(CFLAGS) -o $(BINDIR)/chess $(SRCS)

clean:
	rm ./bin/chess
