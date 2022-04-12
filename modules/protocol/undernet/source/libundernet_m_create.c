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
#include	"libundernet_m_create.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_create ()
                |
 DESCRIPTION    |    Undernet Protocol (m_create)
                |    This function will handle Server Message: CREATE [C]
                |
                |        ADAAh C #newchannel 1572924384
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_create (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}

	nickname_t		*nickname_p;
	nickname_t		*temp_nickname_p;
	nickname_channel_t	*nickname_channel_p;
	channel_t		*channel_p;
	channel_nickname_t	*channel_nickname_p;
	char			*nickname;
	char			*chanlist	= NULL;
	char			*channel[16];
	time_t			timestamp;
	int			count;
	int			argc		= 0;

	assert (sBuf != NULL);

	nickname = strtok (sBuf, " ");			/* Nickname or Numeric of the User joining the Channel(s) */
	strtok (NULL, " ");				/* Server Message: CREATE [C] - Ignore */
	chanlist = strtok (NULL, " ");			/* Channel(s) being Created */
	timestamp = atol(strtok (NULL, " "));		/* Time Stamp */

	nickname_p = Nickname_Find (nickname);
	assert (nickname_p != NULL);

	count = tokenize (chanlist, ',');
	if (count == 1)
	{
		channel_p = Channel_Find (chop(chanlist));
		if (channel_p)
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    We should never really hit this unless there is a problem... m_create should only get issued
			    when a new channel is being created.  We will try to correct this by deleting the channel
			    and any users on said channel before we create a new channel.
			---------------------------------------------------------------------------------------------------------
			*/
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Duplicate Channel [%s]! Attempting to correct this\n",
				__FILE__, __LINE__, __FUNCTION__,
				channel_p->name
			);

			channel_nickname_p = channel_p->channel_nickname_h;
			while (channel_nickname_p)
			{
				temp_nickname_p = channel_nickname_p->nickname_p;

				NicknameChannel_Del (temp_nickname_p, channel_p);
				ChannelNickname_Del (channel_p, temp_nickname_p);

				channel_nickname_p = channel_nickname_p->next;
			}
		}

		channel_p = Channel_Add (chop(chanlist), 0, timestamp, NULL, 0);
		assert (channel_p != NULL);

		nickname_channel_p = NicknameChannel_Add (nickname_p, channel_p);
		channel_nickname_p = ChannelNickname_Add (channel_p, nickname_p);

		assert (nickname_channel_p != NULL);
		assert (channel_nickname_p != NULL);

		channel_nickname_p->flags |= IS_CHANOP;
		nickname_channel_p->flags |= IS_CHANOP;
	}
	else
	{
		channel[argc] = strtok (chanlist, " ");
		channel_p = Channel_Find (channel[argc]);
		if (channel_p)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Duplicate Channel [%s]! Attempting to correct this\n",
				__FILE__, __LINE__, __FUNCTION__,
				channel_p->name
			);

			channel_nickname_p = channel_p->channel_nickname_h;
			while (channel_nickname_p)
			{
				temp_nickname_p = channel_nickname_p->nickname_p;

				NicknameChannel_Del (temp_nickname_p, channel_p);
				ChannelNickname_Del (channel_p, temp_nickname_p);

				channel_nickname_p = channel_nickname_p->next;
			}
		}

		channel_p = Channel_Add (channel[argc], 0, timestamp, NULL, 0);
		assert (channel_p != NULL);

		nickname_channel_p = NicknameChannel_Add (nickname_p, channel_p);
		channel_nickname_p = ChannelNickname_Add (channel_p, nickname_p);

		assert (nickname_channel_p != NULL);
		assert (channel_nickname_p != NULL);

		nickname_channel_p->flags |= IS_CHANOP;
		channel_nickname_p->flags |= IS_CHANOP;

		while (channel[argc])
		{
			channel[++argc] = strtok (NULL, " ");
			if (channel[argc] != NULL)
			{
				channel_p = Channel_Find (channel[argc]);
				if (channel_p)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Duplicate Channel [%s]! Attempting to correct this\n",
						__FILE__, __LINE__, __FUNCTION__,
						channel_p->name
					);

					channel_nickname_p = channel_p->channel_nickname_h;
					while (channel_nickname_p)
					{
						temp_nickname_p = channel_nickname_p->nickname_p;

						NicknameChannel_Del (temp_nickname_p, channel_p);
						ChannelNickname_Del (channel_p, temp_nickname_p);

						channel_nickname_p = channel_nickname_p->next;
					}
				}


				channel_p = Channel_Add (channel[argc], 0, timestamp, NULL, 0);
				assert (channel_p != NULL);


				nickname_channel_p = NicknameChannel_Add (nickname_p, channel_p);
				channel_nickname_p = ChannelNickname_Add (channel_p, nickname_p);

				assert (nickname_channel_p != NULL);
				assert (channel_nickname_p != NULL);

				nickname_channel_p->flags |= IS_CHANOP;
				channel_nickname_p->flags |= IS_CHANOP;
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
