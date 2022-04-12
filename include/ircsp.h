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


#ifndef __INCLUDED_CORE_IRCSP_H__
#define __INCLUDED_CORE_IRCSP_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		E		extern
#define		TRUE		1
#define		FALSE		0


/*
---------------------------------------------------------------------------------------------------------------------------------
 VARIABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
//E	int		DEBUG;


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _ircsp
{
	int		argc;
	char**		argv;
	pid_t		pid;
	uid_t		uid;
	uid_t		euid;
	time_t		uptime;
	int		connected;
	int		initial_burst;
	int		services_online;
	time_t		scheduler_last_run;			/* Last time the scheduler ran */
	time_t		last_save_cycle;
	time_t		last_gline_cycle;
	time_t		last_services_admin_save;		/* Last time we saved the ServicesAdmin database */
	time_t		last_nickserv_save;			/* Last time we saved NickServ databases */
	time_t		last_nickserv_expire;			/* Last time we expired NickServ registrations */
	time_t		last_nickserv_jupe_expire;		/* Last time we expired NickServ Jupes */
	time_t		last_nickserv_validate;			/* Last time we validated online users (Identify/autokill/etc) */
	int		rehashing;
	int		quitting;
} IRCSP;


typedef struct _status
{
	int		numUsers;
	int		numServers;
	int		numChannels;
	int		numOpers;
	int		numAdmins;
	int		numHosts;
	int		numGlines;
	int		numNumeric;
} Status;


typedef struct _service_list
{
	struct _nickname		*nickserv;
	struct _nickname		*operserv;
} Service;


IRCSP *ircsp;
Status *status;
Service *service;


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	int		ircsp_file_exists				(char *);
E	int		ircsp_check_access				(const char *, int);
E	int		ircsp_init					(int, char *[]);
E	void		ircsp_exit					(int, char *, ...);
E	void		ircsp_panic					(char *, ...);
E	void		ircsp_restart					(void);
E	void		ircsp_spawnUW					(void);
E	void		ircsp_mainloop					(void);
E	void		ircsp_join_operchannel				(void);


#endif		/* __INCLUDED_CORE_IRCSP_H__ */
