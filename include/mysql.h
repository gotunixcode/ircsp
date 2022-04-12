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


#ifndef __INCLUDED_MYSQL_H__
#define __INCLUDED_MYSQL_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 DATABASE TABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		NICKSERV_TABLE		"(						\
						id INT AUTO_INCREMENT PRIMARY KEY,	\
						nickname VARCHAR(255) NOT NULL,		\
						password VARCHAR(255) NOT NULL,		\
						last_login VARCHAR(255) NOT NULL,	\
						last_seen INT(11) NOT NULL,		\
						registered INT(11) NOT NULL,		\
						flags INT(8) NOT NULL			\
					)  ENGINE=INNODB;"

#define		NICKSERV_ACCESS_TABLE	"(						\
						id INT AUTO_INCREMENT PRIMARY KEY,	\
						nickname VARCHAR(255) NOT NULL,		\
						hostmask VARCHAR(255) NOT NULL		\
					)  ENGINE=INNODB;"

#define		SERVICES_ADMIN_TABLE	"(						\
						id INT AUTO_INCREMENT PRIMARY KEY,	\
						nickname VARCHAR(255) NOT NULL,		\
						flags INT(10) NOT NULL			\
						)  ENGINE=INNODB;"



#define		NSDB_NICKSERV			"nickserv"
#define		NSDB_NICKSERV_ACCESS		"nickserv_access"
#define		NSDB_SERVICES_ADMIN		"services_admin"


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		E		extern



#define		HOSTNAME_EXEMPT_TABLE	"(id INT AUTO_INCREMENT PRIMARY KEY,hostmask VARCHAR(255) NOT NULL,count INT(10) NOT NULL)  ENGINE=INNODB;"


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	void		*ircsp_mysql_connect			(void);
E	void		ircsp_mysql_disconnect			();
E	int		ircsp_mysql_backup_table		(char *);
E	int		ircsp_mysql_create_table		(char *, char *);
E	int		ircsp_mysql_insert			(char *, char *, ...);
E	int		ircsp_mysql_update			(char *, char *, ...);
E	int		ircsp_mysql_delete			(char *, char *, ...);

#endif		/* __INCLUDED_MYSQL_H__ */
