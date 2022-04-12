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
#include	<string.h>
#include	<time.h>
#include	<ctype.h>
#include	<unistd.h>


/* IRCSP Core Includes */
#include	"misc.h"
#include	"conf.h"
#include	"log.h"
#include	"socket.h"
#include	"ircsp.h"
#include	"users.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    setflags ()
                |
 DESCRIPTION    |    Used to set flags
                |
                |        SETFL_SET    -    Clears flags and sets the given ones.
                |        SETFL_DELETE -    Deletes given flags, if they are set
                |        SETFL_APPEND -    Adds flags to the flag var if they are not already set.
                |
                |    To use multiple flags be sure to seperate them via |
                |
                |        Example:   setflags (userp->flags, SETFL_DELETE, NICK_IS_OPER|NICK_IS_ADMIN);
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
u_int setflags (u_int *flagvar, u_int mode, u_int flags)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	if (mode == SETFL_SET)
	{
		return (*flagvar = flags);
	}

	if (mode == SETFL_DELETE)
	{
		return (*flagvar &= ~flags);
	}

	if (mode == SETFL_APPEND)
	{
		return (*flagvar |= flags);
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_warning ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_warning (char *format, ...)
{
	nickname_t *myuserp;

	char buf[256];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);

	myuserp = Nickname_Find (config->uworld_nickname);
	if (myuserp)
		ssprintf(s, "%s O %s :\002WARNING\002 - %s",
			myuserp->numeric, config->operchannel_name, buf);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    checkGlines ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
/*
void checkGlines (void)
{
	Gline *glinep;
	time_t curtime;

	curtime = time(NULL);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		if (glinep->expires < curtime)
		{
			ssprintf(s, "%s WA :Removing Expired GLINE for [%s]\n",
				config->server_numeric, glinep->mask);


			ssprintf(s, "%s GL * -%s %lu %lu\n",
				config->server_numeric, glinep->mask, (time(NULL) - 60),
				time(NULL));

			delGline(glinep->mask);
		}
	}

	uw_saveGlineDB ();
}
*/
