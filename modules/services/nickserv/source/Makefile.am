AM_CPPFLAGS=-I$(top_srcdir)/include -I../include

nickservdir = ${prefix}/modules
nickserv_LTLIBRARIES = libnickserv.la
nickserv_la_LDFLAGS = -module -avoid-version -shared
libnickserv_la_SOURCES =\
                        libnickserv.c			\
			libnickserv_admin.c		\
			libnickserv_commands.c		\
			libnickserv_database.c		\
			libnickserv_misc.c		\
			libnickserv_nickinfo.c
