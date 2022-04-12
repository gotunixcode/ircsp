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


#ifndef		__INCLUDED_LIBNICKSERV_ADMIN_H__
#define		__INCLUDED_LIBNICKSERV_ADMIN_H__
#define		E			extern


/*
---------------------------------------------------------------------------------------------------------------------------------
    FLAGS
---------------------------------------------------------------------------------------------------------------------------------
*/
enum
{
	ADMIN_SUSPENDED	=	1,
	ADMIN_INUSE	=	2
};


/*
---------------------------------------------------------------------------------------------------------------------------------
    STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _services_admin
{
	char			*nickname;			/* Nickname of Admin */
	int			flags;				/* Flags */

	struct _nickname	*nickname_p;			/* Nickname currently logged in */
	struct _nickinfo	*nickinfo_p;			/* NickServ account linked to SA */
	struct _services_admin	*next;				/* Pointer to next SA record */
} services_admin_t;


typedef struct _services_root
{
	char			*nickname;			/* Nickname of ROOT */
	int			flags;				/* Flags */

	struct _nickname	*nickname_p;			/* Pointer to Nickname Record */
	struct _nickinfo	*nickinfo_p;			/* Pointer to Nickinfo Record */
	struct _services_root	*next;				/* Pointer to next ROOT record */
} services_root_t;


services_root_t			*services_root_h;
services_admin_t		*services_admin_h;


/*
---------------------------------------------------------------------------------------------------------------------------------
    PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	services_root_t			*ServicesRoot_Create			(char *);
E	services_root_t			*ServicesRoot_Search			(char *);
E	void				ServicesRoot_Remove			(char *);
E	void				NS_ServicesRoot_unregister		(struct _nickname *);
E	services_admin_t		*ServicesAdmin_Create			(char *);
E	services_admin_t		*ServicesAdmin_Search			(char *);
E	void				ServicesRoot_Remove			(char *);
E	void				services_admin_unregister		(struct _nickname *);



#endif		/* __INCLUDED_LIBNICKSERV_ADMIN_H__ */
