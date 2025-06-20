CC      := gcc
LN      := gcc
ODIR    := build
SDIR    := source
IDIR    := include
LDIR    := lib
CFLAGS  := -I$(IDIR) -O2 -Wall
LFLAGS  := -lX11 -lXtst

CFILES  := $(wildcard $(SDIR)/*.c)
OBJS    := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES))

TARGET := 3DSController-linux

$(TARGET): $(ODIR) $(OBJS)
	$(LN) $(OBJS) -o $@ $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(TARGET) $(ODIR)/*.o
