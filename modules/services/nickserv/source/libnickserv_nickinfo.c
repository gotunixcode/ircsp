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
#include	<stdarg.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<string.h>
#include	<assert.h>
#include	<time.h>
#include	<errno.h>
#include	<mariadb/mysql.h>

#if defined (LINUX)
#include	<crypt.h>
#endif


/* IRCSP Core Includes */
#include	"users.h"
#include	"channels.h"
#include	"conf.h"
#include	"alloc.h"
#include	"log.h"
#include	"text.h"
#include	"ircsp.h"
#include	"misc.h"
#include	"match.h"
#include	"modules.h"
#include	"mysql.h"


/* NickServ Module Includes */
#include	"libnickserv.h"
#include	"libnickserv_admin.h"
#include	"libnickserv_commands.h"
#include	"libnickserv_nickinfo.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    Nickinfo_Create ()
                   |
    DESCRIPTION    |    This function will add a registered nickname into our NickServ linked list.  This list is used to keep
                   |    track of nicknames that are registered, their flags.  Data is also saved from here into our MySQL
                   |    nickserv database, or loaded from the mysql database to this list for caching.
                   |
    INPUTS         |    nickname = Nickname for the Nickinfo object we are creating
                   |
    RETURNS        |    Returns a pointer to the nickname added if successful, otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_t
*Nickinfo_Create (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_t	*nickinfo_p;


	assert (nickname != NULL);


	nickinfo_p = (nickinfo_t *)ircsp_calloc (1, sizeof (nickinfo_t));
	if (!nickinfo_p)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	nickinfo_p->nickname = (char *)ircsp_calloc (1, strlen (nickname) + 5);
	if (!nickinfo_p->nickname)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	strcpy (nickinfo_p->nickname, nickname);
	nickinfo_p->password		= NULL;
	nickinfo_p->last_login		= NULL;
	nickinfo_p->last_seen		= 0;
	nickinfo_p->registered		= 0;
	nickinfo_p->flags		= 0L;
	nickinfo_p->nickname_p		= NULL;
	nickinfo_p->nickinfo_access_h	= NULL;


	ircsp_log (F_MAINLOG,
		"[%s:%d:%s()]: NickServ registered nickname [%s] added at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, nickinfo_p->nickname,
		nickinfo_p->nickname
	);


	if (!nickinfo_h)
	{
		nickinfo_h = nickinfo_p;
		nickinfo_p->next = NULL;
	}
	else
	{
		nickinfo_p->next = nickinfo_h;
		nickinfo_h = nickinfo_p;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return nickinfo_p;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    Nickinfo_Search ()
                   |
    DESCRIPTION    |    This function searches our NickServ NickInfo List for a specific nickname
                   |
    RETURNS        |    Returns NULL if nothing is found otherwise we return the pointer
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_t
*Nickinfo_Search (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_t	*nickinfo_p;


	assert (nickname != NULL);


	nickinfo_p = nickinfo_h;
	while (nickinfo_p)
	{
		if (!strcasecmp (nickinfo_p->nickname, nickname))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: NickServ registered " \
				"nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, nickinfo_p->nickname,
				nickinfo_p->nickname
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return nickinfo_p;
		}


		nickinfo_p = nickinfo_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    Nickinfo_Remove ()
                   |
    DESCRIPTION    |    This function will remove a nickname from the NickServ NickInfo List
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
Nickinfo_Remove (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_t	*nickinfo_p;
	nickinfo_t	*nickinfo_delete_p;
	nickname_t	*nickname_p;


	assert (nickname != NULL);


	nickinfo_p = nickinfo_h;
	while (nickinfo_p)
	{
		if (!strcasecmp (nickinfo_p->nickname, nickname))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Deleting NickServ " \
				"registered nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, nickinfo_p->nickname,
				nickinfo_p->nickname
			);


			nickname_p = Nickname_Find (nickinfo_p->nickname);
			if (nickname_p)
			{
				if (nickname_p->flags & NICK_IS_IDENTIFIED)
				{
					nickname_p->flags &= ~NICK_IS_IDENTIFIED;
				}
			}


			if (nickinfo_h == nickinfo_p)
			{
				nickinfo_h = nickinfo_p->next;
			}
			else
			{
				for (nickinfo_delete_p = nickinfo_h; (nickinfo_delete_p != nickinfo_p) &&
					nickinfo_delete_p; nickinfo_delete_p = nickinfo_delete_p->next);

				nickinfo_delete_p->next = nickinfo_p->next;
			}


			ircsp_free (nickinfo_p->nickname);

			if (nickinfo_p->password)
			{
				ircsp_free (nickinfo_p->password);
			}

			if (nickinfo_p->last_login)
			{
				ircsp_free (nickinfo_p->last_login);
			}

			nickinfo_p->last_seen		= 0;
			nickinfo_p->registered		= 0;
			nickinfo_p->flags		= 0L;
			nickinfo_p->nickinfo_access_h	= NULL;
			ircsp_free (nickinfo_p);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return ;
		}


		nickinfo_p = nickinfo_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_unregister ()
                   |
    DESCRIPTION    |    This function is a callback function used by things outside of this module to unregister an IDENTIFIED
                   |    NickServ nickname, this doesn't DROP the nickname, it simply removes the IDENTIFY flag, and NULLs
                   |    pointers from the NickInfo linked list to the Nickname list.
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
nickinfo_unregister (nickname_p)
	struct _nickname	*nickname_p;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickinfo_t		*nickinfo_p;

	nickinfo_p = nickinfo_h;
	while (nickinfo_p)
	{
		if (nickinfo_p->nickname, nickname_p->nickname)
		{
			if (nickname_p->flags & NICK_IS_IDENTIFIED)
			{
				nickinfo_p->nickname_p = NULL;
				nickname_p->flags &= ~NICK_IS_IDENTIFIED;
			}

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}

		nickinfo_p = nickinfo_p->next;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_expire ()
                   |
    DESCRIPTION    |    This function is goes through all NickServ registered nicknames and removes any nicknames that have
                   |    not IDENTIFIED in a set amount of time.
                   |
                   |        NICKSERV_EXPIRE_CYCLE - This is the variable on how often we check for expired nicknames
                   |        NICKSERV_EXPIRE       - This is the variable on how long before we remove a unused nickname.
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
int
nickinfo_expire (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	nickname_t		*nickname_p;

	nickinfo_p = nickinfo_h;
	while (nickinfo_p)
	{
		nickname_p = Nickname_Find (nickinfo_p->nickname);
		if (nickname_p)
		{
			if (nickname_p->flags & NICK_IS_IDENTIFIED)
			{
				nickinfo_p->last_seen = time (NULL);
			}
		}

		if ((time (NULL) - nickinfo_p->last_seen) > NICKSERV_EXPIRE)
		{
			nickinfo_access_p = nickinfo_p->nickinfo_access_h;
			while (nickinfo_access_p)
			{
				ircsp_mysql_delete (NSDB_NICKSERV_ACCESS, "nickname=\"%s\" AND hostmask=\"%s\"",
					nickinfo_p->nickname, nickinfo_access_p->hostmask
				);

				NickinfoAccess_Remove (nickinfo_p, nickinfo_access_p->hostmask);

				nickinfo_access_p = nickinfo_access_p->next;
			}

			ircsp_mysql_delete (NSDB_NICKSERV, "nickname=\"%s\"",
				nickinfo_p->nickname
			);

			Nickinfo_Remove (nickinfo_p->nickname);
		}

		nickinfo_p = nickinfo_p->next;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_validate ()
                   |
    DESCRIPTION    |    This function is goes through all NickServ registered nicknames and if a nick is online but
                   |    not identified will keep harassing the nick to IDENTIFY.  If said nick has KILL PROTECTION
                   |    enabled, after 5 minutes the nickname will be removed from the network and JUPED.
                   |
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
int
nickinfo_validate (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_jupe_t		*nickinfo_jupe_p;


	nickinfo_p = nickinfo_h;
	while (nickinfo_p)
	{
		nickname_p = Nickname_Find (nickinfo_p->nickname);
		if (nickname_p)
		{
			if (!(nickname_p->flags & NICK_IS_IDENTIFIED))
			{
				if (!(nickname_p->flags & NICK_IS_JUPE))
				{
					cmd_notice (config->nickserv_p->nickname, nickname_p->nickname,
						"This nickname is owned by someone else. Please choose another"
					);

					cmd_notice (config->nickserv_p->nickname, nickname_p->nickname,
						"If this is your nickname type: \002/msg %s IDENTIFY password\002",
						config->nickserv_p->nickname
					);

					if (nickinfo_p->flags & NICKSERV_KILLPROTECT)
					{
						if ((time (NULL) - nickname_p->stamp) > 300)
						{
							cmd_kill (config->nickserv_p->nickname, nickname_p->nickname, "Failed to identify for nickname");
							cmd_nickjupe (nickinfo_p->nickname, config->nickserv_p->username, config->server_name, "NickServ jupe");
							nickname_p = Nickname_Find (nickinfo_p->nickname);
							nickinfo_jupe_p = NickinfoJupe_Create (nickinfo_p->nickname);
							if (nickinfo_jupe_p)
							{
								nickinfo_jupe_p->nickinfo_p = nickinfo_p;
								nickinfo_jupe_p->nickname_p = nickname_p;
							}
						}
						else
						{
							cmd_notice (config->nickserv_p->nickname, nickname_p->nickname,
								"You will be automatically removed in \002%d\002 seconds",
								300 - (time (NULL) - nickname_p->stamp)
							);
						}
					}
				}
			}

			if (nickname_p->flags & NICK_IS_IDENTIFIED)
			{
				nickinfo_p->last_seen = time (NULL);
			}
		}

		nickinfo_p = nickinfo_p->next;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoAccess_Create ()
                   |
    DESCRIPTION    |    This function will add a hostname to the nickserv access list for users that have the flag set to only
                   |    allow specific hostnames to identify for their nicks
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_access_t
*NickinfoAccess_Create (nickinfo_p, hostmask)
	struct _nickinfo	*nickinfo_p;
	char			*hostmask;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_access_t	*nickinfo_access_p;


	assert (nickinfo_p != NULL);
	assert (hostmask != NULL);


	nickinfo_access_p = nickinfo_p->nickinfo_access_h;
	nickinfo_access_p = (nickinfo_access_t *)ircsp_calloc (1, sizeof (nickinfo_access_t));
	if (!nickinfo_access_p)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	nickinfo_access_p->hostmask = (char *)ircsp_calloc (1, strlen (hostmask) + 5);
	if (!nickinfo_access_p->hostmask)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	strcpy (nickinfo_access_p->hostmask, hostmask);
	nickinfo_access_p->nickinfo_p = nickinfo_p;


	ircsp_log (F_MAINLOG, "[%s:%d:s()]: Hostmask [%s] added to " \
		"NickServ registered nickname [%s] at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, nickinfo_access_p->hostmask,
		nickinfo_p->nickname, nickinfo_access_p->hostmask
	);


	if (!nickinfo_p->nickinfo_access_h)
	{
		nickinfo_p->nickinfo_access_h = nickinfo_access_p;
		nickinfo_access_p->next = NULL;
	}
	else
	{
		nickinfo_access_p->next = nickinfo_p->nickinfo_access_h;
		nickinfo_p->nickinfo_access_h = nickinfo_access_p;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return nickinfo_access_p;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoAccess_Search ()
                   |
    DESCRIPTION    |    This function will search for a hostname in the nickserv access list for users that have the flag set
                   |    to only allow specific hostnames to identify for their nicks
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_access_t
*NickinfoAccess_Search (nickinfo_p, hostmask)
	struct _nickinfo	*nickinfo_p;
	char			*hostmask;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_access_t	*nickinfo_access_p;


	assert (nickinfo_p != NULL);
	assert (hostmask != NULL);


	nickinfo_access_p = nickinfo_p->nickinfo_access_h;
	while (nickinfo_access_p)
	{
		if (!strcasecmp (nickinfo_access_p->hostmask, hostmask))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Hostmask [%s] for NickServ" \
				"registered nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, nickinfo_access_p->hostmask,
				nickinfo_p->nickname, nickinfo_access_p->hostmask
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return nickinfo_access_p;
		}


		nickinfo_access_p = nickinfo_access_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoAccess_Wildcard ()
                   |
    DESCRIPTION    |    This function will search for a hostname in the nickserv access list using wildcards
                   |    (Ex: user@*.domain.com)
                   |
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_access_t
*NickinfoAccess_Wildcard (nickinfo_p, hostmask)
	struct _nickinfo	*nickinfo_p;
	char			*hostmask;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_access_t	*nickinfo_access_p;


	assert (nickinfo_p != NULL);
	assert (hostmask != NULL);


	nickinfo_access_p = nickinfo_p->nickinfo_access_h;
	while (nickinfo_access_p)
	{
		if (!match (nickinfo_access_p->hostmask, hostmask))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Hostmask [%s] for NickServ"\
				 "registered nickname [%s] found at [%p] using [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, nickinfo_access_p->hostmask,
				nickinfo_p->nickname, nickinfo_access_p->hostmask, hostmask
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return nickinfo_access_p;
		}


		nickinfo_access_p = nickinfo_access_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoAccess_Remove ()
                   |
    DESCRIPTION    |    This function will search for a hostname in the nickserv access list for users that have the flag set
                   |    to only allow specific hostnames to identify for their nicks
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
void
NickinfoAccess_Remove (nickinfo_p, hostmask)
	struct _nickinfo	*nickinfo_p;
	char			*hostmask;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_access_t	*ni_acc_p;
	nickinfo_access_t	*ni_acc_del_p;


	assert (nickinfo_p != NULL);
	assert (hostmask != NULL);


	ni_acc_p = nickinfo_p->nickinfo_access_h;
	while (ni_acc_p)
	{
		if (!strcasecmp (ni_acc_p->hostmask, hostmask))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Deleting hostmask [%s] for NickServ "\
				"registered nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, ni_acc_p->hostmask,
				nickinfo_p->nickname, ni_acc_p->hostmask
			);


			if (nickinfo_p->nickinfo_access_h == ni_acc_p)
			{
				nickinfo_p->nickinfo_access_h = ni_acc_p->next;
			}
			else
			{
				for (ni_acc_del_p = nickinfo_p->nickinfo_access_h; (ni_acc_del_p->next != ni_acc_p) &&
					ni_acc_del_p; ni_acc_del_p = ni_acc_del_p->next);

				ni_acc_del_p->next = ni_acc_p->next;
			}


			ircsp_free (ni_acc_p->hostmask);
			ni_acc_p->nickinfo_p = NULL;
			ircsp_free (ni_acc_p);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return ;
		}


		ni_acc_p = ni_acc_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoJupe_Create ()
                   |
    DESCRIPTION    |    This function is used to keep track of NickServ juped nicknames.  This is used by KILLPROTECTION as well as
                   |    the RECOVER command.
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_jupe_t
*NickinfoJupe_Create (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_jupe_t		*nickinfo_jupe_p;


	assert (nickname != NULL);


	nickinfo_jupe_p = (nickinfo_jupe_t *)ircsp_calloc (1, sizeof (nickinfo_jupe_t));
	if (!nickinfo_jupe_p)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	nickinfo_jupe_p->nickname = (char *)ircsp_calloc (1, strlen (nickname) + 5);
	if (!nickinfo_jupe_p->nickname)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return NULL;
	}


	strcpy (nickinfo_jupe_p->nickname, nickname);
	nickinfo_jupe_p->timestamp = time (NULL);


	ircsp_log (F_MAINLOG,
		"[%s:%d:%s()]: Nickname [%s] has been added to the NickServ JUPE list at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, nickinfo_jupe_p->nickname,
		nickinfo_jupe_p->nickname
	);


	if (!nickinfo_jupe_h)
	{
		nickinfo_jupe_h = nickinfo_jupe_p;
		nickinfo_jupe_p->next = NULL;
	}
	else
	{
		nickinfo_jupe_p->next = nickinfo_jupe_h;
		nickinfo_jupe_h = nickinfo_jupe_p;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return nickinfo_jupe_p;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoJupe_Search ()
                   |
    DESCRIPTION    |    This function will search the NickInfoJupe list for a specific nickname.
                   |
    RETURNS        |    if success pointer to the entry on the list otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
nickinfo_jupe_t
*NickinfoJupe_Search (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_jupe_t		*nickinfo_jupe_p;


	assert (nickname != NULL);


	nickinfo_jupe_p = nickinfo_jupe_h;
	while (nickinfo_jupe_p)
	{
		if (!strcasecmp (nickinfo_jupe_p->nickname, nickname))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: NickServ JUPED nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				nickinfo_jupe_p->nickname, nickinfo_jupe_p->nickname
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return nickinfo_jupe_p;
		}


		nickinfo_jupe_p = nickinfo_jupe_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    NickinfoJupe_Remove ()
                   |
    DESCRIPTION    |    This function will search the NickInfoJupe list and delete a specific nickname.
                   |
    RETURNS        |    NOTHING
---------------------------------------------------------------------------------------------------------------------------------
*/
void
NickinfoJupe_Remove (nickname)
	char		*nickname;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_jupe_t		*nickinfo_jupe_p;
	nickinfo_jupe_t		*nickinfo_jupe_del_p;


	assert (nickname != NULL);


	nickinfo_jupe_p = nickinfo_jupe_h;
	while (nickinfo_jupe_p)
	{
		if (!strcasecmp (nickinfo_jupe_p->nickname, nickname))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Deleting NickServ JUPED nickname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				nickinfo_jupe_p->nickname, nickinfo_jupe_p->nickname
			);


			if (nickinfo_jupe_h == nickinfo_jupe_p)
			{
				nickinfo_jupe_h = nickinfo_jupe_p->next;
			}
			else
			{
				for (nickinfo_jupe_del_p = nickinfo_jupe_h; (nickinfo_jupe_del_p->next != nickinfo_jupe_p) &&
					nickinfo_jupe_del_p; nickinfo_jupe_del_p = nickinfo_jupe_del_p->next
				);

				nickinfo_jupe_del_p->next = nickinfo_jupe_p->next;
			}


			ircsp_free (nickinfo_jupe_p->nickname);
			nickinfo_jupe_p->timestamp		= 0;
			nickinfo_jupe_p->nickname_p		= NULL;
			nickinfo_jupe_p->nickinfo_p		= NULL;
			ircsp_free (nickinfo_jupe_p);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return ;
		}


		nickinfo_jupe_p = nickinfo_jupe_p->next;
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_jupe_expire ()
                   |
    DESCRIPTION    |    This function goes through all juped nicknames and releases them after a set amount of time.
                   |
                   |        NICKSERV_JUPE_EXPIRE_CYCLE  - Is the variable on how often we check for expired jupes
                   |        NICKSERV_JUPE_EXPIRE        - Is the variable on how long before we expire a nickname
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
int
nickinfo_jupe_expire (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickinfo_jupe_t		*nickinfo_jupe_p;

	nickinfo_jupe_p = nickinfo_jupe_h;
	while (nickinfo_jupe_p)
	{
		if ((time (NULL) - nickinfo_jupe_p->timestamp) > NICKSERV_JUPE_EXPIRE)
		{
			cmd_kill (NULL, nickinfo_jupe_p->nickname, "Removing JUPE");
			NickinfoJupe_Remove (nickinfo_jupe_p->nickname);
		}

		nickinfo_jupe_p = nickinfo_jupe_p->next;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
