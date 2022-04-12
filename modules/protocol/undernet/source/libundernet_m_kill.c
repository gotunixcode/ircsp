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
#include	"alloc.h"
#include	"conf.h"
#include	"ircsp.h"
#include	"log.h"
#include	"modules.h"
#include	"users.h"
#include	"servers.h"
#include	"channels.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_kill.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_kill ()
                |
 DESCRIPTION    |    Undernet Protocol (m_kill)
                |    This function will handle Server Message: KILL [D]
                |
                |        ABAAd D ADAAU :irc.gotunix.net!pool-98-113-218-11.nycmny.fios.verizon.net!evilicey2 (hi)
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_kill (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	hostname_t		*p_hostname;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;
	module_t		*module_p;
	module_callback_t	*module_callback_p;
	char			*argv[4];
	char			*c_buffer;


	assert (sBuf != NULL);


	/* First, lets make a copy of the Socket Buffer in case we need it unmodified */
	c_buffer = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_buffer != NULL);


	strcpy (c_buffer, sBuf);


	argv[0] = strtok (c_buffer, " ");		/* Numeric of the Nickname/Server that issued the KILL */
	strtok (NULL, " ");				/* Server Message: KILL [D] - Ignore */
	argv[1] = strtok (NULL, " ");			/* Numeric of the user that is being KILLed */


	/*
	-------------------------------------------------------------------------------------------------------------------------
	We should add some logic to figure out of the nickname being KILLED is one of our Jupes or Service nicknames
	and bring them back online if thats the case.

	It shouldn't really matter most newer ircds have a user mode to mark a nickname as a service nickname and forbid killing
	it.  But just incase someone doesn't have the MODE enabled its not a bad idea?

	We will worry about that later.
	-------------------------------------------------------------------------------------------------------------------------
	*/

	p_nickname = Nickname_Find (argv[1]);
	if (!p_nickname)
	{
		/* Ok so we might have removed the user from cmd_kill or cmd_nickjupe
		   So it is possible that we won't get a result here. */

		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_NOSUCHNICK),
			__FILE__, __LINE__, __FUNCTION__, argv[1]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	/* Was the Nickname a Service */
	if (p_nickname->flags & NICK_IS_SERVICE)
	{
		module_p = Module_Find_Service (p_nickname->nickname);
		assert (module_p != NULL);


		ModuleCallback_Call (module_p, "set_service_offline");
		ircsp->services_online = FALSE;
	}


	/* Was the Nickname a IRC Operator? */
	if (p_nickname->flags & NICK_IS_OPER)
	{
		Operator_Del (p_nickname);
		p_nickname->flags &= ~NICK_IS_OPER;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Run any m_kill callbacks on our services modules
	-------------------------------------------------------------------------------------------------------------------------
	*/
	module_p = module_h;
	while (module_p)
	{
		if (module_p->ModuleType & MODULETYPE_SERVICE)
		{
			module_callback_p = ModuleCallback_Find (module_p, "m_kill");
			if (module_callback_p)
			{
				module_callback_p->func (module_p, p_nickname);
			}
		}

		module_p = module_p->next;
	}

	/* Was the user in any channels */
	p_nickname_channel = p_nickname->channel_h;
	while (p_nickname_channel)
	{
		p_channel = p_nickname_channel->channel_p;
		ChannelNickname_Del (p_channel, p_nickname);
		NicknameChannel_Del (p_nickname, p_channel);

		p_nickname_channel = p_nickname_channel->next;
	}


	/* Update Hostname Records */
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

	/* Finally we delete the Nickname entry */
	Nickname_Del (p_nickname->nickname);


	ircsp_free (c_buffer);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
