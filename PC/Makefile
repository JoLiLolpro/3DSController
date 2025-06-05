CC		:=	gcc
LN		:=	gcc
ODIR	:=	build
SDIR	:=	source
IDIR	:=	include
LDIR	:=	lib
CFLAGS	:=	-I$(IDIR) -fms-extensions -O2 -Wall
LFLAGS	:=	$(LDIR)/vJoyInterface.lib -lws2_32 -lGdi32 -lgdiplus -static-libgcc
CFILES	:=	$(wildcard $(SDIR)/*.c)
OBJS	:=	$(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(wildcard $(SDIR)/*.c))

PLATFORM = $(shell uname)

TARGET=3DSController.exe


$(TARGET): $(ODIR) $(OBJS)
	$(LN) $(ODIR)/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) 

$(ODIR):
	@mkdir $@

.PHONY: clean

clean:
	rm -f $(TARGET) $(ODIR)/*.o
