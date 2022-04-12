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
#include	"alloc.h"
#include	"modules.h"
#include	"users.h"
#include	"channels.h"
#include	"servers.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_version.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_nick_new ()
                |
 DESCRIPTION    |    Undernet Protocol (m_nick)
                |    This function will handle Server Message: NICK [N]
                |
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
static int
libundernet_m_nick_new (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t			*p_Nickname;
	nickname_t			*p_Nickname_new;
	nickname_channel_t		*p_NicknameChannel;
	nickname_channel_t		*p_NicknameChannel_update;
	channel_t			*p_Channel;
	channel_nickname_t		*p_ChannelNickname_update;
	hostname_t			*p_Hostname;
	operator_t			*p_Operator;
	server_t			*p_Server;
	module_t			*module_p;
	module_callback_t		*module_callback_p;
	char				*argv[9];
	char				*c_nickname;
	time_t				t_timestamp;


	assert (sBuf != NULL);


	c_nickname = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_nickname != NULL);
	strcpy (c_nickname, sBuf);			/* Make a copy of sBuf in case we need it un modified for whatever reason. */


	argv[0] = strtok (sBuf, " ");			/* This could either be the a Server Numeric or a Nickname Numeric
							   Depending on if this is a new user or a nick change. */
	strtok (NULL, " ");				/* NICK [N] Command, we can ignore this. */


	t_timestamp = time (NULL);


	/*
	------------------------------------------------------------------------------------------------------------------------------------------------
	    NEW NICKNAME!

	        We check the length of argv[0] if its 2 we have a Server Numeric which means this is a new user
	        connecting to the server matching that numeric.

	        AD N shitholez 1 1572734200 ~jovens cthulhu.gotunix.net CsEAA8 ADAAC :Justin Ovens
	------------------------------------------------------------------------------------------------------------------------------------------------
	*/
	int	i_is_oper = 0;


	argv[1] = strtok (NULL, " ");		/* Nickname */
	strtok (NULL, " ");			/* Hopcount, we can ignore this */
	argv[2] = strtok (NULL, " ");		/* Time Stamp */
	argv[3] = strtok (NULL, " ");		/* Username */
	argv[4] = strtok (NULL, " ");		/* Hostname */
	argv[5] = strtok (NULL, " ");		/* Modes */


	p_Nickname = Nickname_Find (argv[1]);
	if (p_Nickname)
	{
		/*
		---------------------------------------------------------------------------------------------------------
		    Duplicate nickname detected, this could end up being a problem later on, but we will attempt to fix
		    this by removing flags, updating lists, and deleting the nickname. Before we add the new nickname.
		---------------------------------------------------------------------------------------------------------
		*/
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Duplicate nickname [%s]... Attempting to correct\n",
			__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname);


		if (p_Nickname->flags & NICK_IS_OPER)
		{
			Operator_Del (p_Nickname);
			p_Nickname->flags &= ~ NICK_IS_OPER;
		}

		module_p = module_find_nickserv ();
		if (module_p)
		{
			if (p_Nickname->flags & NICK_IS_ROOT)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_root_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname);
				}
			}

			if (p_Nickname->flags & NICK_IS_ADMIN)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_admin_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname);
				}
			}

			if (p_Nickname->flags & NICK_IS_IDENTIFIED)
			{
				module_callback_p = ModuleCallback_Find (module_p, "nickinfo_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname);
				}
			}
		}

		p_NicknameChannel = p_Nickname->channel_h;
		while (p_NicknameChannel)
		{
			p_Channel = p_NicknameChannel->channel_p;
			ChannelNickname_Del (p_Channel, p_Nickname);
			NicknameChannel_Del (p_Nickname, p_Channel);

			p_NicknameChannel = p_NicknameChannel->next;
		}

		p_Hostname = p_Nickname->hostname_p;
		assert (p_Hostname != NULL);
		assert (p_Hostname->hostname != NULL);

		if (p_Hostname->numHosts > 1)
		{
			p_Hostname->numHosts--;
		}
		else
		{
			Hostname_Del (p_Hostname->hostname);
		}

		Nickname_Del (p_Nickname->nickname);
	}

	if (argv[5][0] != '+')
	{
		i_is_oper = 0;
	}
	else
	{
		while (*++argv[5] != '\0')
		{
			if (*argv[5] == 'o')
			{
				i_is_oper = 1;
			}
		}
		strtok (NULL, " ");
	}
	argv[6] = strtok (NULL, " ");		/* Nicknames Numeric */

	p_Server = Server_Find (argv[0]);
	assert (p_Server != NULL);
	assert (p_Server->name != NULL);
	assert (p_Server->numeric != NULL);



	p_Hostname = Hostname_Find (argv[4]);
	if (p_Hostname)
	{
		p_Hostname->numHosts++;
	}
	else
	{
		p_Hostname = Hostname_Add (argv[4]);
		assert (p_Hostname != NULL);
		assert (p_Hostname->hostname != NULL);
	}


	p_Nickname = Nickname_Add (argv[1], argv[3], p_Hostname, p_Server, t_timestamp);
	assert (p_Nickname != NULL);
	assert (p_Nickname->nickname != NULL);

	p_Nickname->numeric = (char *)ircsp_calloc (1, strlen (argv[6]) + 5);
	assert (p_Nickname->numeric != NULL);

	strcpy (p_Nickname->numeric, argv[6]);


	if (i_is_oper)
	{
		p_Nickname->flags |= NICK_IS_OPER;
		p_Operator = Operator_Add (p_Nickname);

		assert (p_Operator != NULL);
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Run any m_nick_new callbacks on our service modules
	-------------------------------------------------------------------------------------------------------------------------
	*/
	module_p = module_h;
	while (module_p)
	{
		if (module_p->ModuleType & MODULETYPE_SERVICE)
		{
			module_callback_p = ModuleCallback_Find (module_p, "m_nick_new");
			if (module_callback_p)
			{
				module_callback_p->func (module_p, p_Nickname);
			}
		}

		module_p = module_p->next;
	}

	ircsp_free (c_nickname);

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_nick_change ()
                |
 DESCRIPTION    |    Undernet Protocol (m_nick)
                |    This function will handle Server Message: NICK [N]
                |
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
static int
libundernet_m_nick_change (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t			*p_Nickname;
	nickname_t			*p_Nickname_new;
	nickname_channel_t		*p_NicknameChannel;
	nickname_channel_t		*p_NicknameChannel_update;
	channel_t			*p_Channel;
	channel_nickname_t		*p_ChannelNickname_update;
	hostname_t			*p_Hostname;
	operator_t			*p_Operator;
	server_t			*p_Server;
	module_t			*module_p;
	module_callback_t		*module_callback_p;
	char				*argv[9];
	char				*c_nickname;
	time_t				t_timestamp;


	assert (sBuf != NULL);


	c_nickname = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_nickname != NULL);
	strcpy (c_nickname, sBuf);			/* Make a copy of sBuf in case we need it un modified for whatever reason. */


	argv[0] = strtok (sBuf, " ");			/* This could either be the a Server Numeric or a Nickname Numeric
							   Depending on if this is a new user or a nick change. */
	strtok (NULL, " ");				/* NICK [N] Command, we can ignore this. */


	/*
	---------------------------------------------------------------------------------------------------------
	 NICK CHANGE

	    __BUFFER__: ADAAC N fuck 1572734221
	---------------------------------------------------------------------------------------------------------
	*/
	argv[1] = strtok (NULL, " ");		/* New Nickname */


	/* First we are going to check and make sure the new nickname isn't already in Nickname_List */
	p_Nickname_new = Nickname_Find (argv[1]);
	if (p_Nickname_new != NULL)
	{
		/* The new nickname was found, lets try to correct this. */
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Duplicate nickname [%s]... Attempting to correct\n",
			__FILE__, __LINE__, __FUNCTION__, p_Nickname_new->nickname);

		if (p_Nickname_new->flags & NICK_IS_OPER)
		{
			Operator_Del (p_Nickname_new);
			p_Nickname_new->flags &= ~NICK_IS_OPER;
		}

		module_p = module_find_nickserv ();
		if (module_p)
		{
			if (p_Nickname_new->flags & NICK_IS_ROOT)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_root_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname_new);
				}
			}

			if (p_Nickname_new->flags & NICK_IS_ADMIN)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_admin_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname_new);
				}
			}

			if (p_Nickname_new->flags & NICK_IS_IDENTIFIED)
			{
				module_callback_p = ModuleCallback_Find (module_p, "nickinfo_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (p_Nickname_new);
				}
			}
		}

		p_NicknameChannel = p_Nickname_new->channel_h;
		while (p_NicknameChannel)
		{
			p_Channel = p_NicknameChannel->channel_p;
			ChannelNickname_Del (p_Channel, p_Nickname);
			NicknameChannel_Del (p_Nickname, p_Channel);

			p_NicknameChannel = p_NicknameChannel->next;
		}

		p_Hostname = p_Nickname_new->hostname_p;
		assert (p_Hostname != NULL);
		assert (p_Hostname->hostname != NULL);

		if (p_Hostname->numHosts > 1)
		{
			p_Hostname->numHosts--;
		}
		else
		{
			Hostname_Del (p_Hostname->hostname);
		}


		Nickname_Del (p_Nickname_new->nickname);
	}


	/* Lets find the current nickname in the Nickname_List */
	p_Nickname = Nickname_Find (argv[0]);
	assert (p_Nickname != NULL);
	assert (p_Nickname->nickname != NULL);
	assert (p_Nickname->numeric != NULL);
	assert (p_Nickname->hostname_p != NULL);
	assert (p_Nickname->server_p != NULL);


	p_Hostname = p_Nickname->hostname_p;
	assert (p_Hostname != NULL);
	assert (p_Hostname->hostname != NULL);


	p_Nickname_new = Nickname_Add (argv[1], p_Nickname->username, p_Hostname, p_Nickname->server_p, p_Nickname->stamp);
	assert (p_Nickname_new != NULL);
	assert (p_Nickname_new->nickname != NULL);
	assert (p_Nickname->hostname_p != NULL);
	assert (p_Nickname->server_p != NULL);


	p_Nickname_new->numeric = (char *)ircsp_calloc (1, strlen (p_Nickname->numeric) + 5);
	assert (p_Nickname_new->numeric != NULL);


	strcpy (p_Nickname_new->numeric, p_Nickname->numeric);

	if (p_Nickname->flags & NICK_IS_OPER)
	{
		/* User was an IRC Operator, lets update flags. */
		p_Nickname->flags &= ~NICK_IS_OPER;
		Operator_Del (p_Nickname);

		p_Nickname_new->flags |= NICK_IS_OPER;
		p_Operator = Operator_Add (p_Nickname_new);

		assert (p_Operator != NULL);
	}

	/* Update channel records */
	p_NicknameChannel = p_Nickname->channel_h;
	while (p_NicknameChannel)
	{
		p_Channel = p_NicknameChannel->channel_p;

		p_NicknameChannel_update = NicknameChannel_Add (p_Nickname_new, p_Channel);
		p_ChannelNickname_update = ChannelNickname_Add (p_Channel, p_Nickname_new);

		assert (p_NicknameChannel_update != NULL);
		assert (p_ChannelNickname_update != NULL);

		p_NicknameChannel_update->flags = p_NicknameChannel->flags;
		p_ChannelNickname_update->flags = p_NicknameChannel->flags;

		NicknameChannel_Del (p_Nickname, p_Channel);
		ChannelNickname_Del (p_Channel, p_Nickname);

		p_NicknameChannel = p_NicknameChannel->next;
	}


	module_p = module_h;
	while (module_p)
	{
		if (module_p->ModuleType & MODULETYPE_SERVICE)
		{
			module_callback_p = ModuleCallback_Find (module_p, "m_nick_change");
			if (module_callback_p)
			{
				module_callback_p->func (module_p, p_Nickname_new, p_Nickname);
			}
		}

		module_p = module_p->next;
	}


	/* And finally we can delete the old nickname! */
	Nickname_Del (p_Nickname->nickname);


	ircsp_free (c_nickname);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_nick ()
                |
 DESCRIPTION    |    Undernet Protocol (m_nick)
                |    This function will handle Server Message: NICK [N]
                |
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_nick (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	char				*argv[1];
	char				*c_nickname;
	int				i_count;

	assert (sBuf != NULL);


	c_nickname = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_nickname != NULL);
	strcpy (c_nickname, sBuf);			/* Make a copy of sBuf in case we need it un modified for whatever reason. */


	argv[0] = strtok (sBuf, " ");			/* This could either be the a Server Numeric or a Nickname Numeric
							   Depending on if this is a new user or a nick change. */

	i_count = strlen (argv[0]);
	if (i_count == 2)
	{
		/*
		------------------------------------------------------------------------------------------------------------------------------------------------
		 NEW NICKNAME!

		    We check the length of argv[0] if its 2 we have a Server Numeric which means this is a new user
		    connecting to the server matching that numeric.

		    __BUFFER__: AD N shitholez 1 1572734200 ~jovens cthulhu.gotunix.net CsEAA8 ADAAC :Justin Ovens
		------------------------------------------------------------------------------------------------------------------------------------------------
		*/
		libundernet_m_nick_new (c_nickname);

	}
	else
	{
		/*
		---------------------------------------------------------------------------------------------------------
		 NICK CHANGE

		    __BUFFER__: ADAAC N fuck 1572734221
		---------------------------------------------------------------------------------------------------------
		*/
		libundernet_m_nick_change (c_nickname);
	}


	ircsp_free (c_nickname);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
