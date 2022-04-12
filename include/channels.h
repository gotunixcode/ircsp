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


#ifndef __INCLUDED_CORE_LISTS_CHANNELS_H__
#define __INCLUDED_CORE_LISTS_CHANNELS_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		E		extern


/*
---------------------------------------------------------------------------------------------------------------------------------
 FLAGS
---------------------------------------------------------------------------------------------------------------------------------
*/
enum
{
	CHANNEL_HAS_LIMIT		=	1,
	CHANNEL_HAS_KEY			=	2,
	CHANNEL_LOCKED			=	4
};

enum
{
	IS_CHANOP			=	1,
	IS_CHANVOICE			=	2
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _channel
{
	char				*name;
	char				*key;
	int				flags;
	int				limit;
	time_t				created;
	int				numBans;
	int				numUsers;

	struct	_channel_ban		*channel_ban_h;
	struct	_channel_nickname	*channel_nickname_h;
	struct	_channel		*next;
} channel_t;


typedef struct _channel_nickname
{
	struct	_nickname		*nickname_p;
	int				flags;
	struct _channel_nickname	*next;
} channel_nickname_t;


typedef struct _channel_ban
{
		char			*mask;
		struct	_channel_ban	*next;
} channel_ban_t;

channel_t				*channel_h;


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	channel_t		*Channel_Add				(char *, int, time_t, char *, int);
E	channel_t		*Channel_Find				(char *);
E	void			Channel_Del				(char *);
E	channel_nickname_t	*ChannelNickname_Add			(struct _channel *channelp, struct _nickname *userp);
E	channel_nickname_t	*ChannelNickname_Find			(struct _channel *channelp, struct _nickname *userp);
E	void			ChannelNickname_Del			(struct _channel *channelp, struct _nickname *userp);
E	channel_ban_t		*ChannelBan_Add				(channel_t *, char *);
E	channel_ban_t		*ChannelBan_Find			(channel_t *, char *);
E	void			ChannelBan_Del				(channel_t *, char *);


E	void			channel_remove				(char *);
E	void			channel_user_add			(struct _channel *, struct _nickname *);
E	void			channel_user_op				(struct _channel *, struct _nickname *);
E	void			channel_user_deop			(struct _channel *, struct _nickname *);
E	void			channel_user_voice			(struct _channel *, struct _nickname *);
E	void			channel_user_devoice			(struct _channel *, struct _nickname *);


#endif		/* __INCLUDED_CORE_LISTS_CHANNELS_H__ */
