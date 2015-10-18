MAKE = make
CC = gcc
INSTALL_FILE = install -p -o root -g root -m 644
INSTALL_DIR = install -p -o root -g root -d
LN = ln -s
RM = rm -fv
CFLAGS += -c -O2 -Wall
LDFLAGS +=

CFILES = image_DXT.c image_helper.c SOIL.c stb_image_aug.c
OFILES = $(CFILES:.c=.o)
LIBNAME = libSOIL
VERSION = 1.07-20071110
MAJOR = 1

HFILES = SOIL.h image_DXT.h image_helper.h \
  stbi_DDS_aug.h stbi_DDS_aug_c.h stb_image_aug.h
AFILE = libSOIL.a
SOFILE = libSOIL.so.$(VERSION)
INCLUDEDIR = /usr/include/SOIL
LIBDIR = /usr/lib

all: $(OFILES) lib

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

lib: $(OFILES)
	# create static library
	ar -cvq $(LIBNAME).a $(OFILES)
	# create shared library
	gcc -shared -Wl,-soname,$(LIBNAME).so.$(MAJOR) -o $(LIBNAME).so.$(VERSION) $(OFILES)

install:
	$(INSTALL_DIR) $(DESTDIR)/$(INCLUDEDIR)
	$(INSTALL_FILE) $(HFILES) $(DESTDIR)/$(INCLUDEDIR)
	$(INSTALL_DIR) $(DESTDIR)/$(LIBDIR)
	$(INSTALL_FILE) $(AFILE) $(DESTDIR)/$(LIBDIR)
	$(INSTALL_FILE) $(SOFILE) $(DESTDIR)/$(LIBDIR)
	( cd $(DESTDIR)/$(LIBDIR) && $(LN) $(SOFILE) $(LIBNAME).so.$(MAJOR) \
	  && $(LN) $(SOFILE) $(LIBNAME).so )

clean:
	$(RM) *.o
	$(RM) *~

distclean:
	$(RM) $(AFILE) $(SOFILE)

.PHONY: all lib clean distclean
