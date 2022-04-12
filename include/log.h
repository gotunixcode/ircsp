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


#ifndef		__INCLUDED_CORE_LOG_H__
#define		__INCLUDED_CORE_LOG_H__
#define		E		extern

/*
---------------------------------------------------------------------------------------------------------------------------------
 VARIABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
extern	FILE		*F_MAINLOG;
extern	int		MAINLOG_OPENED;


/*
---------------------------------------------------------------------------------------------------------------------------------
    STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
typedef struct _logfile
{
	char			*log_name;
	FILE			*log_file;
	int			log_open;

	struct _logfile		*next;
} logfile_t;


logfile_t			*logfile_h;
logfile_t			*main_logfile_p;


/*
---------------------------------------------------------------------------------------------------------------------------------
    LOG LEVELS
---------------------------------------------------------------------------------------------------------------------------------
*/
enum
{
		LOG_CRITICAL		=	1,
		LOG_ERROR		=	2,
		LOG_WARNING		=	4,
		LOG_INFO		=	8,
		LOG_FUNC		=	16,
		LOG_ALL			=	32
};


#define		LOG_LEVEL		0x000032		/* What level ware we logging */


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define			LOG_MESSAGE_COUNT					200

/* General Messages - 1 - 50 */

#define			LOG_MESSAGE_OUTOFMEMORY					1		// 0001
#define			LOG_MESSAGE_FUNCSTART					4		// 0004
#define			LOG_MESSAGE_FUNCEND					5		// 0005
#define			LOG_MESSAGE_FUNCEND_ERROR				6		// 0006
#define			LOG_MESSAGE_IRCSP_BUILD					10		// 0010
#define			LOG_MESSAGE_IRCSP_INIT					11		// 0011
#define			LOG_MESSAGE_NULLPTR					15		// 0015
#define			LOG_MESSAGE_ALLOCFAILFOR				20		// 0020

#define			LOG_MESSAGE_NOSUCHNICK					40		// 0040

/* Protocol Undernet (BURST) Messages (81 -) */
#define			BURST_MSG_PROC						81		// 0081
#define			BURST_MSG_PROC_CONTINUE					82		// 0082
#define			BURST_MSG_PROC_USERLIST					83		// 0083
#define			BURST_MSG_PROC_BANLIST					84		// 0084


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	void		ircsp_log					(FILE *, char *, ...);
E	void		LOG						(logfile_t *, int, char *, ...);
E	void		ircsp_error					(int, int, char *, ...);
E	char		*get_log_message				(int);
E	logfile_t	*logfile_register				(char *);
E	logfile_t	*logfile_search					(char *);
E	void		logfile_remove					(char *);


#endif		/* __INCLUDED_CORE_LOG_H__ */
