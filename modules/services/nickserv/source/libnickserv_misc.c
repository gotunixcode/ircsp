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


/* System Includes */
#include	<dlfcn.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"modules.h"
#include	"ircsp.h"
#include	"log.h"
#include	"text.h"
#include	"alloc.h"
#include	"conf.h"
#include	"servers.h"
#include	"users.h"
#include	"channels.h"
#include	"socket.h"
#include	"scheduler.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/* NickServ Module Includes */
#include	"libnickserv.h"
#include	"libnickserv_admin.h"
#include	"libnickserv_database.h"
#include	"libnickserv_nickinfo.h"
#include	"libnickserv_misc.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    m_nick_new ()
                   |
    DESCRIPTION    |    This function will handle NickServ routines around m_nick ()
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
m_nick_new (module_p, nickname_p)
	struct _module		*module_p;
	struct _nickname	*nickname_p;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*my_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;
	char			*hostmask;

	assert (nickname_p != NULL);
	assert (module_p != NULL);
	assert (module_p->ServiceNickname != NULL);

	my_nickname_p = Nickname_Find (module_p->ServiceNickname);

	nickinfo_p = Nickinfo_Search (nickname_p->nickname);
	if (nickinfo_p)
	{
		if (nickinfo_p->flags & NICKSERV_AUTOIDENTIFY)
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    The user has the AUTOIDENTIFY flag enabled, lets first generate a hostmask using info in the
			    nickname list.  We will then compare that list against the hostmasks the user has set in their
			    access list.  If we get a match we will automatically mark the user as identified, otherwise
			    we will continue to haras the user to IDENTIFY or KILL the user if the KILL PROTECTION flag is
			    enabled.

			    One thing to note, we do not allow services root/services admin to have the AUTOIDENTIFY flag set
			    to be safe we will do a check if its on and if it is we will turn it off here.

			    At some point we may want to make this configuratble in the config file.
			---------------------------------------------------------------------------------------------------------
			*/
			services_root_p = ServicesRoot_Search (nickname_p->nickname);
			services_admin_p = ServicesAdmin_Search (nickname_p->nickname);

			if (services_root_p)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Root Users"
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Root Users"
					);
				}

				nickinfo_p->flags &= ~NICKSERV_AUTOIDENTIFY;
			}

			else if (services_admin_p)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Admin Users"
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Admin Users"
					);
				}

				nickinfo_p->flags &= ~NICKSERV_AUTOIDENTIFY;
			}

			else
			{
				hostmask = (char *)ircsp_calloc (1, strlen (nickname_p->username) + strlen (nickname_p->hostname_p->hostname) + 10);
				assert (hostmask != NULL);
				sprintf (hostmask, "%s@%s", nickname_p->username, nickname_p->hostname_p->hostname);

				nickinfo_access_p = NickinfoAccess_Wildcard (nickinfo_p, hostmask);
				if (nickinfo_access_p)
				{
					if (!my_nickname_p)
					{
						cmd_notice (NULL, nickname_p->nickname,
							"\002AUTO IDENTIFY\002 - ENABLED"
						);

						cmd_notice (NULL, nickname_p->nickname,
							"You have been auto identified based on your hostmask access list"
						);
					}
					else
					{
						cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
							"\002AUTO IDENTIFY\002 - ENABLED"
						);

						cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
							"You have been auto identified based on your hostmask access list"
						);
					}

					nickname_p->flags |= NICK_IS_IDENTIFIED;
				}
			}
		}

		if (!(nickname_p->flags & NICK_IS_IDENTIFIED))
		{
			if (!my_nickname_p)
			{
				cmd_notice (NULL, nickname_p->nickname,
					"This nickname is owned by someone else.  Please choose another."
				);

				cmd_notice (NULL, nickname_p->nickname,
					"If this is your nickname, type /msg %s IDENTIFY \002password\002",
					module_p->ServiceNickname
				);

			}
			else
			{
				cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
					"This nickname is owned by someone else.  Please choose another."
				);

				cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
					"If this is your nickname, type /msg %s IDENTIFY \002password\002",
					my_nickname_p->nickname
				);
			}


			if (nickinfo_p->flags & NICKSERV_KILLPROTECT)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, nickname_p->nickname,
						"If you do not change nicknames or \002IDENTIFY\002, you will be "\
						"disconnected in 5 minutes."
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
						"If you do not change nicknames or \002IDENTIFY\002, you will be "\
						"disconnected in 5 minutes."
					);
				}
			}
		}
	}
	else
	{
		if (nickname_p->flags & NICK_IS_OPER)
		{
			if (!my_nickname_p)
			{
				cmd_notice (NULL, nickname_p->nickname,
					"It is \002recommended\002 that IRC Operators register their nicknames"
				);
			}
			else
			{
				cmd_notice (my_nickname_p->nickname, nickname_p->nickname,
					"It is \002recommended\002 that IRC Operators register their nicknames"
				);
			}
		}
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
    FUNCTION       |    m_nick_change ()
                   |
    DESCRIPTION    |    This function will handle NickServ routines around m_nick ()
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
m_nick_change (module_p, new_nickname_p, old_nickname_p)
	struct _module		*module_p;
	struct _nickname	*new_nickname_p;
	struct _nickname	*old_nickname_p;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*my_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;
	char			*hostmask;

	assert (module_p != NULL);
	assert (new_nickname_p != NULL);
	assert (new_nickname_p->nickname != NULL);
	assert (old_nickname_p != NULL);
	assert (old_nickname_p->nickname != NULL);

	my_nickname_p = Nickname_Find (module_p->ServiceNickname);

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    First lets check the old nickname for any flags we need to remove, and pointers we have to update
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (old_nickname_p->flags & NICK_IS_ROOT)
	{
		services_root_p = ServicesRoot_Search (old_nickname_p->nickname);
		assert (services_root_p != NULL);

		services_root_p->nickinfo_p = NULL;
		services_root_p->nickname_p = NULL;

		old_nickname_p->flags &= ~NICK_IS_ROOT;
	}

	if (old_nickname_p->flags & NICK_IS_ADMIN)
	{
		services_admin_p = ServicesAdmin_Search (old_nickname_p->nickname);
		assert (services_admin_p != NULL);

		services_admin_p->nickinfo_p = NULL;
		services_admin_p->nickname_p = NULL;

		old_nickname_p->flags &= ~NICK_IS_ADMIN;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Lets move on to the new nickname
	-------------------------------------------------------------------------------------------------------------------------
	*/
	nickinfo_p = Nickinfo_Search (new_nickname_p->nickname);
	if (nickinfo_p)
	{
		if (nickinfo_p->flags & NICKSERV_AUTOIDENTIFY)
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    The user has the AUTOIDENTIFY flag enabled, lets first generate a hostmask using info in the
			    nickname list.  We will then compare that list against the hostmasks the user has set in their
			    access list.  If we get a match we will automatically mark the user as identified, otherwise
			    we will continue to haras the user to IDENTIFY or KILL the user if the KILL PROTECTION flag is
			    enabled.

			    One thing to note, we do not allow services root/services admin to have the AUTOIDENTIFY flag set
			    to be safe we will do a check if its on and if it is we will turn it off here.

			    At some point we may want to make this configuratble in the config file.
			---------------------------------------------------------------------------------------------------------
			*/
			services_root_p = ServicesRoot_Search (new_nickname_p->nickname);
			services_admin_p = ServicesAdmin_Search (new_nickname_p->nickname);

			if (services_root_p)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, new_nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Root Users"
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
						"\002AUTO IDENTIFY - DISALLOWED for Services Root Users"
					);
				}

				nickinfo_p->flags &= ~NICKSERV_AUTOIDENTIFY;
			}

			else if (services_admin_p)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, new_nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Admin Users"
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
						"\002AUTO IDENTIFY\002 - DISALLOWED for Services Admin Users"
					);
				}

				nickinfo_p->flags &= ~NICKSERV_AUTOIDENTIFY;
			}

			else
			{
				hostmask = (char *)ircsp_calloc (1, strlen (new_nickname_p->username) + strlen (new_nickname_p->hostname_p->hostname) + 10);
				assert (hostmask != NULL);
				sprintf (hostmask, "%s@%s", new_nickname_p->username, new_nickname_p->hostname_p->hostname);

				nickinfo_access_p = NickinfoAccess_Wildcard (nickinfo_p, hostmask);
				if (nickinfo_access_p)
				{
					if (!my_nickname_p)
					{
						cmd_notice (NULL, new_nickname_p->nickname,
							"\002AUTO IDENTIFY\002 - ENABLED"
						);

						cmd_notice (NULL, new_nickname_p->nickname,
							"You have been auto identified based on your hostmask access list"
						);
					}
					else
					{
						cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
							"\002AUTO IDENTIFY\002 - ENABLED"
						);

						cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
							"You have been auto identified based on your hostmask access list"
						);
					}

					new_nickname_p->flags |= NICK_IS_IDENTIFIED;
				}
			}
		}

		if (!(new_nickname_p->flags & NICK_IS_IDENTIFIED))
		{
			if (!my_nickname_p)
			{
				cmd_notice (NULL, new_nickname_p->nickname,
					"This nickname is owned by someone else.  Please choose another."
				);

				cmd_notice (NULL, new_nickname_p->nickname,
					"If this is your nickname, type /msg %s IDENTIFY \002password\002",
					module_p->ServiceNickname
				);
			}
			else
			{
				cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
					"This nickname is owned by someone else.  Please choose another."
				);

				cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
					"If this is your nickname, type /msg %s IDENTIFY \002password\002",
					my_nickname_p->nickname
				);
			}

			if (nickinfo_p->flags & NICKSERV_KILLPROTECT)
			{
				if (!my_nickname_p)
				{
					cmd_notice (NULL, new_nickname_p->nickname,
						"If you do not change nicknames or \002IDENTIFY\002, you will be "\
						"disconnected in 5 minutes."
					);
				}
				else
				{
					cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
						"If you do not change nicknames or \002IDENTIFY\002, you will be "\
						"disconnected in 5 minutes."
					);
				}
			}
		}
	}
	else
	{
		if (new_nickname_p->flags & NICK_IS_OPER)
		{
			if (!my_nickname_p)
			{
				cmd_notice (NULL, new_nickname_p->nickname,
					"It is \002recommended\002 that IRC Operators register their nicknames"
				);
			}
			else
			{
				cmd_notice (my_nickname_p->nickname, new_nickname_p->nickname,
					"It is \002recommended\002 that IRC Operators register their nicknames"
				);
			}
		}
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
    FUNCTION       |    m_quit ()
                   |
    DESCRIPTION    |    This function will handle NickServ routines around m_nick ()
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
m_quit (module_p, nickname_p)
	struct _module		*module_p;
	struct _nickname	*nickname_p;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickinfo_t		*nickinfo_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;

	assert (module_p != NULL);
	assert (nickname_p != NULL);
	assert (nickname_p->nickname != NULL);

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Lets first check if the nickname is registered, if its not we don't need to do anything.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	nickinfo_p = Nickinfo_Search (nickname_p->nickname);
	if (nickinfo_p)
	{
		if (nickname_p->flags & NICK_IS_IDENTIFIED)
		{
			nickname_p->flags &= ~NICK_IS_IDENTIFIED;
			nickinfo_p->nickname_p = NULL;
		}

		if (nickname_p->flags & NICK_IS_ROOT)
		{
			services_root_p = ServicesRoot_Search (nickname_p->nickname);
			assert (services_root_p != NULL);

			services_root_p->nickinfo_p = NULL;
			services_root_p->nickname_p = NULL;

			nickname_p->flags &= ~NICK_IS_ROOT;
		}

		if (nickname_p->flags & NICK_IS_ADMIN)
		{
			services_admin_p = ServicesAdmin_Search (nickname_p->nickname);
			assert (services_admin_p != NULL);

			services_admin_p->nickinfo_p = NULL;
			services_admin_p->nickname_p = NULL;

			nickname_p->flags &= ~NICK_IS_ADMIN;
		}
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
    FUNCTION       |    m_kill ()
                   |
    DESCRIPTION    |    This function will handle NickServ routines around m_kill ()
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void
m_kill (module_p, nickname_p)
	struct _module		*module_p;
	struct _nickname	*nickname_p;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickinfo_t		*nickinfo_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;

	assert (module_p != NULL);
	assert (nickname_p != NULL);
	assert (nickname_p->nickname != NULL);

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Lets first check if the nickname is registered, if its not we don't need to do anything.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	nickinfo_p = Nickinfo_Search (nickname_p->nickname);
	if (nickinfo_p)
	{
		if (nickname_p->flags & NICK_IS_IDENTIFIED)
		{
			nickname_p->flags &= ~NICK_IS_IDENTIFIED;
			nickinfo_p->nickname_p = NULL;
		}

		if (nickname_p->flags & NICK_IS_ROOT)
		{
			services_root_p = ServicesRoot_Search (nickname_p->nickname);
			assert (services_root_p != NULL);

			services_root_p->nickinfo_p = NULL;
			services_root_p->nickname_p = NULL;

			nickname_p->flags &= ~NICK_IS_ROOT;
		}

		if (nickname_p->flags & NICK_IS_ADMIN)
		{
			services_admin_p = ServicesAdmin_Search (nickname_p->nickname);
			assert (services_admin_p != NULL);

			services_admin_p->nickinfo_p = NULL;
			services_admin_p->nickname_p = NULL;

			nickname_p->flags &= ~NICK_IS_ADMIN;
		}
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
