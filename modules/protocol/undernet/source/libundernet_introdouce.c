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
#include	<sys/time.h>


/* IRCSP Core Includes */
#include	"alloc.h"
#include	"ircsp.h"
#include	"users.h"
#include	"channels.h"
#include	"servers.h"
#include	"log.h"
#include	"modules.h"
#include	"conf.h"
#include	"socket.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_cmds.h"
#include	"libundernet_introdouce.h"


/* Debug */
#include	"debug.h"


/* Compile Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_introdouce_service ()
                |
 DESCRIPTION    |    This function will bring up a service nickname using the Undernet P10 protocol
                |
                |    Arguments *MUST* be the same on all protocols.
                |
 INPUTS         |        argv[0] = Nickname
                |        argv[1] = Username
                |        argv[2] = Hostname
                |        argv[3] = IRCModes
                |        argv[4] = Realname
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_introdouce_service (argv)
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	nickname_numeric_t	*p_nickname_numeric;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	hostname_t		*p_hostname;
	server_t		*p_server;
	char			*c_numeric;


	assert (argv[0] != NULL);
	assert (argv[1] != NULL);
	assert (argv[2] != NULL);
	assert (argv[3] != NULL);
	assert (argv[4] != NULL);


	/* First lets generate a numeric */
	c_numeric = ircsp_calloc (1, 20);
	NicknameNumeric_Generate (c_numeric);
	p_nickname_numeric = NicknameNumeric_Add (c_numeric);

	assert (p_nickname_numeric != NULL);
	ircsp_free (c_numeric);


	assert (config != NULL);
	assert (config->server_name != NULL);
	assert (config->server_numeric != NULL);


	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->name != NULL);
	assert (p_server->numeric != NULL);


	p_hostname = Hostname_Find (argv[2]);
	if (p_hostname)
	{
		p_hostname->numHosts++;
	}
	else
	{
		p_hostname = Hostname_Add (argv[2]);
	}


	assert (p_hostname != NULL);
	assert (p_hostname->hostname != NULL);


	p_nickname = Nickname_Find (argv[0]);
	if (p_nickname)
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		Something or someone is using our nickname, before we COLLIDE them off lets delete them and update our lists
		accordingly.
		-----------------------------------------------------------------------------------------------------------------
		*/
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
			status->numHosts--;
		}

		if (p_nickname->flags & NICK_IS_OPER)
		{
			p_nickname->flags &= ~NICK_IS_OPER;
			Operator_Del (p_nickname);
		}

		p_nickname_channel = p_nickname->channel_h;
		while (p_nickname_channel)
		{
			p_channel = p_nickname_channel->channel_p;
			ChannelNickname_Del (p_channel, p_nickname);
			NicknameChannel_Del (p_nickname, p_channel);

			p_nickname_channel = p_nickname_channel->next;
		}


		Nickname_Del (p_nickname->nickname);
	}
	else
	{
		p_hostname = Hostname_Find (argv[2]);
		if (p_hostname)
		{
			p_hostname->numHosts++;
		}
		else
		{
			p_hostname = Hostname_Add (argv[2]);
			assert (p_hostname != NULL);
			assert (p_hostname->hostname != NULL);
		}


		p_nickname = Nickname_Add (argv[0], argv[1], p_hostname, p_server, 0);
		assert (p_nickname != NULL);
		assert (p_nickname->nickname != NULL);
		assert (p_nickname->hostname_p != NULL);
		assert (p_nickname->server_p != NULL);

		p_nickname->numeric = (char *)ircsp_calloc (1, strlen (p_nickname_numeric->numeric) + 5);
		assert (p_nickname->numeric != NULL);

		strcpy (p_nickname->numeric, p_nickname_numeric->numeric);
		status->numUsers++;

		p_nickname->flags |= NICK_IS_OPER;
		p_nickname->flags |= NICK_IS_SERVICE;


		ssprintf (s,
			"%s N %s 1 0 %s %s %s AAAAAA %s :%s\n",
			p_server->numeric, argv[0], argv[1], argv[2], argv[3], p_nickname->numeric, argv[4]);
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_introdouce_server ()
                |
 DESCRIPTION    |    This function will introdouce our server using the IRCU protocol
                |
                |    Arguments *MUST* be the same on all protocols.
                |
 INPUTS         |    NONE
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_introdouce_server (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}

	assert (config != NULL);
	assert (config->server_name != NULL);
	assert (config->server_numeric != NULL);
	assert (config->server_comment != NULL);
	assert (config->uplink_pass != NULL);




	server_t	*p_server;


	p_server = Server_Add (config->server_name, time (NULL));
	assert (p_server != NULL);
	assert (p_server->name != NULL);

	p_server->numeric = (char *)ircsp_calloc (1, strlen (config->server_numeric) + 5);
	assert (p_server->numeric != NULL);

	strcpy (p_server->numeric, config->server_numeric);


	ssprintf (s,
		"PASS :%s\n",
		config->uplink_pass);

	ssprintf (s,
		"SERVER %s 1 %d %d J10 %s]]] :%s\n",
		config->server_name, time (NULL), time (NULL), config->server_numeric,
		config->server_comment);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}
}
