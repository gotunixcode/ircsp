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
#include	"servers.h"
#include	"users.h"


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
 FUNCTION       |    libundernet_m_squit ()
                |
 DESCRIPTION    |    Undernet Protocol (m_squit)
                |    This function will handle Server Message: SQUIT [SQ]
                |
                |        AB SQ UWorld.GOTUNIX.NET 1572713485 :EOF from client
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_squit (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	server_t	*p_server;
	server_t	*p_server_leaf;
	char		*argv[2];


	assert (sBuf != NULL);


	strtok (sBuf, " ");			/* Numeric of the User/Server issuing the SQUIT */
	strtok (NULL, " ");			/* Server Message: SQUIT [SQ] - Ignore */
	argv[0] = strtok (NULL, " ");		/* Server being SQUITed */


	assert (config != NULL);
	assert (config->server_name != NULL);


	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->my_uplink_p != NULL);


	if (!strcasecmp (p_server->my_uplink_p->name, argv[0]))
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		The server thats being SQUIT is me?  I will add some logic to this later on to either force us to
		save and restart, or just purge all Nickname/Channel/Server lists and flags, and reconnect.


		For now i'm just going to exit
		-----------------------------------------------------------------------------------------------------------------
		*/
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: SQUIT Recieved for [%s], exiting\n",
			__FILE__, __LINE__, __FUNCTION__, config->server_name);

		ircsp_exit (0, "I've been SQUITed");
	}


	p_server = Server_Find (argv[0]);
	assert (p_server != NULL);


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First we are going to check and see if the server being SQUITed had any leafs attached to it, if it did we need
	to remove all the users assoiacted with them and then delete the server.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	for (p_server_leaf = server_h; p_server_leaf; p_server_leaf = p_server_leaf->next)
	{
		if (p_server_leaf->my_uplink_p)
		{
			if (!strcasecmp (p_server_leaf->my_uplink_p->name, p_server->name))
			{
				Nickname_DelServer (p_server_leaf);
				Server_Del (p_server_leaf->name);
			}
		}
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	And now finally we can delete any users on the server itself and its server record.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	Nickname_DelServer (p_server);
	Server_Del (p_server->name);


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}
}
