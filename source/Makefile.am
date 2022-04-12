SUBDIRS = .

## src/Makefile.am
bin_PROGRAMS = ircsp
ircsp_SOURCES = \
	alloc.c			\
	channels.c		\
	cmds.c			\
	conf.c			\
	ircsp.c			\
	log.c			\
	match.c			\
	misc.c			\
	modules.c		\
	mysql.c			\
	rehash.c		\
	scheduler.c		\
	servers.c		\
	signals.c		\
	socket.c		\
	strings.c		\
	synch.c			\
	users.c			\
	version.c



AM_CPPFLAGS = -I../include @MYSQL_CONFIG_CFLAGS@

AM_LDFLAGS = -g -ggdb -O2 -pipe -Wall -Wstrict-prototypes -Wmissing-prototypes -Wunused-function -Wno-unused -fno-strict-aliasing
ircsp_LDFLAGS=@MYSQL_CONFIG_LIBS@ -lcrypt -ldl -rdynamic
