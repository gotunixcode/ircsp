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


#ifndef __INCLUDED_CORE_LISTS_SERVER_H__
#define __INCLUDED_CORE_LISTS_SERVER_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
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
	SERVER_IN_BURST		=	1,
	SERVER_IS_HUB		=	2,
	SERVER_IS_MY_UPLINK	=	4,
	SERVER_IS_JUPED		=	8
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _server
{
	char				*name;			/* Name of server */
	char				*numeric;		/* Numeric of server */
	time_t				created;		/* Time server was created */
	int				flags;			/* Server flags */
	int				numUsers;		/* Number of users on this server */
	int				numOpers;		/* Number of operators on this server */
	struct _server			*my_uplink_p;		/* Pointer to our UPLINK server */
	struct _server			 *next;			/* Pointer to the next record */
} server_t;

server_t		*server_h;
server_t		*Num2Server[64];


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	server_t		*Server_Add				(char *, time_t);
E	server_t		*Server_Find				(char *);
E	void			Server_Del				(char *);


#endif		/* __INCLUDED_CORE_LISTS_SERVER_H__ */
