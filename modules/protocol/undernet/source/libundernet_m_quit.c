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


/* IRCSP Core Includes */
#include	"conf.h"
#include	"ircsp.h"
#include	"log.h"
#include	"users.h"
#include	"channels.h"
#include	"servers.h"
#include	"modules.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_quit.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_quit ()
                |
 DESCRIPTION    |    Undernet Protocol (m_quit)
                |    This function will handle Server Message: QUIT [Q]
                |
                |        [ircsp.c:725:ircsp_mainloop()]: Socket (READ): ADAAY Q :Signed off
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_quit (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
//	NickInfo		*p_nickinfo;
	hostname_t		*p_hostname;
//	services_root_t		*p_services_root;
//	services_admin_t	*p_services_admin;
	channel_t		*p_channel;
	server_t		*p_server;
	module_t		*module_p;
	module_callback_t	*module_callback_p;
	char			*argv[1];


	assert (sBuf != NULL);


	argv[0] = strtok (sBuf, " ");		/* Numeric of the user QUITING */


	p_nickname = Nickname_Find (argv[0]);
	assert (p_nickname != NULL);
	assert (p_nickname->numeric != NULL);
	assert (p_nickname->nickname != NULL);


	/* Was the user an IRC Operator */
	if (p_nickname->flags & NICK_IS_OPER)
	{
		Operator_Del (p_nickname);
		p_nickname->flags &= ~NICK_IS_OPER;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Run any m_quit callbacks on our service modules
	-------------------------------------------------------------------------------------------------------------------------
	*/
	module_p = module_h;
	while (module_p)
	{
		if (module_p->ModuleType & MODULETYPE_SERVICE)
		{
			module_callback_p = ModuleCallback_Find (module_p, "m_quit");
			if (module_callback_p)
			{
				module_callback_p->func (module_p, p_nickname);
			}
		}

		module_p = module_p->next;
	}


	/* Was the user on any channels */
	p_nickname_channel = p_nickname->channel_h;
	while (p_nickname_channel)
	{
		p_channel = p_nickname_channel->channel_p;
		ChannelNickname_Del (p_channel, p_nickname);
		NicknameChannel_Del (p_nickname, p_channel);

		p_nickname_channel = p_nickname_channel->next;
	}


	p_hostname = p_nickname->hostname_p;
	assert (p_hostname != NULL);
	assert (p_hostname->hostname != NULL);


	if (p_hostname->numHosts > 1)
	{
		p_hostname->numHosts--;
	}
	else
	{
		Hostname_Del (p_hostname->hostname);
	}

	Nickname_Del (p_nickname->nickname);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
