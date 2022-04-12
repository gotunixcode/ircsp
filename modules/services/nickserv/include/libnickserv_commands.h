
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


#ifndef		__INCLUDED_LIBNICKSERV_COMMANDS_H__
#define		__INCLUDED_LIBNICKSERV_COMMANDS_H__
#define		E			extern


/*
---------------------------------------------------------------------------------------------------------------------------------
    DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		NICKSERV_NUM_COMMANDS		9


/*
---------------------------------------------------------------------------------------------------------------------------------
    STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
struct nickserv_commands
{
	char		command [64];					/* Name of the Command */
	int		(*func)(int, char *[]);				/* Function that handles the command */
	int		minargs;					/* Minimum arguments required for the command */
	int		disabled;					/* Is the command disabled */
	int		ident;						/* IDENT required for command */
	int		minlevel;					/* Minimum Level required for command */
};


/*
---------------------------------------------------------------------------------------------------------------------------------
    PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	void		m_privmsg		(struct _module *, char *, char *, char *);
E	int		ns_version		(int, char *[]);
E	int		ns_help			(int, char *[]);
E	int		ns_register		(int, char *[]);
E	int		ns_identify		(int, char *[]);
E	int		ns_drop			(int, char *[]);
E	int		ns_access		(int, char *[]);
E	int		ns_set			(int, char *[]);
E	int		ns_ghost		(int, char *[]);
E	int		ns_recover		(int, char *[]);
E	int		ns_release		(int, char *[]);


#endif		/* __INCLUDED_LIBNICKSERV_COMMANDS_H__ */
