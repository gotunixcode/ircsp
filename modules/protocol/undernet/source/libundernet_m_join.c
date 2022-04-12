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
#include	<time.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"ircsp.h"
#include	"log.h"
#include	"users.h"
#include	"channels.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_join.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_join ()
                |
 DESCRIPTION    |    Undernet Protocol (m_join)
                |    This function will handle Server Message: JOIN [J]
                |
                |        [ircsp.c:725:ircsp_mainloop()]: Socket (READ): ADAAY J #fuck 1575597426
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_join (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	char			*argv[4];


	assert (sBuf != NULL);


	argv[0] = strtok(sBuf, " ");		/* Numeric of the user joining the channel */
	strtok (NULL, " ");			/* Server Messages: JOIN [J] - Ignore */
	argv[1] = strtok (NULL, " ");		/* Channel being joined */


	p_nickname = Nickname_Find (argv[0]);
	assert (p_nickname != NULL);
	assert (p_nickname->numeric != NULL);
	assert (p_nickname->nickname != NULL);


	p_channel = Channel_Find (argv[1]);
	if (!p_channel)
	{
		p_channel = Channel_Add (argv[1], 0, time (NULL), NULL, 0);
		assert (p_channel != NULL);
		assert (p_channel->name != NULL);


		p_channel_nickname = ChannelNickname_Add (p_channel, p_nickname);
		p_nickname_channel = NicknameChannel_Add (p_nickname, p_channel);


		assert (p_channel_nickname != NULL);
		assert (p_nickname_channel != NULL);


		p_channel_nickname->flags |= IS_CHANOP;
		p_nickname_channel->flags |= IS_CHANOP;
	}
	else
	{
		p_channel_nickname = ChannelNickname_Add (p_channel, p_nickname);
		p_nickname_channel = NicknameChannel_Add (p_nickname, p_channel);


		assert (p_channel_nickname != NULL);
		assert (p_nickname_channel != NULL);
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}
