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
#include	"alloc.h"
#include	"modules.h"
#include	"cmds.h"
#include	"users.h"
#include	"servers.h"
#include	"channels.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_mode.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_mode ()
                |
 DESCRIPTION    |    Undernet Protocol (m_mode)
                |    This function will handle Server Message: MODE [M]
                |
                |    This function has many contingencies to deal with (we are going to try to split this up into a few
                |    functions.
                |
                |        libundernet_m_cmode() - will handle channel modes
                |        libundernet_m_umode() - will handle user modes
                |        libundernet_m_mode()  - will figure out if its a channel mode or user mode and send the request to the
                |                                correct function
                |
                |        Usermode:
                |            [NUM] M <nickname> :<modes>
                |
                |        Channel Mode:
                |            [NUM] M <#channel> <modes> <ts>
                |
                |        Channel User Mode:
                |            [NUM] M <#channel> <modes> [NUM] [NUM] [NUM]
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_mode (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	char		*argv[1];
	char		*c_buffer;


	assert (sBuf != NULL);


	c_buffer = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_buffer != NULL);


	strcpy (c_buffer, sBuf);


	strtok (c_buffer, " ");			/* Numeric */
	strtok (NULL, " ");			/* Server Message: MODE [M] - Ignore */
	argv[0] = strtok (NULL, " ");		/* Channel or User Numeric */


	if (argv[0][0] != '#')
	{
		libundernet_m_umode (sBuf);
	}
	else
	{
		libundernet_m_cmode (sBuf);
	}


	ircsp_free (c_buffer);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_cmode ()
                |
 DESCRIPTION    |    Undernet Protocol (m_cmode)
                |    This function will handle Server Message: MODE [M]
                |
                |        Channel Mode:
                |            [NUM] M <#channel> <modes> <ts>
                |
                |        Channel User Mode:
                |            [NUM] M <#channel> <modes> [NUM] [NUM] [NUM]
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_cmode (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART), __FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	char			*argv[9];
	char			*ban;
	char			*key;
	char			*numeric;
	int			limit;
	int			flag;
	int			p = -1;
	int			addModes = 0;


	assert (sBuf != NULL);


	argv[0] = strtok(sBuf, " ");		/* User Numeric */
	strtok(NULL, " ");			/* Server Message: MODE [M] - Ignore */
	argv[1] = strtok(NULL, " ");		/* Channel */
	argv[2] = strtok(NULL, " ");		/* Channel Modes */


	p_channel = Channel_Find (argv[1]);
	assert (p_channel != NULL);


	while (argv[2][++p] != '\0')
	{
		if (argv[2][p] == '+')
		{
			addModes = 1;
			continue;
		}
		else if (argv[2][p] == '-')
		{
			addModes = 0;
			continue;
		}


		if (argv[2][p] == 'o')
		{
			numeric = strtok(NULL, " ");
			p_nickname = Nickname_Find (numeric);


			assert (p_nickname != NULL);


			p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);
			p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);


			assert (p_channel_nickname != NULL);
			assert (p_nickname_channel != NULL);


			if (addModes)
			{
				p_channel_nickname->flags |= IS_CHANOP;
				p_nickname_channel->flags |= IS_CHANOP;
				continue;
			}
			else
			{
				p_channel_nickname->flags &= ~IS_CHANOP;
				p_nickname_channel->flags &= ~IS_CHANOP;
				continue;
			}
		}


		if (argv[2][p] == 'v')
		{
			numeric = strtok(NULL, " ");
			p_nickname = Nickname_Find (numeric);


			assert (p_nickname != NULL);


			p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);
			p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);


			assert (p_channel_nickname != NULL);
			assert (p_nickname_channel != NULL);


			if (addModes)
			{
				p_channel_nickname->flags |= IS_CHANVOICE;
				p_nickname_channel->flags |= IS_CHANVOICE;
				continue;
			}
			else
			{
				p_channel_nickname->flags &= ~IS_CHANVOICE;
				p_nickname_channel->flags &= ~IS_CHANVOICE;
				continue;
			}
		}


		if (argv[2][p] == 'b')
		{
			ban = strtok(NULL, " ");
			if (addModes)
			{
				ChannelBan_Add (p_channel, ban);
				continue;
			}
			else
			{
				ChannelBan_Del (p_channel, ban);
				continue;
			}
		}


		if (argv[2][p] == 'k')
		{
			key = strtok(NULL, " ");
			if (addModes)
			{
				if (!p_channel->key)
				{
					p_channel->key = (char *)ircsp_calloc (1, strlen (key) + 5);
					assert (p_channel->key != NULL);
				}
				else
				{
					ircsp_free (p_channel->key);
					p_channel->key = (char *)ircsp_calloc (1, strlen (key) + 5);
					assert (p_channel->key != NULL);
				}

				strcpy(p_channel->key, key);
				p_channel->flags |= CHANNEL_HAS_KEY;
				continue;
			}
			else
			{
				p_channel->key = NULL;
				p_channel->flags &= ~CHANNEL_HAS_KEY;
				continue;
			}
		}


		if (argv[2][p] == 'l')
		{
			limit = atoi(strtok(NULL, " "));
			if (addModes)
			{
				p_channel->limit = limit;
				p_channel->flags |= CHANNEL_HAS_LIMIT;
				continue;
			}
			else
			{
				p_channel->limit = 0;
				p_channel->flags &= ~CHANNEL_HAS_LIMIT;
				continue;
			}
		}
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND), __FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_umode ()
                |
 DESCRIPTION    |    Undernet Protocol (m_umode)
                |    This function will handle Server Message: MODE [M]
                |
                |        Usermode:
                |            [NUM] M <nickname> :<modes>
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_umode (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t	*p_nickname;
	operator_t	*p_operator;
//	NickInfo	*p_nickinfo;
	server_t	*p_server;
	char		*argv[9];
	int		flag;
	int		p = -1;
	int		 addModes = 0;


	assert (sBuf != NULL);


	argv[0] = strtok(sBuf, " ");		/* User Numeric */
	strtok(NULL, " ");			/* Server Message: MODE [M] - Ignore */
	argv[1] = strtok(NULL, " ");		/* Nickname Modes being set on */
	argv[2] = strtok(NULL, " ");		/* User Modes */


	p_nickname = Nickname_Find (argv[1]);
	assert (p_nickname != NULL);
	assert (p_nickname->nickname != NULL);

	p_server = p_nickname->server_p;
	assert (p_server != NULL);


	while (argv[2][++p] != '\0')
	{
		if (argv[2][p] == '+')
		{
			addModes = 1;
			continue;
		}
		if (argv[2][p] == '-')
		{
			addModes = 0;
			continue;
		}

		if (argv[2][p] == 'o')  /* Operator! */
		{
			if (addModes)
			{
//				p_nickinfo = nickinfo_find (p_nickname->nickname);
//				if (!p_nickinfo)
//				{
//					if (service->nickserv)
//					{
//						cmd_notice (service->nickserv->nickname, p_nickname->nickname,
//							"It is \002STRONGLY\002 encourged that IRC Operators register their nicknames.");
//
//						cmd_notice (service->nickserv->nickname, p_nickname->nickname,
//							"Type: \002/msg %s HELP REGISTER\002", config->nickserv->nickname);
//					}
//					else
//					{
//						cmd_notice (config->server_name, p_nickname->nickname,
//							"It is \002STRONGLY\002 encourged that IRC Operators register their nicknames.");
//
//						cmd_notice (config->server_name, p_nickname->nickname,
//							"Type: \002/msg %s HELP REGISTER\002", config->nickserv->nickname);
//					}
//				}


				nickname_t		*p_nickname_operserv;
//
//				p_nickname_operserv = Nickname_Find (config->operserv->nickname);
//				if (p_nickname_operserv != NULL)
//				{
//					cmd_notice_operators (p_nickname_operserv->nickname, "\002OPERATOR DETECTED\002 - %s (%s@%s) on %s",
//						p_nickname->nickname, p_nickname->username, p_nickname->hostname_p->hostname, p_nickname->server_p->name);
//
//					cmd_notice (p_nickname_operserv->nickname, p_nickname->nickname, "Greetings Operator!");
//				}

				p_operator = Operator_Add (p_nickname);
				p_nickname->flags |= NICK_IS_OPER;
				p_server->numOpers++;
			}
			else
			{
				p_nickname->flags &= ~NICK_IS_OPER;
				Operator_Del (p_nickname);

				p_server->numOpers--;
			}
		}
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND), __FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
