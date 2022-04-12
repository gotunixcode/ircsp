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
#include	"ircsp.h"
#include	"log.h"
#include	"socket.h"
#include	"modules.h"
#include	"users.h"
#include	"servers.h"



/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_cmds.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_stats.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_stats ()
                |
 DESCRIPTION    |    Undernet Protocol (m_stats)
                |    This function will handle Server Message: STATS [R]
                |
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_stats (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	server_t		*p_server;
	char			*argv[2];


	assert (sBuf != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);


	argv[0] = strtok (sBuf, " ");		/* Numeric of the user requesting information */
	strtok (NULL, " ");			/* Server Message: STATS [R] - Ignored */
	argv[1] = strtok (NULL, " ");		/* Stats Paramater */


	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->name != NULL);
	assert (p_server->numeric != NULL);


	p_nickname = Nickname_Find (argv[0]);
	assert (p_nickname != NULL);
	assert (p_nickname->numeric != NULL);


	if (!(p_nickname->flags & NICK_IS_OPER))
	{
		cmd_notice (p_server->name, p_nickname->nickname, "\002Access Denied!!\002");
		return FALSE;
	}


	if ( (!strcasecmp (argv[1], "UPTIME")) || (!strcasecmp (argv[1], "U")) )
	{
		int		i_days;
		int		i_hours;
		int		i_mins;
		int		i_secs;
		time_t		t_curtime;
		time_t		t_uptime;


		assert (ircsp != NULL);
		assert (ircsp->uptime != 0);

		t_curtime = time (NULL);
		t_uptime = t_curtime - ircsp->uptime;


		i_days = t_uptime/86400;
		i_hours = (t_uptime/3600)%24;
		i_mins = (t_uptime/60)%60;
		i_secs = t_uptime%60;


		cmd_notice (p_server->name, p_nickname->nickname, "\002%d\002 Days, \002%d\002 Hours, \002%d\002 Minutes, \002%d\002 Seconds",
			i_days, i_hours, i_mins, i_secs);

		return TRUE;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
