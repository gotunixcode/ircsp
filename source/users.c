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
#include	<signal.h>
#include	<errno.h>


/* IRCSP Core Includes */
#include	"alloc.h"
#include	"log.h"
#include	"ircsp.h"
#include	"signals.h"
#include	"users.h"
#include	"channels.h"
#include	"servers.h"
#include	"conf.h"
#include	"modules.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


const char convert2y[] = {
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
};


/* Converts a character to its (base64) numnick value. */
const unsigned int convert2n[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
	0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    base64toint ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
unsigned int base64toint (s)
	const char *s;
{
	unsigned int i = convert2n[(unsigned char) *s++];
	while (*s)
	{
		i <<= NUMNICKLOG;
		i += convert2n[(unsigned char) *s++];
	}
	return i;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    intobase64
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char* intobase64 (buf, v, count)
	char *buf;
	unsigned int v;
	unsigned int count;
{
	buf[count] = '\0';
	while (count > 0)
	{
		buf[--count] = convert2y[(v & NUMNICKMASK)];
		v >>= NUMNICKLOG;
	}
	return buf;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Nickname_Add ()
                |
 DESCRIPTION    |    This function will add a new nickname to our Nickname_List.
                |
                |    @TODO:
                |        We need to change the server variable to use a _server_list pointer to save memory.
                |
                |    @CHANGELOG:
                |        [2019/10/25] Updated to allow the numeric variable to be NULL to support other protocols.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_t *Nickname_Add (nickname, username, p_Hostname, p_Server, timestamp)
	char			*nickname;
	char			*username;
	struct _hostname	*p_Hostname;
	struct _server		*p_Server;
	time_t			timestamp;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_Nickname;

	p_Nickname = (nickname_t *)ircsp_calloc (1, sizeof (nickname_t));
	if (!p_Nickname)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for User_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	p_Nickname->nickname = (char *)ircsp_calloc (1, strlen (nickname) + 5);
	if (!p_Nickname->nickname)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for User_p->nickname\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	p_Nickname->username = (char *)ircsp_calloc (1, strlen (username) + 5);
	if (!p_Nickname->username)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for User_p->username\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}


	strcpy (p_Nickname->nickname, nickname);
	strcpy (p_Nickname->username, username);

	p_Nickname->hostname_p		= p_Hostname;
	p_Nickname->server_p		= p_Server;
	p_Nickname->numeric		= NULL;
	p_Nickname->flags		= 0;
	p_Nickname->stamp		= timestamp;
	p_Nickname->numChans		= 0;
	p_Nickname->channel_h		= NULL;
	p_Nickname->admin_p		= NULL;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: User [%s] added at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname, p_Nickname->nickname
		);
	}

	if (!nickname_h)
	{
		nickname_h = p_Nickname;
		p_Nickname->next = NULL;
	}
	else
	{
		p_Nickname->next = nickname_h;
		nickname_h = p_Nickname;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_Nickname;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Nickname_Find ()
                |
 DESCRIPTION    |    This function is used to search our nickname list for a specific nickname.
                |
                |       [2019/10/25] Adding support for numerics so we can DEPRECIATE ircsp_nickname_find_by_numeric ()
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_t *Nickname_Find (search)
	char		*search;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t	*p_Nickname;

	for (p_Nickname = nickname_h; p_Nickname; p_Nickname = p_Nickname->next)
	{
		/* Lets check and see if the nick entry has a numeric assigned or not */

		if (p_Nickname->numeric)
		{
			if ((!strcasecmp (search, p_Nickname->nickname)) || (!strcmp (search, p_Nickname->numeric)))
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: NICKNAME [%s] found at [%p] USING [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname,
							p_Nickname->nickname, search
						);
					}

					if (DEBUG_FUNC)
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}
				}

				return p_Nickname;
			}
		}
		else
		{
			if (!strcasecmp(search, p_Nickname->nickname))
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: NICKNAME [%s] found at [%p] USING [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname,
							p_Nickname->nickname, search
						);
					}

					if (DEBUG_FUNC)
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}
				}

				return p_Nickname;
			}
		}
	}


	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Nickname_Del ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void Nickname_Del (char *nick)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t	*p_Nickname;
	nickname_t	*p_Next;
	nickname_t	*p_Delete;

	for (p_Nickname = nickname_h; p_Nickname; p_Nickname = p_Nickname->next)
	{
		p_Next = p_Nickname->next;
		if (!strcasecmp(p_Nickname->nickname, nick))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting nickname [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname,
					p_Nickname->nickname
				);
			}

			if (nickname_h == p_Nickname)
			{
				nickname_h = p_Nickname->next;
			}
			else
			{
				for (p_Delete = nickname_h; (p_Delete->next != p_Nickname) && p_Delete; p_Delete = p_Delete->next);
				p_Delete->next = p_Nickname->next;
			}

			ircsp_free (p_Nickname->nickname);
			ircsp_free (p_Nickname->username);
			p_Nickname->hostname_p = NULL;
			ircsp_free (p_Nickname->numeric);
			p_Nickname->server_p = NULL;
			ircsp_free (p_Nickname);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Nickname_DelServer ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void Nickname_DelServer (p_Server)
	struct _server		*p_Server;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_Nickname;
	hostname_t		*p_Hostname;
	channel_t		*p_Channel;
	nickname_channel_t	*p_NicknameChannel;
	module_t		*module_p;
	module_callback_t	*module_callback_p;

	for (p_Nickname = nickname_h; p_Nickname; p_Nickname = p_Nickname->next)
	{
		if (p_Server == p_Nickname->server_p)
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting nickname [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname,
					p_Nickname->nickname
				);
			}


			if (!p_Nickname->hostname_p)
			{
				/* User doesn't have a hostname pointer, this is a problem. */

				if ((DEBUG) && (DEBUG_LIST))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Nickname [%s] did not have a hostname pointer!\n",
						__FILE__, __LINE__, __FUNCTION__, p_Nickname->nickname
					);
				}
			}
			else
			{
				p_Hostname = p_Nickname->hostname_p;
				if (p_Hostname->numHosts > 1)
				{
					// Not the last user using this hostname so we decrease numHosts
					if ((DEBUG) && (DEBUG_LIST))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Hostname [%s] - Decreasing host count\n",
							__FILE__, __LINE__, __FUNCTION__, p_Hostname->hostname
						);
					}

					p_Hostname->numHosts--;
				}
				else
				{
					if ((DEBUG) && (DEBUG_LIST))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Deleting hostname [%s] found at [%p]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Hostname->hostname,
							p_Hostname->hostname
						);
					}

					Hostname_Del (p_Hostname->hostname);
					p_Nickname->hostname_p = NULL;
					status->numHosts--;
				}
			}

			p_Server->numUsers--;
			status->numUsers--;


			module_p = module_find_nickserv ();
			if (module_p)
			{
				if (p_Nickname->flags & NICK_IS_IDENTIFIED)
				{
					module_callback_p = ModuleCallback_Find (module_p, "nickinfo_unregister");
					if (module_callback_p)
					{
						module_callback_p->func (p_Nickname);
					}
				}

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
			}

			if (p_Nickname->flags & NICK_IS_OPER)
			{
				p_Nickname->flags &= ~NICK_IS_OPER;
				if (p_Server)
					p_Server->numOpers--;
				status->numOpers--;
			}

			for (p_NicknameChannel = p_Nickname->channel_h; p_NicknameChannel; p_NicknameChannel = p_NicknameChannel->next)
			{
				p_Channel = p_NicknameChannel->channel_p;
				ChannelNickname_Del (p_Channel, p_Nickname);
				NicknameChannel_Del (p_Nickname, p_Channel);
			}

			Nickname_Del (p_Nickname->nickname);
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Hostname_Add ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
hostname_t *Hostname_Add (hostname)
	char			*hostname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	hostname_t		*p_Hostname;


	p_Hostname = (hostname_t *)ircsp_calloc (1, sizeof (hostname_t));
	if (!p_Hostname)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for Hostname_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	p_Hostname->hostname = (char *)ircsp_calloc (1, strlen (hostname) + 5);
	if (!p_Hostname->hostname)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for Hostname_p->hostname\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	strcpy(p_Hostname->hostname, hostname);
	p_Hostname->numHosts = 1;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Hostname [%s] added at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__,
			p_Hostname->hostname, p_Hostname->hostname
		);
	}

	if (!hostname_h)
	{
		hostname_h = p_Hostname;
		p_Hostname->next = NULL;
	}
	else
	{
		p_Hostname->next = hostname_h;
		hostname_h = p_Hostname;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_Hostname;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Hostname_Find ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
hostname_t *Hostname_Find (hostname)
	char			*hostname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	hostname_t		*p_Hostname;

	for (p_Hostname = hostname_h; p_Hostname; p_Hostname = p_Hostname->next)
	{
		if (!strcasecmp(p_Hostname->hostname, hostname))
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Hostname [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Hostname->hostname,
						p_Hostname->hostname
					);
				}

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}
			}

			return p_Hostname;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Hostname_Del ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void Hostname_Del (hostname)
	char			*hostname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	hostname_t		*p_Hostname;
	hostname_t		*p_Next;
	hostname_t		*p_Delete;

	for (p_Hostname = hostname_h; p_Hostname; p_Hostname = p_Next)
	{
		p_Next = p_Hostname->next;
		if (!strcasecmp(p_Hostname->hostname, hostname))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting hostname [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, p_Hostname->hostname,
					p_Hostname->hostname
				);
			}

			if (hostname_h == p_Hostname)
			{
				hostname_h = p_Hostname->next;
			}
			else
			{
				for (p_Delete = hostname_h; (p_Delete->next != p_Hostname) && p_Delete; p_Delete = p_Delete->next);
				p_Delete->next = p_Hostname->next;
			}

			ircsp_free (p_Hostname->hostname);
			ircsp_free (p_Hostname);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Operator_Add ()
                |
 DESCRIPTION    |    This function will initialize a new pointer in our Operator_List
                |
                |    @TODO:
                |
                |    @CHANGELOG:
                |        [2019/10/27] Function created.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
operator_t *Operator_Add (p_Nickname)
	struct _nickname	*p_Nickname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	operator_t		*p_Operator;

	p_Operator = (operator_t *)ircsp_calloc(1, sizeof (operator_t));
	if (!p_Operator)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for Operator_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	p_Operator->nickname_p = p_Nickname;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Operator [%s] added to Operator List at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, p_Operator->nickname_p->nickname,
			p_Operator->nickname_p->nickname
		);
	}

	if (!operator_h)
	{
		operator_h = p_Operator;
		p_Operator->next = NULL;
	}
	else
	{
		p_Operator->next = operator_h;
		operator_h = p_Operator;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_Operator;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Operator_Find ()
                |
 DESCRIPTION    |    This function will searches for a pointer in our Operator_List
                |
                |    @TODO:
                |
                |    @CHANGELOG:
                |        [2019/10/27] Function created.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
operator_t *Operator_Find (p_Nickname)
	struct _nickname		*p_Nickname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	operator_t			*p_Operator;

	for (p_Operator = operator_h; p_Operator; p_Operator = p_Operator->next)
	{
		if (p_Operator->nickname_p == p_Nickname)
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Operator [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Operator->nickname_p->nickname,
						p_Operator->nickname_p->nickname
					);
				}
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return p_Operator;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Operator_Del ()
                |
 DESCRIPTION    |    This function will deletes a pointer in our Operator_List
                |
                |    @TODO:
                |
                |    @CHANGELOG:
                |        [2019/10/27] Function created.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void Operator_Del (p_Nickname)
	struct _nickname		*p_Nickname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	operator_t			*p_Operator;
	operator_t			*p_Next;
	operator_t			*p_Delete;


	for (p_Operator = operator_h; p_Operator; p_Operator = p_Operator->next)
	{
		p_Next = p_Operator->next;
		if (p_Operator->nickname_p == p_Nickname)
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting operator [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, p_Operator->nickname_p->nickname,
					p_Operator
				);
			}

			if (operator_h == p_Operator)
			{
				operator_h = p_Operator->next;
			}
			else
			{
				for (p_Delete = operator_h; (p_Delete->next != p_Operator) && p_Delete; p_Delete = p_Delete->next);
				p_Delete->next = p_Operator->next;
			}

			ircsp_free (p_Operator);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameChannel_Add ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_channel_t
*NicknameChannel_Add (userp, channelp)
	struct _nickname	*userp;
	struct _channel		*channelp;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_channel_t *chanlistp;

	chanlistp = (nickname_channel_t *)ircsp_calloc(1, sizeof(nickname_channel_t));
	if (!chanlistp)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for NicknameChannel_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	chanlistp->channel_p = channelp;
	chanlistp->flags = 0;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] added to Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname,
			channelp->name
		);
	}

	if (!userp->channel_h)
	{
		userp->channel_h = chanlistp;
		chanlistp->next = NULL;
	}
	else
	{
		chanlistp->next = userp->channel_h;
		userp->channel_h = chanlistp;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return chanlistp;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameChannel_Find ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_channel_t *NicknameChannel_Find (struct _nickname *userp, struct _channel *channelp)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickname_channel_t *chanlistp;
	chanlistp = userp->channel_h;
	while (chanlistp)
	{
		if (chanlistp->channel_p == channelp)
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Nickname [%s] was found in Channel [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, userp->nickname, channelp->name
					);
				}

				if (DEBUG_FUNC)
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}
			}

			return chanlistp;
		}

		chanlistp = chanlistp->next;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameChannel_Del ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void NicknameChannel_Del (struct _nickname *userp, struct _channel *channelp)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickname_channel_t *chanlistp;
	nickname_channel_t *prev;

	prev = NULL;
	chanlistp = userp->channel_h;
	while (chanlistp)
	{
		if (chanlistp->channel_p == channelp)
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting Nickname [%s] from Channel [%s]\n",
					__FILE__, __LINE__, __FUNCTION__,
					userp->nickname, channelp->name
				);
			}

			if (!prev)
			{
				userp->channel_h = chanlistp->next;
			}
			else
			{
				prev->next = chanlistp->next;
			}

			ircsp_free (chanlistp);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
		prev = chanlistp;
		chanlistp = chanlistp->next;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameNumeric_Add ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_numeric_t *NicknameNumeric_Add (numeric)
	char *numeric;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_numeric_t *nnumericp;


	nnumericp = (nickname_numeric_t *)ircsp_calloc(1, sizeof(nickname_numeric_t));
	if (!nnumericp)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for NicknameNumeric_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	nnumericp->numeric = (char *)ircsp_calloc(1, strlen(numeric) + 5);
	if (!nnumericp->numeric)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for NicknameNumeric_p->numeric\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	strcpy (nnumericp->numeric, numeric);

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: NicknameNumeric [%s] added at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__,
			nnumericp->numeric, nnumericp->numeric
		);
	}

	if (!nickname_numeric_h)
	{
		nickname_numeric_h = nnumericp;
		nnumericp->next = NULL;
	}
	else
	{
		nnumericp->next = nickname_numeric_h;
		nickname_numeric_h = nnumericp;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return nnumericp;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameNumeric_Find ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
nickname_numeric_t *NicknameNumeric_Find (numeric)
	char *numeric;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_numeric_t *nnumericp;

	nnumericp = nickname_numeric_h;
	while (nnumericp)
	{
		if (!strcasecmp(nnumericp->numeric, numeric))
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: NicknameNumeric [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__,
						nnumericp->numeric, nnumericp->numeric
					);
				}

				if (DEBUG_FUNC)
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}
			}

			return nnumericp;
		}

		nnumericp = nnumericp->next;
	}

 	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameNumeric_Del ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void NicknameNumeric_Del (numeric)
	char *numeric;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_numeric_t *nnumericp, *next, *delete;

	for (nnumericp = nickname_numeric_h; nnumericp; nnumericp = next)
	{
		next = nnumericp->next;
		if (!strcasecmp(nnumericp->numeric, numeric))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting NicknameNumeric [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__,
					nnumericp->numeric, nnumericp->numeric
				);
			}

			if (nickname_numeric_h == nnumericp)
			{
				nickname_numeric_h = nnumericp->next;
			}
			else
			{
				for (delete = nickname_numeric_h; (delete->next != nnumericp) && delete; delete = delete->next);
				delete->next = nnumericp->next;
			}

			ircsp_free (nnumericp->numeric);
			ircsp_free (nnumericp);

			status->numNumeric--;

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    NicknameNumeric_Generate ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *NicknameNumeric_Generate (numeric)
	char *numeric;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_numeric_t *nnumericp;

	int yyPos = 0;
	int yPos = 0;
	int xPos = 0;

	numeric[0] = config->server_numeric[0];
	numeric[1] = config->server_numeric[1];


	for (xPos = 0; xPos < config->xPos; xPos++)
	{
		for (yPos = 0; yPos < config->yPos; yPos++)
		{
			for (yyPos = 0; yyPos < config->yyPos; yyPos++)
			{
				numeric[2] = convert2y[xPos];
				numeric[3] = convert2y[yPos];
				numeric[4] = convert2y[yyPos];
				numeric[5] = '\0';

				nnumericp = NicknameNumeric_Find (numeric);
				if (!nnumericp)
				{
					if (DEBUG)
					{
						if (DEBUG_LIST)
						{
							ircsp_log (F_MAINLOG,
								"[%s:%d:%s()]: Available NicknameNumeric [%s] found\n",
								__FILE__, __LINE__, __FUNCTION__,
								numeric
							);
						}

						if (DEBUG_FUNC)
						{
							ircsp_log (F_MAINLOG,
								get_log_message (LOG_MESSAGE_FUNCEND),
								__FILE__, __LINE__, __FUNCTION__
							);
						}
					}

					return numeric;
				}

			}
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}
