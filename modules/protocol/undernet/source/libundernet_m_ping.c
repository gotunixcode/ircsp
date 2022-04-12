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
#include	<assert.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<sys/time.h>
#include	<ctype.h>


/* IRCSP Core Includes */
#include	"ircsp.h"
#include	"log.h"
#include	"alloc.h"
#include	"conf.h"
#include	"socket.h"
#include	"users.h"
#include	"servers.h"
#include	"channels.h"
#include	"modules.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_ping.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_ping ()
                |
 DESCRIPTION    |    Undernet Protocol (m_ping)
                |    This function will handle Server Message: PING [G]
                |
                |        AC G !1236971750.709779 UWorld.GOTUNIX.NET 1236971750.709779
                |        @(#) - (socket.c:168) ssprintf():  Socket(SEND): AE Z AC AC -32768 4646262703.916172
                |
                |    We need to reploy with the following:
                |        <my_numeric> Z <remote_numeric> !<remotets> <difference> <localts>
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_ping (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	struct timeval		orig;
	struct timeval		now;
	char			*c_delim;
	char			*argv[4];
	int			i_diff;


	assert (sBuf != NULL);


	argv[0] = strtok (sBuf, " ");			/* Remote Numeric */
	strtok (NULL, " ");				/* Server Message: PING [G] - Ignored */
	argv[1] = strtok (NULL, " ");			/* Remote TS with '!' */
	strtok (NULL, " ");				/* Our Server Numeric */
	argv[2] = strtok (NULL, " ");			/* Remote TS without ! (orig_ts) */


	orig.tv_sec = strtoul(argv[2], &c_delim, 10);
	orig.tv_usec = (*c_delim == '.') ? strtoul(c_delim + 1, NULL, 10) : 0;
	gettimeofday(&now, NULL);
	i_diff = (now.tv_sec - orig.tv_sec) * 1000 + (now.tv_usec - orig.tv_usec) / 1000;


	ssprintf (s,
		"%s Z %s %s %d %d.%d\n",
		config->server_numeric, argv[0], argv[1], i_diff, now.tv_sec,
		(unsigned)now.tv_usec);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
