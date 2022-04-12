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
#include	"rehash.h"
#include	"synch.h"
#include	"misc.h"
#include	"conf.h"
#include	"alloc.h"
#include	"log.h"
#include	"text.h"
#include	"socket.h"
#include	"ircsp.h"
#include	"users.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_rehash
                |
 DESCRIPTION    |    Lets drop all the glines and admins from memory and reload them from our database. And eventually do the same
                |    with our config file.  However some config option will require a restart, such as UPLINKS, SERVER names, etc.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_rehash (void)
{
	nickname_t *myuserp;
	nickname_t *userp;
//	Gline *glinep;

//	LOG (MAINLOG, "[%s:%d:%s()]:  Signal HUP (Rehash) received from SHELL\n",
//		__FILE__, __LINE__, __FUNCTION__);


	/* This shouldn't be an issue but lets make sure we are connected
	   and that we are already done with our initial burst. */

	if (ircsp->connected)
	{
		if (ircsp->initial_burst)
		{
			myuserp = Nickname_Find (config->uworld_nickname);
			if (!myuserp)
			{
//				LOG (MAINLOG, "[%s:%d:%s()]:  Failed to find UWORLD nickname\n",
//					__FILE__, __LINE__, __FUNCTION__);
				return ;
			}

//			ircsp_notice (
//				"@(#) - (%s:%d) %s():  Signal HUP Received.  (Rehashing)\n",
//				__FILE__, __LINE__, __FUNCTION__);

			ircsp->rehashing = 1;


			/* Remove GLINES from memory */

//			for (glinep = glineHEAD; glinep; glinep = glinep->next)
//			{
//				delGline (glinep->mask);
//			}

			/* Reload Administrators from DB */

//			loadAdminDB ();

			/* Reload GLINES from DB */

//			uw_loadGlineDB ();

			/* Sync ! */

			synch ();

			ircsp->rehashing = 0;
		}
	}

//	LOG (MAINLOG, "[%s:%d:%s()]:  End of signal HUP\n",
//		__FILE__, __LINE__, __FUNCTION__);
}
