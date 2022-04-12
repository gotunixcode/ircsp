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
#include	"text.h"
#include	"users.h"
#include	"channels.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_part.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_part ()
                |
 DESCRIPTION    |    Undernet Protocol (m_part)
                |    This function will handle Server Message: PART [P]
                |
                |        [ircsp.c:725:ircsp_mainloop()]: Socket (READ): ADAAY L #fuck
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_part (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*nickname_p;
	channel_t		*channel_p;
	char			*argv[16];
	char			*chanlist = NULL;
	int			count = 0;
	int			loop = 0;
	int			argc = 1;

	assert (sBuf != NULL);


	argv[0] = strtok (sBuf, " ");			/* Numeric of the user leaving the channel */
	strtok (NULL, " ");				/* Server Message: PART [L] - Ignore */
	chanlist = strtok (NULL, " ");			/* Channel the user left */


	nickname_p = Nickname_Find (argv[0]);
	assert (nickname_p != NULL);


	count = tokenize (chanlist, ',');

	if (count == 1)
	{
		channel_p = Channel_Find (chop(chanlist));
		assert (channel_p != NULL);


		NicknameChannel_Del (nickname_p, channel_p);
		ChannelNickname_Del (channel_p, nickname_p);
	}
	else
	{
		argv[argc] = strtok(chanlist, " ");
		channel_p = Channel_Find (argv[argc]);
		assert (channel_p != NULL);

		NicknameChannel_Del (nickname_p, channel_p);
		ChannelNickname_Del (channel_p, nickname_p);


		while (argv[argc])
		{
			argv[++argc] = strtok (NULL, " ");
			if (argv[argc] != NULL)
			{
				channel_p = Channel_Find (argv[argc]);
				assert (channel_p != NULL);

				NicknameChannel_Del (nickname_p, channel_p);
				ChannelNickname_Del (channel_p, nickname_p);
			}
		}
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
