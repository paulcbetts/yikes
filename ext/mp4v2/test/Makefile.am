
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/util

check_PROGRAMS = c_api mp4broadcaster nullcreate nullvplayer urltrack mp4clip

c_api_SOURCES = c_api.c
c_api_LDADD = $(top_builddir)/libmp4v2.la -lstdc++

mp4broadcaster_SOURCES = mp4broadcaster.cpp
mp4broadcaster_LDADD = $(top_builddir)/libmp4v2.la

nullcreate_SOURCES = nullcreate.cpp
nullcreate_LDADD = $(top_builddir)/libmp4v2.la

nullvplayer_SOURCES = nullvplayer.cpp
nullvplayer_LDADD = $(top_builddir)/libmp4v2.la

urltrack_SOURCES = urltrack.cpp
urltrack_LDADD = $(top_builddir)/libmp4v2.la

mp4clip_SOURCES = mp4clip.cpp
mp4clip_LDADD = $(top_builddir)/libmp4v2.la 

