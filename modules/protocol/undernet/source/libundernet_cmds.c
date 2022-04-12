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


/* Module Includes */
#include	"libundernet.h"
#include	"cmds.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_notice ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_notice)
                |    This function will handle sending NOTICE commands on using the Undernet P10 Protocol
                |
 INPUTS         |    src    = Source the NOTICE is coming from (Could be service nickname, or our server)
                |    dst    = Destination of the NOTICE (Could be nickname or channel)
                |    format = Message and any variables that make up the message
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_notice (char *src, char *dst, char *format, ...)
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	server_t		*p_server;
	nickname_t		*p_nickname_src;
	nickname_t		*p_nickname_dst;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	int			i_server_notice = 0;
	char			buf [256];
	va_list			msg;

	assert (dst != NULL);
	assert (format != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	va_start (msg, format);
	vsprintf (buf, format, msg);


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First we need to figure out if we are sending a notice out as a server, or using a service nickname.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (src != NULL)
	{
		p_server = Server_Find (src);
		p_nickname_src = Nickname_Find (src);

		if ( (p_server) && (!p_nickname_src) )
		{
			i_server_notice = 1;
		}
		else if ( (!p_server) && (p_nickname_src) )
		{
			i_server_notice = 0;
		}
	}
	else
	{
		p_server = Server_Find (config->server_name);
		assert (p_server != NULL);

		i_server_notice = 1;
	}


	if (dst[0] == '#')
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		We are sending a notice to a channel, we need to make sure that the channel exists.
		If we are sending the notice from a service nickname we also need to make sure that the service sending the
		notice is in the channel, if not we will revert back to a server notice.
		-----------------------------------------------------------------------------------------------------------------
		*/
		p_channel = Channel_Find (dst);
		if (!p_channel)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Channel [%s] not found\n",
				__FILE__, __LINE__, __FUNCTION__
			);
		}
		else
		{
			if (i_server_notice)
			{
				ssprintf (s,
					"%s O %s :%s\n",
					p_server->numeric, p_channel->name, buf);
			}
			else
			{
				p_nickname_channel = NicknameChannel_Find (p_nickname_src, p_channel);
				p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname_src);


				if ( (!p_nickname_channel) || (!p_channel_nickname) )
				{
					/*
					-----------------------------------------------------------------------------------------
					Service is not in the channel, we are going to revert to sending the notice from our
					server.
					-----------------------------------------------------------------------------------------
					*/
					p_server = Server_Find (config->server_name);
					if (!p_server)
					{
						/*
						---------------------------------------------------------------------------------
						Failed to look up our server... we should log this and probably crash or restart
						---------------------------------------------------------------------------------
						*/
					}
					else
					{
						ssprintf (s,
							"%s O %s :%s\n",
							p_server->numeric, p_channel->name, buf);
					}
				}
				else
				{
					ssprintf (s,
						"%s O %s :%s\n",
						p_nickname_src->numeric, p_channel->name, buf);
				}
			}
		}
	}
	else
	{
		p_nickname_dst = Nickname_Find (dst);
		if (!p_nickname_dst)
		{
			/*
			---------------------------------------------------------------------------------------------------------
			Destination nickname was not found. We should probably log this
			---------------------------------------------------------------------------------------------------------
			*/
		}
		else
		{
			if (i_server_notice)
			{
				ssprintf (s,
					"%s O %s :%s\n",
					p_server->numeric, p_nickname_dst->numeric, buf);
			}
			else
			{
				ssprintf (s,
					"%s O %s :%s\n",
					p_nickname_src->numeric, p_nickname_dst->numeric, buf);
			}
		}
	}


	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_wallops ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_wallops)
                |    This function will handle sending WALLOPS commands on using the Undernet P10 Protocol
                |
 INPUTS         |    src    = Source the NOTICE is coming from (Could be service nickname, or our server)
                |    format = Message and any variables that make up the message
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_wallops (char *src, char *format, ...)
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	server_t		*p_server;
	char			buf [256];
	va_list			msg;

	assert (src != NULL);
	assert (format != NULL);

	va_start (msg, format);
	vsprintf (buf, format, msg);


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First we need to figure out if we are sending the WALLOPS using our server or a service nickname.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	p_server = Server_Find (src);
	p_nickname = Nickname_Find (src);

	if ( (p_server) && (!p_nickname) )
	{
		ssprintf (s,
			"%s WA :%s\n",
			p_server->numeric, buf);
	}
	else
	{
		ssprintf (s,
			"%s WA :%s\n",
			p_nickname->numeric, buf);
	}

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_join ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_join)
                |    This function will handle sending JOIN commands on using the Undernet P10 Protocol
                |
 INPUTS         |    src     = Service nickname that is going to join a channel
                |    channel = The channel the service is going to be joining
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_join (src, channel)
	char		*src;
	char		*channel;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;

	assert (src != NULL);
	assert (channel != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->numeric != NULL);

	p_nickname = Nickname_Find (src);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		return ;
	}

	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		Channel doesn't exist so we are going to create it
		-----------------------------------------------------------------------------------------------------------------
		*/
		if (LOG_INFO)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Channel [%s] not found, creating\n",
				__FILE__, __LINE__, __FUNCTION__, channel
			);
		}


		p_channel = Channel_Add (channel, 0, time (NULL), NULL, 0);
		assert (p_channel != NULL);
	}

	p_channel->numUsers++;

	p_nickname_channel = NicknameChannel_Add (p_nickname, p_channel);
	p_channel_nickname = ChannelNickname_Add (p_channel, p_nickname);
	assert (p_nickname_channel != NULL);
	assert (p_channel_nickname != NULL);
	p_nickname_channel->flags |= IS_CHANOP;
	p_channel_nickname->flags |= IS_CHANOP;


	ssprintf (s,
		"%s J %s\n",
		p_nickname->numeric, p_channel->name);

	ssprintf (s,
		"%s M %s +o %s\n",
		p_server->numeric, p_channel->name, p_nickname->numeric);

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_part ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_part)
                |    This function will handle sending PART commands on using the Undernet P10 Protocol
                |
 INPUTS         |    src     = Service nickname that is going to leaving a channel
                |    channel = The channel the service is going to be leaving
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_part (src, channel)
	char		*src;
	char		*channel;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	channel_t		*p_channel;

	assert (src != NULL);
	assert (channel != NULL);

	p_nickname = Nickname_Find (src);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, src
		);

		return ;
	}


	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Channel [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, channel
		);

		return ;
	}


	NicknameChannel_Del (p_nickname, p_channel);
	ChannelNickname_Del (p_channel, p_nickname);


	p_channel->numUsers--;


	ssprintf (s,
		"%s L %s\n",
		p_nickname->numeric, p_channel->name);

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_chanop ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_chanop)
                |    This function will handle sending Channel Op (+o)  commands on using the Undernet P10 Protocol
                |
 INPUTS         |    channel  = The Channel we are changing modes on
                |    nickname = The nickname we are changing modes on
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_chanop (channel, nickname)
	char		*channel;
	char		*nickname;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;

	assert (channel != NULL);
	assert (nickname != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->numeric != NULL);

	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Channel [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, channel
		);

		return ;
	}


	p_nickname = Nickname_Find (nickname);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, nickname
		);

		return ;
	}


	p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);
	p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);


	if ( (!p_nickname_channel) || (!p_channel_nickname) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] was not found in Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}


	if ( (p_nickname_channel->flags & IS_CHANOP) && (p_channel_nickname->flags & IS_CHANOP) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] is already a CHANOP on Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}


	p_nickname_channel->flags |= IS_CHANOP;
	p_channel_nickname->flags |= IS_CHANOP;


	ssprintf (s,
		"%s M %s +o %s\n",
		p_server->numeric, p_channel->name, p_nickname->numeric);


	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_chandeop ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_chandeop)
                |    This function will handle sending Channel DeOp (-o)  commands on using the Undernet P10 Protocol
                |
 INPUTS         |    channel  = The channel we are changing modes on
                |    nickname = The nickname we are changing modes on
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_chandeop (channel, nickname)
	char		*channel;
	char		*nickname;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;

	assert (channel != NULL);
	assert (nickname != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->numeric != NULL);

	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Channel [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, channel
		);

		return ;
	}

	p_nickname = Nickname_Find (nickname);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, nickname
		);

		return ;
	}

	p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);
	p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);

	if ( (!p_nickname_channel) || (!p_channel_nickname) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] was not found in Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}


	if ( (!(p_nickname_channel->flags & IS_CHANOP)) && (!(p_channel_nickname->flags & IS_CHANOP)) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] is already a CHANDEOP on Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}


	p_nickname_channel->flags &= ~IS_CHANOP;
	p_channel_nickname->flags &= ~IS_CHANOP;


	ssprintf (s,
		"%s M %s -o %s\n",
		p_server->numeric, p_channel->name, p_nickname->numeric);


	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_chanvoice ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_chanvoice)
                |    This function will handle sending Channel Voice (+v)  commands on using the Undernet P10 Protocol
                |
 INPUTS         |    channel  = The Channel we are changing modes on
                |    nickname = The nickname we are changing modes on
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_chanvoice (channel, nickname)
	char		*channel;
	char		*nickname;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;

	assert (channel != NULL);
	assert (nickname != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->numeric != NULL);

	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Channel [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, channel
		);

		return ;
	}

	p_nickname = Nickname_Find (nickname);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, nickname
		);

		return ;
	}

	p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);
	p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);

	if ( (!p_nickname_channel) || (!p_channel_nickname) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] was not found in Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}

	if ( (p_nickname_channel->flags & IS_CHANVOICE) && (p_channel_nickname->flags & IS_CHANVOICE) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] is already a CHANVOICE on Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}

	p_nickname_channel->flags |= IS_CHANVOICE;
	p_channel_nickname->flags |= IS_CHANVOICE;

	ssprintf (s,
		"%s M %s +v %s\n",
		p_server->numeric, p_channel->name, p_nickname->numeric);

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_chandevoice ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_chandevoice)
                |    This function will handle sending Channel DeVoice (-v)  commands on using the Undernet P10 Protocol
                |
 INPUTS         |    channel  = The channel we are changing modes on
                |    nickname = The nickname we are changing modes on
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_chandevoice (channel, nickname)
	char		*channel;
	char		*nickname;
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_channel;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t			*p_server;

	assert (channel != NULL);
	assert (nickname != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	p_server = Server_Find (config->server_name);
	assert (p_server != NULL);
	assert (p_server->numeric != NULL);

	p_channel = Channel_Find (channel);
	if (!p_channel)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Channel [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, channel
		);

		return ;
	}

	p_nickname = Nickname_Find (nickname);
	if (!p_nickname)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] not found\n",
			__FILE__, __LINE__, __FUNCTION__, nickname
		);

		return ;
	}

	p_nickname_channel = NicknameChannel_Find (p_nickname, p_channel);
	p_channel_nickname = ChannelNickname_Find (p_channel, p_nickname);

	if ( (!p_nickname_channel) || (!p_channel_nickname) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] was not found in Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}

	if ( (!(p_nickname_channel->flags & IS_CHANVOICE)) && (!(p_channel_nickname->flags & IS_CHANVOICE)) )
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] is already a CHANDEVOICE on Channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_nickname->nickname, p_channel->name
		);

		return ;
	}

	p_nickname_channel->flags &= ~IS_CHANVOICE;
	p_channel_nickname->flags &= ~IS_CHANVOICE;

	ssprintf (s,
		"%s M %s -v %s\n",
		p_server->numeric, p_channel->name, p_nickname->numeric);

	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_kill ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_kill)
                |    This function will handle sending KILL commands on using the Undernet P10 Protocol
                |
 INPUTS         |    src    = Source of the KILL (could be service nickname or server)
                |    dst    = Destination of the KILL
                |    format = Kill reason and any arguments that make up the kill reason
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_kill (char *src, char *dst, char *format, ...)
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname_src;
	nickname_t		*p_nickname_dst;
	nickname_channel_t	*p_nickname_channel;
	hostname_t		*p_hostname;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nickname;
	server_t		*p_server;
	module_t		*module_p;
	module_callback_t	*module_callback_p;
	int			i_server_kill = 0;
	char			buf [256];
	va_list			msg;

	assert (dst != NULL);
	assert (format != NULL);

	va_start (msg, format);
	vsprintf (buf, format, msg);


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First we need to determine if the KILL is coming from a service nickname or our server.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (src != NULL)
	{
		p_nickname_src = Nickname_Find (src);
		p_server = Server_Find (src);

		if ( (p_server) && (!p_nickname_src) )
		{
			i_server_kill = 1;
		}
		else if ( (!p_server) && (p_nickname_src) )
		{
			i_server_kill = 0;
		}
	}
	else
	{
		p_server = Server_Find (config->server_name);
		assert (p_server != NULL);

		i_server_kill = TRUE;
	}

	p_nickname_dst = Nickname_Find (dst);
	if (!p_nickname_dst)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Nickname [%s] was not found\n",
			__FILE__, __LINE__, __FUNCTION__, dst
		);

		return ;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------

	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (i_server_kill)
	{
		ssprintf (s,
			"%s D %s :%s\n",
			p_server->numeric, p_nickname_dst->numeric, buf);
	}

	else
	{
		ssprintf (s,
			"%s D %s :%s\n",
			p_nickname_src->numeric, p_nickname_dst->numeric, buf);
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Run any m_kill callbacks on our services modules
	-------------------------------------------------------------------------------------------------------------------------
	*/
	module_p = module_h;
	while (module_p)
	{
		if (module_p->ModuleType & MODULETYPE_SERVICE)
		{
			module_callback_p = ModuleCallback_Find (module_p, "m_kill");
			if (module_callback_p)
			{
				module_callback_p->func (module_p, p_nickname_dst);
			}
		}

		module_p = module_p->next;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	Update our lists -

		1) Hostname List (Update hostcount or delete hostname if its the last hostname)
		3) ChannelNickname/NicknameChannel - Remove user from any channels they are on
	-------------------------------------------------------------------------------------------------------------------------
	*/


	p_hostname = Hostname_Find (p_nickname_dst->hostname_p->hostname);
	assert (p_hostname != NULL);

	if (p_hostname->numHosts > 1)
	{
		p_hostname->numHosts--;
	}
	else
	{
		Hostname_Del (p_hostname->hostname);
		status->numHosts--;
	}

	if (p_nickname_dst->flags & NICK_IS_OPER)
	{
		Operator_Del (p_nickname_dst);
		p_nickname_dst->flags &= ~NICK_IS_OPER;
	}

	p_nickname_channel = p_nickname_dst->channel_h;
	while (p_nickname_channel)
	{
		p_channel = p_nickname_channel->channel_p;
		ChannelNickname_Del (p_channel, p_nickname_dst);
		NicknameChannel_Del (p_nickname_dst, p_channel);
		p_nickname_channel = p_nickname_channel->next;
	}

	Nickname_Del (p_nickname_dst->nickname);


	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_cmd_nickjupe ()
                |
 DESCRIPTION    |    Undernet Protocol (cmd_nickjupe)
                |    This function will handle sending NICKJUPE commands on using the Undernet P10 Protocol
                |
 INPUTS         |    nickname = Nickname we are JUPEing
                |    username = Username for the JUPE
                |    hostnme  = Hostname for the JUPE
                |    format   = Reason and any arguments that make up the reason
                |
 RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void libundernet_cmd_nickjupe (char *nickname, char *username, char *hostname, char *format, ...)
{
	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*nickname_p;
	nickname_channel_t	*nickname_channel_p;
	nickname_numeric_t	*nickname_numeric_p;
	channel_t		*channel_p;
	hostname_t		*hostname_p;
	server_t		*server_p;
	module_t		*module_p;
	module_callback_t	*module_callback_p;
	char			*c_numeric;
	char			buf [256];
	va_list			msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	assert (nickname != NULL);
	assert (username != NULL);
	assert (hostname != NULL);
	assert (format != NULL);
	assert (config != NULL);
	assert (config->server_name != NULL);

	server_p = Server_Find (config->server_name);
	assert (server_p != NULL);
	assert (server_p->numeric != NULL);

	c_numeric = (char *)ircsp_calloc (1, 20);
	NicknameNumeric_Generate (c_numeric);
	nickname_numeric_p = NicknameNumeric_Add (c_numeric);

	ssprintf (s,
		"%s N %s 1 0 %s %s %s AAAAAA %s :%s\n",
		server_p->numeric, nickname, username, hostname,
		"+idwk", c_numeric, buf);


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First lets check and see if the nickname is already online and update our lists and flags accordingly
	-------------------------------------------------------------------------------------------------------------------------
	*/
	nickname_p = Nickname_Find (nickname);
	if (nickname_p)
	{
		module_p = module_find_nickserv ();
		if (module_p)
		{
			if (nickname_p->flags & NICK_IS_IDENTIFIED)
			{
				module_callback_p = ModuleCallback_Find (module_p, "nickinfo_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (nickname_p);
				}
			}

			if (nickname_p->flags & NICK_IS_ROOT)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_root_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (nickname_p);
				}
			}

			if (nickname_p->flags & NICK_IS_ADMIN)
			{
				module_callback_p = ModuleCallback_Find (module_p, "services_admin_unregister");
				if (module_callback_p)
				{
					module_callback_p->func (nickname_p);
				}
			}
		}

		if (nickname_p->flags & NICK_IS_OPER)
		{
			nickname_p->flags &= ~NICK_IS_OPER;
			Operator_Del (nickname_p);
		}

		hostname_p = nickname_p->hostname_p;
		assert (hostname_p != NULL);
		assert (hostname_p->hostname != NULL);

		if (hostname_p->numHosts > 1)
		{
			hostname_p->numHosts--;
		}
		else
		{
			Hostname_Del (hostname_p->hostname);
		}

		nickname_channel_p = nickname_p->channel_h;
		while (nickname_channel_p)
		{
			channel_p = nickname_channel_p->channel_p;
			ChannelNickname_Del (channel_p, nickname_p);
			NicknameChannel_Del (nickname_p, channel_p);

			nickname_channel_p = nickname_channel_p->next;
		}

		Nickname_Del (nickname_p->nickname);
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	First lets check and see if the nickname is already online and update our lists and flags accordingly
	-------------------------------------------------------------------------------------------------------------------------
	*/
	hostname_p = Hostname_Find (hostname);
	if (hostname_p)
	{
		hostname_p->numHosts++;
	}
	else
	{
		hostname_p = Hostname_Add (hostname);
		assert (hostname_p != NULL);
		assert (hostname_p->hostname != NULL);
	}

	nickname_p = Nickname_Add (nickname, username, hostname_p, server_p, time (NULL));
	assert (nickname_p != NULL);
	assert (nickname_p->nickname != NULL);
	assert (nickname_p->username != NULL);
	assert (nickname_p->hostname_p != NULL);
	assert (nickname_p->server_p != NULL);


	nickname_p->flags |= NICK_IS_JUPE;

	nickname_p->numeric = (char *)ircsp_calloc (1, strlen (c_numeric) + 5);
	strcpy (nickname_p->numeric, c_numeric);

	ircsp_free (c_numeric);

	assert (nickname_p->numeric != NULL);



	if (LOG_FUNC)
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
