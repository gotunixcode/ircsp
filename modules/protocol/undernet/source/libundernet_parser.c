/*
---------------------------------------------------------------------------------------------------------------------------------
                                          ___   ____     ____   ____    ____
                                         |_ _| |  _ \   / ___| / ___|  |  _ \
                                          | |  | |_) | | |     \___ \  | |_) |
                                          | |  |  _ <  | |___   ___) | |  __/
                                         |___| |_| \_\  \____| |____/  |_|

                                        INTERNET RELAY CHAT SERVICES PACKAGE
---------------------------------------------------------------------------------------------------------------------------------
                                                                .::::.
                                                              .::::::::.
                                                              :::::::::::
                                                              ':::::::::::..
                                                               :::::::::::::::'
                                                                ':::::::::::.
                                                                  .::::::::::::::'
                                                                .:::::::::::...
                                                               ::::::::::::::''
                                                   .:::.       '::::::::''::::
                                                 .::::::::.      ':::::'  '::::
                                                .::::':::::::.    :::::    '::::.
                                              .:::::' ':::::::::. :::::      ':::.
                                            .:::::'     ':::::::::.:::::       '::.
                                          .::::''         '::::::::::::::       '::.
                                         .::''              '::::::::::::         :::...
                                      ..::::                  ':::::::::'        .:' ''''
                                   ..''''':'                    ':::::.'
---------------------------------------------------------------------------------------------------------------------------------

This file is part of the [IRCSP] source code.

Detailed copyright and licensing information can be found in the COPYRIGHT and LICENSE files which should have been
included in this distribution.

@Author       - Justin Ovens <jovens@gotunix.net>
@Copyright    - (C) 1995 - 2019  Justin Ovens and
              -                  GOTUNIX NETWORKS
@License Type - GPLv3/Open Source

---------------------------------------------------------------------------------------------------------------------------------

SOFTWARE LICENSE AGREEMENT
IMPORTANT - READ CAREFULLY

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

---------------------------------------------------------------------------------------------------------------------------------
*/


/* Standard Includes */
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<stdarg.h>
#include	<unistd.h>
#include	<string.h>
#include	<assert.h>
#include	<time.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"alloc.h"
#include	"log.h"
#include	"ircsp.h"
#include	"text.h"
#include	"modules.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_m_burst.h"
#include	"libundernet_m_create.h"
#include	"libundernet_m_destruct.h"
#include	"libundernet_m_eob.h"
#include	"libundernet_m_info.h"
#include	"libundernet_m_join.h"
#include	"libundernet_m_kill.h"
#include	"libundernet_m_mode.h"
#include	"libundernet_m_motd.h"
#include	"libundernet_m_nick.h"
#include	"libundernet_m_part.h"
#include	"libundernet_m_ping.h"
#include	"libundernet_m_privmsg.h"
#include	"libundernet_m_quit.h"
#include	"libundernet_m_server.h"
#include	"libundernet_m_squit.h"
#include	"libundernet_m_stats.h"
#include	"libundernet_m_version.h"
#include	"libundernet_parser.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 Server Messages


   Message, Function
---------------------------------------------------------------------------------------------------------------------------------
*/
struct _server_messages Server_Messages[] = {
	{	"B",		&libundernet_m_burst,		},		/* [B]  - BURST */
	{	"C",		&libundernet_m_create,		},		/* [C]  - CREATE */
	{	"D",		&libundernet_m_kill,		},		/* [D]  - KILL */
	{	"DE",		&libundernet_m_destruct,	},		/* [DE] - DESTRUCT */
	{	"EA",		&libundernet_m_eob_ack,		},		/* [EA] - END_OF_BURST_ACK */
	{	"EB",		&libundernet_m_eob,		},		/* [EB] - END_OF_BURST */
	{	"F",		&libundernet_m_info,		},		/* [F]  - INFO */
	{	"G",		&libundernet_m_ping,		},		/* [G]  - PING */
	{	"J",		&libundernet_m_join,		},		/* [J]  - JOIN */
	{	"L",		&libundernet_m_part,		},		/* [L]  - PART */
	{	"M",		&libundernet_m_mode,		},		/* [M]  - MODE */
	{	"N",		&libundernet_m_nick,		},		/* [N]  - NICK */
	{	"P",		&libundernet_m_privmsg,		},		/* [P]  - PRIVMSG */
	{	"Q",		&libundernet_m_quit,		},		/* [Q]  - QUIT */
	{	"R",		&libundernet_m_stats,		},		/* [R]  - STATS */
	{	"S",		&libundernet_m_server,		},		/* [S]  - SERVER */
	{	"SERVER",	&libundernet_m_server,		},		/* [S]  - SERVER */
	{	"SQ",		&libundernet_m_squit,		},		/* [SQ] - SQUIT */
	{	"V",		&libundernet_m_version,		},		/* [V]  - VERSION */
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_parser ()
                |
 DESCRIPTION    |    Undernet Protocol (parser)
                |    This function will handle parsing the socket buffer and forwarding commands to their specific
                |    parsers.
                |
 INPUTS         |    sBuf  = Socket Buffer
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_parser (sBuf)
	char		*sBuf;		/* Socket Buffer */
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	char		*c_parse;
	char		*argv[2];
	int		i_cmd;


	assert (sBuf != NULL);

	c_parse = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	if (!c_parse)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__);

		ircsp_panic (get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__);
	}


	strcpy (c_parse, sBuf);
	argv[0] = strtok (c_parse, " ");
	argv[1] = strtok (NULL, " ");


	for (i_cmd = 0; i_cmd <= LIBUNDERNET_NUM_MSGS; i_cmd++)
	{
		if ( (!strcasecmp (argv[0], Server_Messages[i_cmd].command)) || (!strcasecmp (argv[1], Server_Messages[i_cmd].command)) )
		{
			if (Server_Messages[i_cmd].func (sBuf))
			{

			}
		}
	}

	ircsp_free (c_parse);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}
}
