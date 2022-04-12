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
#include	<string.h>
#include	<unistd.h>
#include	<stdarg.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/wait.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<ctype.h>
#include	<time.h>
#include	<errno.h>
#include	<netinet/in.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"text.h"
#include	"alloc.h"
#include	"log.h"
#include	"ircsp.h"
#include	"servers.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Server_Add ()
                |
 DESCRIPTION    |    This function will add a new server to our Server_List
                |
                |    @TODO:
                |        We need to change the uplink to use a _server_list pointer to support other protocols, right now
                |        we are adding the numeric of the uplink that a server is connected to, this will only work
                |        on undernet.  Either that or we allocate memory and set the uplink later on?
                |
                |    @CHANGELOG:
                |        [2019/10/25] Updated to allow the numeric variable to be NULL to support other protocols.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
server_t *Server_Add (name, created)
	char		*name;
	time_t		created;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	server_t		*p_Server;

	p_Server = (server_t *)ircsp_calloc(1, sizeof(server_t));
	p_Server->name = (char *)ircsp_calloc(1, strlen(name) +5);
	if ((!p_Server) || (!p_Server->name))
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for Server_p\n",
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

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Server [%s] added at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, p_Server->name, p_Server->name
		);
	}

	p_Server->numeric = NULL;
	strcpy(p_Server->name, name);
	p_Server->created = created;
	p_Server->numUsers = 0;
	p_Server->numOpers = 0;
	p_Server->flags |= SERVER_IN_BURST;

	p_Server->my_uplink_p = NULL;

	if (!server_h)
	{
		server_h = p_Server;
		p_Server->next = NULL;
	}
	else
	{
		p_Server->next = server_h;
		server_h = p_Server;
	}
	status->numServers++;

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_Server;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Server_Find ()
                |
 DESCRIPTION    |    This function searches our server list for a specific server
                |
                |        [2019/10/25] Adding support for server numerics so we can DEPRECIATE ircsp_server_find_by_numeric ()
                |
 RETURNS        |    Returns NULL if we do not find a match, otherwise we return a pointer to the server.
---------------------------------------------------------------------------------------------------------------------------------
*/
server_t *Server_Find (search)
	char		*search;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	server_t *p_Server;

	for (p_Server = server_h; p_Server; p_Server = p_Server->next)
	{
		if (p_Server->numeric != NULL)
		{
			if ((!strcasecmp (p_Server->name, search)) || (!strcasecmp (p_Server->numeric, search)))
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Server [%s] found at [%p] using [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Server->name, p_Server->name,
							search
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

				return p_Server;
			}
		}
		else
		{
			if (!strcasecmp(search, p_Server->name))
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Server [%s] found at [%p] using [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Server->name,
							p_Server->name, search
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

				return p_Server;
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
 FUNCTION       |    Server_Del ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void Server_Del (name)
	char		*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	server_t	*p_Server;
	server_t	*p_Next;
	server_t	*p_Delete;

	for (p_Server = server_h; p_Server; p_Server = p_Next)
	{
		p_Next = p_Server->next;

		if (p_Server->numeric != NULL)
		{
			if ( (!strcasecmp (p_Server->name, name)) || (!strcasecmp (p_Server->numeric, name)) )
			{
				if ((DEBUG) && (DEBUG_LIST))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Deleting server [%s] found at [%p] using [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Server->name,
						p_Server->name, name
					);
				}

				if (server_h == p_Server)
				{
					server_h = p_Server->next;
				}
				else
				{
					for (p_Delete = server_h; (p_Delete->next != p_Server) && p_Delete; p_Delete = p_Delete->next);
					p_Delete->next = p_Server->next;
				}

				p_Server->flags = 0;
				p_Server->numUsers = 0;
				p_Server->numOpers = 0;

				ircsp_free (p_Server->name);
				ircsp_free (p_Server->numeric);
				ircsp_free (p_Server);

				status->numServers--;

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
		else
		{
			if (!strcasecmp (p_Server->name, name))
			{
				if ((DEBUG) && (DEBUG_LIST))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Deleting server [%s] found at [%p] using [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Server->name,
						p_Server->name, name
					);
				}

				if (server_h == p_Server)
				{
					server_h = p_Server->next;
				}
				else
				{
					for (p_Delete = server_h; (p_Delete->next != p_Server) && p_Delete; p_Delete = p_Delete->next);
					p_Delete->next = p_Server->next;
				}

				p_Server->flags = 0;
				p_Server->numUsers = 0;
				p_Server->numOpers = 0;

				ircsp_free (p_Server->name);
				ircsp_free (p_Server->numeric);
				ircsp_free (p_Server);

				status->numServers--;


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
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
