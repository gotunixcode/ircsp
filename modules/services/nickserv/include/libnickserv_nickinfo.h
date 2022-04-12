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


#ifndef		__INCLUDED_LIBNICKSERV_NICKINFO_H__
#define		__INCLUDED_LIBNICKSERV_NICKINFO_H__
#define		E		extern


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		NICKSERV_EXPIRE			2592000			/* Time in Seconds when a nickname will be automatically
									   DROPPED from the Registration database.
									*/

#define		NICKSERV_JUPE_EXPIRE		1800			/* Time in seconds before a nick is released automatically
									   after it is JUPED.
									*/


/*
---------------------------------------------------------------------------------------------------------------------------------
 FLAGS
---------------------------------------------------------------------------------------------------------------------------------
*/
enum
{
	NICKSERV_KILLPROTECT		=	1,			/* NickInfo flag for autokill protection */
	NICKSERV_SECURE			=	2,			/* NickInfo flag to enable nick secure */
	NICKSERV_AUTOIDENTIFY		=	4,			/* NickInfo flag to enable auto identify */
	NICKSERV_SUSPENDED		=	256			/* Nickinfo flag for susendending nickname */
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _nickinfo
{
	char				*nickname;			/* NickServ Registered Nickname */
	char				*password;			/* Password */
	char				*last_login;			/* user@host that last IDENTIFIED */
	time_t				last_seen;			/* Timestamp user was last seen */
	time_t				registered;			/* Timestamp the nickname was registered */
	int				flags;				/* Flags set on the nick */


	struct _nickname		*nickname_p;			/* Pointer to the nickname IDENTIFIED for the nick */
	struct _nickinfo_access		*nickinfo_access_h;		/* Access list head */
	struct _nickinfo		*next;				/* Pointer to the next record in the list */
} nickinfo_t;


typedef struct _nickinfo_access
{
	char				*hostmask;			/* USER@HOST in Access List */

	struct _nickinfo		*nickinfo_p;			/* Pointer to the Registered Nickname this entry belongs to */
	struct _nickinfo_access		*next;				/* Pointer to next record in the list */
} nickinfo_access_t;


typedef struct _nickinfo_jupe
{
	char				*nickname;			/* JUPE Nickname */
	time_t				timestamp;			/* Timestamp JUPE was created */
	struct _nickname		*nickname_p;			/* Pointer to nickname record for the JUPE */
	struct _nickinfo		*nickinfo_p;			/* Pointer to the registration */
	struct _nickinfo_jupe		*next;				/* Pointer to the next record on the JUPE list */
} nickinfo_jupe_t;


nickinfo_t				*nickinfo_h;
nickinfo_jupe_t				*nickinfo_jupe_h;


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	nickinfo_t			*Nickinfo_Create			(char *);
E	nickinfo_t			*Nickinfo_Search			(char *);
E	void				Nickinfo_Remove				(char *);
E	void				nickinfo_unregister			(struct _nickname *);
E	int				nickinfo_expire				(void);
E	int				nickinfo_validate			(void);
E	nickinfo_access_t		*NickinfoAccess_Create			(struct _nickinfo *, char *);
E	nickinfo_access_t		*NickinfoAccess_Search			(struct _nickinfo *, char *);
E	nickinfo_access_t		*NickinfoAccess_Wildcard		(struct _nickinfo *, char *);
E	void				NickinfoAccess_Remove			(struct _nickinfo *, char *);
E	nickinfo_jupe_t			*NickinfoJupe_Create			(char *);
E	nickinfo_jupe_t			*NickinfoJupe_Search			(char *);
E	void				NickinfoJupe_Remove			(char *);
E	int				nickinfo_jupe_expire			(void);


#endif		/* __INCLUDED_NICKSERV_H__ */
