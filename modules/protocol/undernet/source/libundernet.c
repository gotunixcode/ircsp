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
#include	"ircsp.h"
#include	"log.h"
#include	"alloc.h"
#include	"conf.h"
#include	"servers.h"
#include	"users.h"
#include	"channels.h"
#include	"socket.h"
#include	"modules.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/* Undernet Module Includes */
#include	"libundernet.h"
#include	"libundernet_cmds.h"
#include	"libundernet_introdouce.h"
#include	"libundernet_parser.h"


char ModuleVersion[] = "1.0.0";
char ModuleName[] = "Protocol_IRCUP10";
char ModuleType[] = "protocol";


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    init_module ()
                |
 DESCRIPTION    |    This function will initialize our module
                |
 RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void init_module (p_Module)
	struct _module		*p_Module;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	p_Module->ModuleType |= MODULETYPE_PROTOCOL;

//	ircsp_add_callback (p_Module, "do_parse");

	cmd_notice		=	libundernet_cmd_notice;
	cmd_wallops		=	libundernet_cmd_wallops;
	cmd_join		=	libundernet_cmd_join;
	cmd_part		=	libundernet_cmd_part;
	cmd_chanop		=	libundernet_cmd_chanop;
	cmd_chandeop		=	libundernet_cmd_chandeop;
	cmd_chanvoice		=	libundernet_cmd_chanvoice;
	cmd_chandevoice		=	libundernet_cmd_chandevoice;
	cmd_kill		=	libundernet_cmd_kill;
	cmd_nickjupe		=	libundernet_cmd_nickjupe;

	introdouce_service	=	libundernet_introdouce_service;
	introdouce_server	=	libundernet_introdouce_server;

	m_parse			=	libundernet_parser;

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
