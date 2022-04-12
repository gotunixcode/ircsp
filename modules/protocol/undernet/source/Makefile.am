AM_CPPFLAGS=-I$(top_srcdir)/include -I../include

undernetdir = ${prefix}/modules
undernet_LTLIBRARIES = libundernet.la
undernet_la_LDFLAGS = -module -avoid-version -shared
libundernet_la_SOURCES =\
			libundernet.c			\
			libundernet_cmds.c		\
			libundernet_introdouce.c	\
			libundernet_m_burst.c		\
			libundernet_m_create.c		\
			libundernet_m_destruct.c	\
			libundernet_m_eob.c		\
			libundernet_m_info.c		\
			libundernet_m_join.c		\
			libundernet_m_kill.c		\
			libundernet_m_mode.c		\
			libundernet_m_motd.c		\
			libundernet_m_nick.c		\
			libundernet_m_part.c		\
			libundernet_m_ping.c		\
			libundernet_m_privmsg.c		\
			libundernet_m_quit.c		\
			libundernet_m_server.c		\
			libundernet_m_squit.c		\
			libundernet_m_stats.c		\
			libundernet_m_version.c		\
			libundernet_parser.c

