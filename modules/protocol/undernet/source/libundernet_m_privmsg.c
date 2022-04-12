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
#include	"users.h"
#include	"modules.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_privmsg.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_privmsg ()
                |
 DESCRIPTION    |    Undernet Protocol (m_privmsg)
                |    This function will handle Server Message: PRIVMSG [P]
                |
                |        [ircsp.c:725:ircsp_mainloop()]: Socket (READ): ADAAY P AIAAB :SUP
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_privmsg (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname_src;
	nickname_t		*p_nickname_dst;
	module_t		*p_Module;
	module_callback_t	*p_ModuleCallback;
	ConfigServiceModule	*p_ConfigServiceModule;
	char			*argv[2];
	char			*c_privmsg;


	c_privmsg = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_privmsg != NULL);


	strcpy (c_privmsg, sBuf);

	argv[0] = strtok (c_privmsg, " ");
	strtok (NULL, " ");
	argv[1] = strtok (NULL, " ");
	argv[2] = strtok (NULL, "\n");


	if (argv[1][0] == '#')
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		We don't do anything with channel messges yet.
		-----------------------------------------------------------------------------------------------------------------
		*/


		return 0;
	}


	p_nickname_src = Nickname_Find (argv[0]);
	assert (p_nickname_src != NULL);


	p_nickname_dst = Nickname_Find (argv[1]);
	assert (p_nickname_dst != NULL);


	p_ConfigServiceModule = ConfigServiceModule_find (p_nickname_dst->nickname);
	if (p_ConfigServiceModule)
	{
		if (p_ConfigServiceModule->p_Module)
		{
			p_ModuleCallback = ModuleCallback_Find (p_ConfigServiceModule->p_Module, "m_privmsg");
			if (p_ModuleCallback)
			{
				p_ModuleCallback->func (p_ConfigServiceModule->p_Module, argv[0], argv[1], argv[2]);
			}

		}
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND), __FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
