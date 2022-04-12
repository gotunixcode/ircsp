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
#include	"channels.h"
#include	"users.h"
#include	"servers.h"
#include	"cmds.h"
#include	"alloc.h"
#include	"ircsp.h"
#include	"log.h"
#include	"conf.h"
#include	"socket.h"
#include	"modules.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    cmd_notice_operators ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void cmd_notice_operators (char *src, char *format, ...)
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	if (src != NULL)
	{
		nickname_t		*p_nickname_src;


		p_nickname_src = Nickname_Find (src);
		if (p_nickname_src != NULL)
		{
			char		buf[256];
			va_list		msg;

			va_start (msg, format);
			vsprintf (buf, format, msg);


			if (config->operator_notice & OPERNOTICE_CHANNEL)
			{
				channel_t		*p_channel;
				nickname_channel_t	*p_nickname_chanlist;


				if (config->operchannel_name != NULL)
				{
					p_channel = Channel_Find (config->operchannel_name);
					if (p_channel != NULL)
					{
						p_nickname_chanlist = NicknameChannel_Find (p_nickname_src, p_channel);
						if (p_nickname_chanlist != NULL)
						{
							cmd_notice (p_nickname_src->numeric, p_channel->name, buf);
						}
					}
					else
					{
						LOG (main_logfile_p, LOG_WARNING,
							"[WARN] - [%s:%d:%s()]:  Failed to locate operchannel [%s] in Channel_p\n",
							__FILE__, __LINE__, __FUNCTION__,
							config->operchannel_name
						);
					}
				}
				else
				{
					LOG (main_logfile_p, LOG_WARNING,
						"[WARN] - [%s:%d:%s()]:  Operator channel appears to be undefined, check conf file [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, CONFFILE
					);
				}
			}

			else if (config->operator_notice & OPERNOTICE_DIRECT)
			{
				operator_t		*p_Operator;


				for (p_Operator = operator_h; p_Operator; p_Operator = p_Operator->next)
				{
					cmd_notice (p_nickname_src->numeric, p_Operator->nickname_p->numeric, buf);
				}
			}

			else if (config->operator_notice & OPERNOTICE_WALLOPS)
			{
				cmd_wallops (p_nickname_src->numeric, buf);
			}
		}
		else
		{
			LOG (main_logfile_p, LOG_WARNING,
				get_log_message (LOG_MESSAGE_NOSUCHNICK),
				__FILE__, __LINE__, __FUNCTION__, src
			);
		}
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);
}
