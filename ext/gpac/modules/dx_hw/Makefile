include ../../config.mak

vpath %.c $(SRC_PATH)/modules/dx_hw

#DIRECTSOUND_VERSION is needed for GCC compil..
CFLAGS= $(OPTFLAGS) -w -I$(SRC_PATH)/include -DDIRECTSOUND_VERSION=0x0500

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif

ifeq ($(DX_PATH), system)
LDFLAGS_DX=-ldsound -ldxguid -lddraw -lole32 -lgdi32 -lopenGL32
else
CFLAGS+= -I$(DX_PATH)/include
LDFLAGS_DX=-L$(DX_PATH)/lib -ldsound -ldxguid -lddraw -lole32 -lgdi32 -lopenGL32
endif


#common obj
OBJS=dx_audio.o dx_video.o dx_window.o dx_2d.o copy_pixels.o


SRCS := $(OBJS:.o=.c) 

LIB=gm_dx_hw.$(DYN_LIB_SUFFIX)
ifeq ($(CONFIG_WIN32),yes)
LDFLAGS+=-export-symbols dx_hw.def
endif

all: $(LIB)


$(LIB): $(OBJS)
	windres $(SRC_PATH)/modules/dx_hw/dx_hw.rc dw_hw.o
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) dw_hw.o $(LDFLAGS_DX) -L../../bin/gcc -lgpac $(EXTRALIBS) 


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean: 
	rm -f $(OBJS) ../../bin/gcc/$(LIB)
ifeq ($(CONFIG_WIN32),yes)
	rm -f dx_hw.o
endif

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
