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


#if !defined (__UWORLD_H__)
#define __UWORLD_H__


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define UWORLD_VERSION	"1.1.3 - PRERELEASE"
#define UW_NUM_CMDS 35


/*
---------------------------------------------------------------------------------------------------------------------------------
 STRUCTURES
---------------------------------------------------------------------------------------------------------------------------------
*/
struct uworld_comtab
{
	char	command[20];
	int (*func)(int, char *[]);
	int minlevel;
	int minargs;
	int disabled;
	char *reason;
};


typedef struct _gline
{
	char		*mask;
	char		*reason;
	char		*setby;
	time_t	 setwhen;
	time_t	 expires;

	struct	_gline *next;
} Gline;

Gline *glineHEAD;

typedef struct _nickjupe
{
	char	*nickname;
	char	*reason;
	int	flags;

	struct _nickjupe *next;
} NickJupe;

NickJupe *nickjupeHEAD;


/*
---------------------------------------------------------------------------------------------------------------------------------
 PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
extern	Gline		*addGline		(char *, time_t, time_t, char *, char *);
extern	Gline		*findGline		(char *);
extern	void		delGline		(char *);
extern	NickJupe	*addNickJupe		(char *, char *, int);
extern	NickJupe	*findNickJupe		(char *);
extern	void		delNickJupe		(char *);
extern	void		do_uworld		(char *);
extern	int		loadUWCmd		(void);
extern	int		writeUWCmd		(void);
extern	int		uw_loadCommand		(void);
extern	int		uw_saveCommand		(void);
extern	int		uw_loadGlineDB		(void);
extern	int		uw_saveGlineDB		(void);
extern	int		uw_loadNickJupe		(void);
extern	int		uw_saveNickJupe		(void);
extern	int		uw_version		(int, char *[]);
extern	int		uw_help			(int, char *[]);
extern	int		uw_operlist		(int, char *[]);
extern	int		uw_verify		(int, char *[]);
extern	int		uw_servlist		(int, char *[]);
extern	int		uw_nicklist		(int, char *[]);
extern	int		uw_hostlist		(int, char *[]);
extern	int		uw_adminlist		(int, char *[]);
extern	int		uw_glinelist		(int, char *[]);
extern	int		uw_chanlist		(int, char *[]);
extern	int		uw_cexemptlist		(int, char *[]);
extern	int		uw_uptime		(int, char *[]);
extern	int		uw_auth			(int, char *[]);
extern	int		uw_deauth		(int, char *[]);
extern	int		uw_banlist		(int, char *[]);
extern	int		uw_gline		(int, char *[]);
extern	int		uw_remgline		(int, char *[]);
extern	int		uw_whoison		(int, char *[]);
extern	int		uw_xlate		(int, char *[]);
extern	int		uw_whois		(int, char *[]);
extern	int		uw_clearbans		(int, char *[]);
extern	int		uw_opermsg		(int, char *[]);
extern	int		uw_clearops		(int, char *[]);
extern	int		uw_opcom		(int, char *[]);
extern	int		uw_clearmodes		(int, char *[]);
extern	int		uw_scan			(int, char *[]);
extern	int		uw_mode			(int, char *[]);
extern	int		uw_masskill		(int, char *[]);
extern	int		uw_disablecmd		(int, char *[]);
extern	int		uw_enablecmd		(int, char *[]);
extern	int		uw_cmdlist		(int, char *[]);
extern	int		uw_save			(int, char *[]);
extern	int		uw_die			(int, char *[]);
extern	int		uw_restart		(int, char *[]);
extern	int		uw_rehash		(int, char *[]);
extern	int		uw_chlevel		(int, char *[]);


#endif		/* __UWORLD_H__ */
