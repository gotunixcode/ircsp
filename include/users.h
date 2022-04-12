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


#ifndef __INCLUDED_CORE_LISTS_NICKNAME_H__
#define __INCLUDED_CORE_LISTS_NICKNAME_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		E			extern
#define		NUMNICKLOG		6
#define		NUMNICKMASK		63


/*
---------------------------------------------------------------------------------------------------------------------------------
 FLAGS
---------------------------------------------------------------------------------------------------------------------------------
*/
enum
{
	NICK_IS_JUPE		=	1,
	NICK_IS_OPER		=	2,
	NICK_IS_IDENTIFIED	=	4,
	NICK_IS_ADMIN		=	8,
	NICK_IS_ROOT		=	16,
	NICK_IS_SERVICE		=	32
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _nickname
{
	char				*nickname;			/* Nickname */
	char				*username;			/* Username */
	struct _hostname		*hostname_p;			/* Pointer to hostname record for my hostname */
	struct _server			*server_p;			/* Pointer to the server record user is on */
	char				*numeric;			/* Numeric (only needed for undernet) */
	int				flags;				/* NICK_IS flags */
	time_t				stamp;				/* Timestamp user connected */
	int				numChans;			/* How many channels am i in */
	struct	_nickname_channel	*channel_h;			/* Channels that the User is in */
	struct	_nickname		*next;				/* Next record in Users list */
	struct	_admin_list		*admin_p;
} nickname_t;


typedef struct _nickname_numeric
{
	char				*numeric;
	struct _nickname		*nickname_p;			/* Pointer to nickname using the numeric */
	struct _nickname_numeric	*next;
} nickname_numeric_t;


typedef struct _hostname
{
	char				*hostname;
	int				numHosts;

	struct	_hostname		*next;
} hostname_t;


typedef struct _operator
{
	struct	_nickname		*nickname_p;
	struct _operator		*next;
} operator_t;


typedef struct _nickname_channel
{
	struct	_channel		*channel_p;
	int				flags;
	struct	_nickname_channel	*next;
} nickname_channel_t;


nickname_t				*nickname_h;
nickname_numeric_t			*nickname_numeric_h;
operator_t				*operator_h;
hostname_t				*hostname_h;


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	const char			convert2y[];
E	const unsigned int		convert2n[];
E	unsigned int			base64toint				(const char *);
E	char				*intobase64				(char *buf, unsigned int, unsigned int);
E	nickname_t			*Nickname_Add				(char *, char *, struct _hostname *, struct _server *, time_t);
E	nickname_t			*Nickname_Find				(char *);
E	void				Nickname_Del				(char *);
E	void				Nickname_DelServer			(struct _server *);
E	hostname_t			*Hostname_Add				(char *);
E	hostname_t			*Hostname_Find				(char *);
E	void				Hostname_Del				(char *);
E	operator_t			*Operator_Add				(struct _nickname *);
E	operator_t			*Operator_Find				(struct _nickname *);
E	void				Operator_Del				(struct _nickname *);
E	nickname_channel_t		*NicknameChannel_Add			(nickname_t *, struct _channel *);
E	nickname_channel_t		*NicknameChannel_Find			(nickname_t *, struct _channel *);
E	void				NicknameChannel_Del			(nickname_t *, struct _channel *);
E	char				*NicknameNumeric_Generate		(char *);
E	nickname_numeric_t		*NicknameNumeric_Add			(char *);
E	nickname_numeric_t		*NicknameNumeric_Find			(char *);
E	void				NicknameNumeric_Del			(char *);

#endif		/* __INCLUDED_CORE_LISTS_NICKNAME_H__ */
