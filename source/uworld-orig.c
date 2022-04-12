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


/* Standard Includes */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdarg.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/wait.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<ctype.h>
#include	<time.h>
#include	<errno.h>
#include	<assert.h>
#include	<netinet/in.h>


/* IRCSP Core Includes */
#include	"rehash.h"
#include	"synch.h"
#include	"misc.h"
#include	"uworld.h"
#include	"channels.h"
#include	"users.h"
#include	"servers.h"
#include	"conf.h"
#include	"cmds.h"
#include	"version.h"
#include	"alloc.h"
#include	"text.h"
#include	"match.h"
#include	"socket.h"
#include	"ircsp.h"
#include	"log.h"
#include	"mysql.h"
#include	"modules.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
    Uworld Command Structure

        Command Name,
        Command Function,
        Default Level,
        Min Arguments
        Disabled
---------------------------------------------------------------------------------------------------------------------------------
*/
struct uworld_comtab UWorldCommand[] = {
	{	"version",		&uw_version,		0,		3,	0,	NULL	},
	{	"help",			&uw_help,		0,		3,	0,	NULL	},
	{	"operlist",		&uw_operlist,		0,		3,	0,	NULL	},
	{	"verify",		&uw_verify,		0,		4,	0,	NULL	},
	{	"uptime",		&uw_uptime,		1,		3,	0,	NULL	},
	{	"servlist",		&uw_servlist,		1,		3,	0,	NULL	},
	{	"nicklist",		&uw_nicklist,		1,		3,	0,	NULL	},
	{	"hostlist",		&uw_hostlist,		1,		3,	0,	NULL	},
	{	"adminlist",		&uw_adminlist,		1,		3,	0,	NULL	},
	{	"glinelist",		&uw_glinelist,		1,		3,	0,	NULL	},
	{	"chanlist",		&uw_chanlist,		1,		3,	0,	NULL	},
	{	"auth",			&uw_auth,		1,		5,	0,	NULL	},
	{	"deauth",		&uw_deauth,		1,		3,	0,	NULL	},
	{	"gline",		&uw_gline,		1,		5,	0,	NULL	},
	{	"remgline",		&uw_remgline,		1,		4,	0,	NULL	},
	{	"whoison",		&uw_whoison,		1,		4,	0,	NULL	},
	{	"banlist",		&uw_banlist,		1,		4,	0,	NULL	},
	{	"xlate",		&uw_xlate,		1,		4,	0,	NULL	},
	{	"whois",		&uw_whois,		1,		4,	0,	NULL	},
	{	"clearbans",		&uw_clearbans,		1,		4,	0,	NULL	},
	{	"opermsg",		&uw_opermsg,		1,		4,	0,	NULL	},
	{	"clearops",		&uw_clearops,		1,		4,	0,	NULL	},
	{	"opcom",		&uw_opcom,		1,		6,	0,	NULL	},
	{	"clearmodes",		&uw_clearmodes,		1,		4,	0,	NULL	},
	{	"scan",			&uw_scan,		1,		4,	0,	NULL	},
	{	"mode",			&uw_mode,		1,		5,	0,	NULL	},
	{	"masskill",		&uw_masskill,		1,		5,	0,	NULL	},
	{	"save",			&uw_save,		1,		3,	0,	NULL	},
	{	"die",			&uw_die,		1,		3,	0,	NULL	},
	{	"cmdlist",		&uw_cmdlist,		1,		3,	0,	NULL	},
	{	"disablecmd",		&uw_disablecmd,		1,		4,	0,	NULL	},
	{	"enablecmd",		&uw_enablecmd,		1,		4,	0,	NULL	},
	{	"restart",		&uw_restart,		1,		4,	0,	NULL	},
	{	"rehash",		&uw_rehash,		1,		3,	0,	NULL	},
	{	"chlevel",		&uw_chlevel,		1,		5,	0,	NULL	},
};


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_loadCommand ()
                   |
    DESCRIPTION    |    This function will load the UWorld Command Database which will consist of, the command name, command level
                   |    and weather or not the command is disabled, allowing admins to change the access requirements of commands
                   |    without having to recompile.
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_loadCommand (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	FILE		*f;
	int		c;
	int		level;
	int		disabled;
	char		read[512];
	char		*command;


	f = fopen (config->uw_cmddb, "r");
	if (!f)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to load uworld levels database\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return FALSE;
	}
	else
	{
		while (!feof (f))
		{
			if (fgets (read, 255, f))
			{
				if (is_comment (read))
				{
					continue;
				}

				if (is_blank (read))
				{
					continue;
				}

				command		= strtok (read, ":");
				level		= atoi (strtok (NULL, ":"));
				disabled	= atoi (strtok (NULL, ":"));

				for (c = 0; c <  UW_NUM_CMDS; c++)
				{
					if (!strcasecmp (command, UWorldCommand[c].command))
					{
						if (disabled)
						{
							UWorldCommand[c].disabled = disabled;
						}

						if (level)
						{
							UWorldCommand[c].minlevel = level;
						}
					}
				}
			}
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_saveCommand ()
                   |
    DESCRIPTION    |    This function will save the command database out to disk
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_saveCommand (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*myuserp;
	FILE			*f;
	int			cmd;

	myuserp = Nickname_Find (config->uworld_nickname);
	if (myuserp)
	{
		cmd_notice_operators (myuserp->nickname,
			"Saving UWorld command database..."
		);
	}

	if (!(f = fopen (config->uw_cmddb, "w")))
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to write to uworld command database\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return FALSE;
	}


	fprintf(f,
		"; UWorld Command Level DB [%s]\n",
		config->uw_cmddb);
	fprintf(f,
		"; Last Saved on: %lu\n",
		time (NULL));

	for (cmd = 0; cmd < UW_NUM_CMDS; cmd++)
	{
		fprintf (f,
			"%s:%d:%d:\n",
			UWorldCommand[cmd].command,
			UWorldCommand[cmd].minlevel,
			UWorldCommand[cmd].disabled);
	}

	fclose (f);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    addGline ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
Gline
*addGline (mask, setwhen, expires, reason, setby)
	char		*mask;
	time_t		setwhen;
	time_t		expires;
	char		*reason;
	char		*setby;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	Gline		*glinep;

	glinep = (Gline *)ircsp_calloc (1, sizeof (Gline));
	glinep->mask = (char *)ircsp_calloc (1, strlen (mask) + 5);
	glinep->reason = (char *)ircsp_calloc (1, strlen (reason) + 5);
	glinep->setby = (char *)ircsp_calloc (1, strlen (setby) + 5);

	if (!glinep)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for gline_p\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return NULL;
	}

	strcpy (glinep->mask, mask);
	strcpy (glinep->reason, reason);
	strcpy (glinep->setby, setby);
	glinep->setwhen = setwhen;
	glinep->expires = expires;


	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Adding G:Line [%s] (%s)\n",
			__FILE__, __LINE__, __FUNCTION__,
			glinep->mask, glinep->reason
		);
	}

	if (!glineHEAD)
	{
		glineHEAD = glinep;
		glinep->next = NULL;
	}
	else
	{
		glinep->next = glineHEAD;
		glineHEAD = glinep;
	}

	status->numGlines++;

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return glinep;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    findGline ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
Gline
*findGline (mask)
	char		*mask;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	Gline		*glinep;

	glinep = glineHEAD;
	while (glinep)
	{
		if (!strcasecmp (glinep->mask, mask))
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: G:Line [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__,
						glinep->mask, glinep->mask
					);
				}

				if (DEBUG_FUNC)
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}
			}

			return glinep;
		}
		glinep = glinep->next;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return glinep;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    delGline ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void
delGline (mask)
	char		*mask;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	Gline		*glinep;
	Gline		*prev;

	prev = NULL;

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		if (!strcasecmp (glinep->mask, mask))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Deleting G:Line [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__,
					glinep->mask, glinep->mask
				);
			}

			if (!prev)
			{
				glineHEAD = glinep->next;
			}
			else
			{
				prev->next = glinep->next;
			}

			ircsp_free (glinep->mask);
			ircsp_free (glinep->reason);
			ircsp_free (glinep->setby);
			ircsp_free (glinep);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
		prev = glinep;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_loadGlineDB ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_loadGlineDB (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	Gline		*glinep;
	FILE		*f;
	char		inBuf[256];
	char		*argv[9];

	f = fopen (config->gline_db, "r");
	if (!f)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to load G:Line database\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return FALSE;
	}
	else
	{
		while (!feof (f))
		{
			if (fgets (inBuf, 256, f))
			{
				if ((inBuf[0] == ';') || (inBuf[0] <= 32))
				{
					continue;
				}

				argv[0] = strtok(inBuf, ":");		/* Mask */
				argv[1] = strtok(NULL, ":");		/* Set When */
				argv[2] = strtok(NULL, ":");		/* Expires */
				argv[3] = strtok(NULL, ":");		/* Set by */
				argv[4] = strtok(NULL, "\n");		/* Reason */

				glinep = addGline (argv[0], atol(argv[1]), atol(argv[2]), argv[4], argv[3]);
			}
		}
	}

	fclose (f);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_saveGlineDB ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_saveGlineDB (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	FILE		*f;
	Gline		*glinep;

	if (!(f = fopen (config->gline_db, "w")))
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to write to G:Line database\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			if (DEBUG_FUNC)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return FALSE;
	}

	fprintf (f, "; GLINE DB file\n");
	fprintf (f, "; Last Saved: %lu\n", time (NULL));

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		fprintf (f,
			"%s:%lu:%lu:%s:%s\n",
			glinep->mask, glinep->setwhen, glinep->expires,
			glinep->setby, glinep->reason);
	}

	fclose (f);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    do_uworld ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void
do_uworld (sBuf)
	char		*sBuf;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t	*myuserp;
	nickname_t	*userp;
	char		*parse[5];
	char		*argv[256];
	char		*temp;
	int		c;
	int		argc = 1;
	int		cmd = 0;
	int		level = 0;

	parse[0]		= strtok(sBuf, " "); 		/* nickname */
	strtok(NULL, " "); 					/* skip */
	parse[1]		= strtok(NULL, " "); 		/* Dest nickname */
	temp			= strtok(NULL, "\r"); 		/* Command and args */
	parse[2]		= strtok(temp, " "); 		/* Command */
	parse[2]++;
	parse[3]		= strtok(NULL, "\r"); 		/* Args */

	myuserp= Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
		if (DEBUG)
		{
			if (DEBUG_SERVICE)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_NOSUCHNICK),
					__FILE__, __LINE__, __FUNCTION__, parse[1]
				);
			}

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return ;
	}

	userp = Nickname_Find (parse[0]);
	if (!userp)
	{
		if (DEBUG)
		{
			if (DEBUG_SERVICE)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_NOSUCHNICK),
					__FILE__, __LINE__, __FUNCTION__, parse[0]
				);
			}

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
		}

		return ;
	}

	for (c = 0; c <= UW_NUM_CMDS; c++)
	{
		if (!strcasecmp (parse[2], UWorldCommand[c].command))
		{
			cmd		= 1;
			argv[0]		= parse[0];		/* Nickname */
			argv[1]		= parse[1];		/* Dest Nickname */
			argv[2]		= parse[2];		/* Command */
			argc		= 3;

			if (parse[3])
			{
				argv[3]	= strtok (parse[3], " ");

				while (argv[argc])
				{
					argv[++argc] = strtok(NULL, " ");
				}
			}

			if (userp->flags & NICK_IS_ROOT)
			{
				level = 3;
			}
			else if (userp->flags & NICK_IS_ADMIN)
			{
				level = 2;
			}
			else if (userp->flags & NICK_IS_OPER)
			{
				level = 1;
			}
			else
			{
				level = 0;
			}

			if (level >= UWorldCommand[c].minlevel)
			{
				if (UWorldCommand[c].disabled)
				{
					if ((DEBUG) && (DEBUG_SERVICE))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: UWorld Command [%s] DISABLED\n",
							__FILE__, __LINE__, __FUNCTION__,
							UWorldCommand[c].command
						);
					}

					cmd_notice_operators (myuserp->nickname,
						"Denied access to DISABLED command \002%s\002 from \002%s\002",
						UWorldCommand[c].command, userp->nickname
					);

					cmd_notice (myuserp->nickname, userp->nickname,
						"Command is \002DISABLED\002"
					);


					if ((DEBUG) && (DEBUG_FUNC))
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return ;
				}

				if (argc < UWorldCommand[c].minargs)
				{
					cmd_notice (myuserp->nickname, userp->nickname,
						"Incorrect Syntax. try /msg %s HELP %s",
						config->uworld_nickname, UWorldCommand[c].command
					);


					if ((DEBUG) && (DEBUG_FUNC))
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return ;
				}

				if (UWorldCommand[c].func (argc, argv))
				{
					if ((DEBUG) && (DEBUG_SERVICE))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Pushing UWorld command [%s] for user [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, userp->nickname
						);
					}
				}
			}

			else
			{
				cmd_notice_operators (myuserp->nickname,
					"Denied access to \002%s\002 from \002%s\002",
					UWorldCommand[c].command, userp->nickname
				);

				cmd_notice (myuserp->nickname, userp->nickname,
					"\002%s\002",
					config->deny_message
				);


				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return ;
			}
		}
	}

	if (parse[2][0] == '\001')
	{
		parse[2]++;
		if (!strncasecmp (parse[2], "VERSION", 7))
		{
			cmd_notice (myuserp->nickname, userp->nickname,
				"\001VERSION IRCSP %s - Copyright (C) 1998-2009 GOTUNIX Networks. \001",
				version_number
			);


			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}

	if (!cmd)
	{
		cmd_notice (myuserp->nickname, userp->nickname,
			"Unknown command [\002%s\002]",
			parse[2]
		);


		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return ;
	}


	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_version ()
                   |
    DESCRIPTION    |    This function will return the current service and ircsp version back to the sender.
                   |
                   |    argv[0] = Source Nickname/Numeric
                   |    argv[1] = Service Nickname
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_version (argc, argv)
	int		argc;
	char		*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	int			i;

	src_nickname_p = Nickname_Find (argv[0]);
	dst_nickname_p = Nickname_Find (argv[1]);


	/* We are going to assume that these will never be NULL */
	assert (src_nickname_p != NULL);
	assert (dst_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);
	assert (dst_nickname_p->nickname != NULL);

	cmd_notice_operators (dst_nickname_p->nickname,
		"UWorld command \002VERSION\002 issued by \002%s\002",
		src_nickname_p->nickname
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"---------------------------------------------------"
	);

	for (i = 0; info_text[i]; i++)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"%s",
			info_text[i]
		);
	}

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"---------------------------------------------------"
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"IRCSP version information"
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"- Codename: \002%s\002",
		code_name
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"-  Version: \002%s\002",
		version_number
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"-    BUILD: \002%s\002",
		version_build
	);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_help ()
                   |
    DESCRIPTION    |    This command will display the help file for UWorld as well as any help on a specified topic
                   |
                   |    argv[0] = Sender Numeric
                   |    argv[3] = (optional) help topic
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_help (argc, argv)
	int 		argc;
	char 		*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname_src;
	nickname_t		*p_nickname_dst;
	FILE 			*fp;
	char			buf[256];
	char			newbuf[512];
	char			helpfile[128];
	int 			level;
	int			show = 1;

	p_nickname_dst = Nickname_Find (config->uworld_nickname);
	p_nickname_src = Nickname_Find (argv[0]);

	/* These should never come back NULL */

	assert (p_nickname_dst != NULL);
	assert (p_nickname_src != NULL);
	assert (p_nickname_src->nickname != NULL);
	assert (p_nickname_dst->nickname != NULL);

	cmd_notice_operators (p_nickname_dst->nickname,
		"UWorld command \002HELP\002 issued by \002%s\002",
		p_nickname_src->nickname
	);

	if (argc == 3)
	{
		sprintf (helpfile, "%s/helpfiles/uworld/index", VARPATH);
	}
	else
	{
		sprintf (helpfile, "%s/helpfiles/uworld/%s", VARPATH, argv[3]);
	}

	fp = fopen (helpfile, "r");
	if (!fp)
	{
		cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname,
			"No help available on \002%s\002",
			argv[3]);


		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	while (!feof (fp))
	{
		if (fgets (buf, 255, fp))
		{
			if (buf[0] == '@')
			{
				buf[0] = '0';
				level = atoi(buf);

				if ( (p_nickname_src->flags & NICK_IS_ROOT) && (p_nickname_src->flags & NICK_IS_OPER) && (level <= 3))
				{
					show = 1;
				}

				else if ( (p_nickname_src->flags & NICK_IS_ADMIN) && (p_nickname_src->flags & NICK_IS_OPER) && (level <= 2))
				{
					show = 1;
				}

				else if ( (p_nickname_src->flags & NICK_IS_OPER) && (level <= 1))
				{
					show = 1;
				}

				else if ( (!(p_nickname_src->flags & NICK_IS_OPER)) && (level == 0))
				{
					show = 1;
				}

				else
				{
					show = 0;
				}

				continue;
			}


			if (show)
			{
				cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname, "%s", translate(buf, newbuf));
			}
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_operlist ()
                   |
    DESCRIPTION    |    This command will list all GLOBAL operators on the network
                   |
                   |    argv[0] = Source
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_operlist (argc, argv)
	int		argc;
	char		*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*p_nickname_src;
	nickname_t		*p_nickname_dst;
	operator_t		*p_operator;
	channel_t		*p_channel;

	p_nickname_src = Nickname_Find (argv[0]);
	p_nickname_dst = Nickname_Find (config->uworld_nickname);

	assert (p_nickname_src != NULL);
	assert (p_nickname_dst != NULL);
	assert (p_nickname_src->nickname != NULL);
	assert (p_nickname_dst->nickname != NULL);

	cmd_notice_operators (p_nickname_dst->nickname,
		"UWorld command \002OPERLIST\002 issued by \002%s\002",
		p_nickname_src->nickname
	);

	cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname,
		"\002GLOBAL OPERATOR LIST\002"
	);

	for (p_operator = operator_h; p_operator; p_operator = p_operator->next)
	{
		cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname,
			"[\002%s\002] is an IRC Operator",
			p_operator->nickname_p->nickname
		);
	}

	cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname,
		"\002END OF OPERATOR LIST\002");

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_verify ()
                   |
    DESCRIPTION    |    This function will allow users to verify that someone is infact a UWorld Administrator
                   |
                   |    argv[0] = Source
                   |    argv[3] = Nickname to verify
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_verify (argc, argv)
	int		argc;
	char		*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*myuserp;	/* UWorld Nickname */
	nickname_t		*userp;		/* Sender Nickname */
	nickname_t		*vuserp;	/* Nickname to Verify */

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL);


	cmd_notice_operators (myuserp->nickname,
		"UWorld command \002VERIFY\002 issued by \002%s\002 on \002%s\002",
		userp->nickname, argv[3]
	);


	vuserp = Nickname_Find (argv[3]);
	if (!vuserp)
	{
		cmd_notice (myuserp->nickname, userp->nickname,
			"\002%s\002 was not found on this network",
			argv[3]
		);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	else
	{
		if (vuserp->flags & NICK_IS_ADMIN)
		{
			cmd_notice (myuserp->nickname, userp->nickname,
				"\002%s\002 is an authorized services administrator",
				vuserp->nickname
			);


			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return TRUE;
		}
		else
		{
			cmd_notice (myuserp->nickname, userp->nickname,
				"\002%s\002 is not an authorized services administrator",
				argv[3]
			);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return FALSE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    uw_servlist ()
                   |
    DESCRIPTION    |    This function will return a list of servers that are currently connected to the network as well as their
                   |    uplink information back to the sender.
                   |
                   |    argv[0] = Sender
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int
uw_servlist (argc, argv)
	int		argc;
	char		*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*userp;
	nickname_t		*myuserp;
	server_t		*serverp;
	int			numServers = 0;

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL);

	ssprintf(s,
		"%s O %s :Global \002SERVER\002 list\n",
		myuserp->numeric, argv[0]);

	for (serverp = server_h; serverp; serverp = serverp->next)
	{

		ssprintf(s,
			"%s O %s :Server [\002%s\002] Numeric: [\002%s\002]\n",
			myuserp->numeric, argv[0], serverp->name, serverp->numeric);

		if (serverp->flags & SERVER_IS_MY_UPLINK)
		{
			// Server is IRCSP's uplink.

			ssprintf(s,
				"%s O %s :|-  IRCSP UPLINK\n",
				myuserp->numeric, argv[0]);
		}
		else
		{
			if (serverp->my_uplink_p != NULL)
			{
				ssprintf (s,
					"%s O %s :|- UPLINK [\002%s\002]\n",
					myuserp->numeric, argv[0], serverp->my_uplink_p->name);
			}
		}

		ssprintf(s,
			"%s O %s :|-  Current Users [\002%d\002]\n",
			myuserp->numeric, argv[0], serverp->numUsers);

		ssprintf(s,
			"%s O %s :|-  Current Operators [\002%d\002]\n",
			myuserp->numeric, argv[0], serverp->numOpers);


		numServers++;
	}

	ssprintf(s,
		"%s O %s :[\002%d\002] Servers listed.\n",
		myuserp->numeric, argv[0], numServers);


	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_nicklist ()
                |
 DESCRIPTION    |    This function will display a list of all nicknames currently online as well as their status
                |    (oper, service, admin, etc)
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_nicklist (argc, argv)
	int argc;
	char *argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t *myuserp;
	nickname_t *userp;
	server_t *serverp;
	int numNicks = 0, numOpers = 0, numService = 0, numJupes = 0;

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL);

	ssprintf(s,
		"%s O %s :Global \002NICK\002 list\n",
		myuserp->numeric, argv[0]);

	for (userp = nickname_h; userp; userp = userp->next)
	{
		serverp = userp->server_p;
		assert (serverp != NULL);
		assert (serverp->name != NULL);

		if (userp->flags & NICK_IS_SERVICE)
		{
			ssprintf(s,
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002SERVICE BOT\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostname_p->hostname, serverp->name);

			numService++;
		}


		else if ( (userp->flags & NICK_IS_OPER) && (!(userp->flags & NICK_IS_SERVICE)) )
		{
			ssprintf(s,
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002OPER\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostname_p->hostname, serverp->name);
			numOpers++;
		}
		else if (userp->flags & NICK_IS_JUPE)
		{
			ssprintf(s,
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002JUPE\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostname_p->hostname, serverp->name);
			numJupes++;
		}

		else if ( (!(userp->flags & NICK_IS_OPER)) && (!(userp->flags & NICK_IS_SERVICE)) )
		{
			ssprintf(s,
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostname_p->hostname, serverp->name);
		}
		numNicks++;
	}

	ssprintf(s,
		"%s O %s :[\002%d\002] Nicknames [\002%d\002] Operators  and [\002%d\002] Service Bots and [\002%d\002] Nick Jupes listed\n",
		myuserp->numeric, argv[0], numNicks, numOpers, numService, numJupes);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_uptime ()
                |
 DESCRIPTION    |    Display the current uptime of IRCSP
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_uptime (argc, argv)
	int argc;
	char *argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t *userp;
	nickname_t *myuserp;

	int days, hours, mins, secs;
	time_t curtime, uptime;

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL);

	curtime = time(NULL);
	uptime = curtime - ircsp->uptime;

	days = uptime/86400;
	hours = (uptime/3600)%24;
	mins = (uptime/60)%60;
	secs = uptime%60;

	ssprintf(s,
		"%s O %s :\002%s\002 has been up for \002%d\002 days \002%d\002 hours \002%d\002 minutes \002%d\002 seconds\n",
		myuserp->numeric, argv[0], config->server_name, days, hours, mins,
		secs);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_hostlist ()
                |
 DESCRIPTION    |    Display all HOSTNAMES currently online, including a count of how many clients are connected using each host
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_hostlist (argc, argv)
	int argc;
	char *argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t *userp;
	nickname_t *myuserp;
	hostname_t *hostp;
	int numHosts = 0;

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL)

	ssprintf(s,
		"%s O %s :Global \002HOST\002 list\n",
		myuserp->numeric, argv[0]);

	for (hostp = hostname_h; hostp; hostp = hostp->next)
	{
		ssprintf(s,
			"%s O %s :Hostname [\002%s\002]  Host Count [\002%d\002]\n",
			myuserp->numeric, argv[0], hostp->hostname, hostp->numHosts);
		numHosts++;
	}

	ssprintf(s,
		"%s O %s :[\002%d\002] Hostnames listed.\n",
		myuserp->numeric, argv[0], numHosts);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_adminlist ()
                |
 DESCRIPTION    |    This function will display a list of all administrators and their status (logged in, logged in from where,
                |    level, hostmask, etc)
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_adminlist (argc, argv)
	int argc;
	char *argv[];
{
	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t *userp;
	nickname_t *myuserp;
	int numAdmin = 0;

	userp = Nickname_Find (argv[0]);
	myuserp = Nickname_Find (config->uworld_nickname);

	assert (userp != NULL);
	assert (userp->nickname != NULL);
	assert (myuserp != NULL);
	assert (myuserp->nickname != NULL);

	ssprintf(s,
		"%s O %s :IRCSP Administrator LIST\n",
		myuserp->numeric, argv[0]);

/*	for (adminp = admin_list_head; adminp; adminp = adminp->next)
	{
		ssprintf(s,
			"%s O %s :[\002%s\002]\n",
			myuserp->numeric, argv[0], adminp->nickname);

		ssprintf(s,
			"%s O %s :|- Level   :  %d\n",
			myuserp->numeric, argv[0], adminp->level);

		ssprintf(s,
			"%s O %s :|- Hostmask:  %s\n",
			myuserp->numeric, argv[0], adminp->hostmask);

		if (adminp->userp)
			ssprintf(s,
				"%s O %s :|- \002IN-USE\002 %s (%s@%s)\n",
				myuserp->numeric, argv[0], adminp->userp->nickname,
				adminp->userp->username, adminp->userp->p_Hostname->hostname);

		if (IsAdminSuspended(adminp))
		{
			ssprintf(s,
				"%s O %s :|- %s is Suspended!\n",
				myuserp->numeric, argv[0], adminp->nickname);
		}

		numAdmin++;
	} */

	ssprintf(s,
		"%s O %s :[\002%d\002] Admins listed\n",
		myuserp->numeric, argv[0], numAdmin);

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_auth ()
                |
 DESCRIPTION    |    This function is used to authenitcate yourself with UWorld, this will be changed in upcoming versions to use
                |    NickServ.
                |
                |    argv[0] = Sender
                |    argv[3] = Account
                |    argv[4] = Password
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_auth (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
//	Admin_List *adminp;
	char userhost[2096];
	int hostMatch = 0;



	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	userp = Nickname_Find(argv[0]);
	if (!userp)
	{

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	} /*

printf ("we made it here [1]\n");

	if (IsAdministrator(userp))
	{
		ssprintf(s,
			"%s O %s :You are already authorized.\n",
			myuserp->numeric, argv[0]);
		return 0;
	}

printf ("we made it here [2]\n");
	adminp = ircsp_admin_find (argv[3]);
	if (!adminp)
	{
		ssprintf(s,
			"%s O %s :Authentication failed! [1]\n",
			myuserp->numeric, argv[0]);


		return 0;
	}
printf ("we made it here [3]\n");

	if (IsAdminLogin(adminp))
	{
		ssprintf(s,
			"%s O %s :Authentication failed! (Account already in use) [\002%s\002]\n",
			myuserp->numeric, argv[0], adminp->userp->nickname);


		return 0;
	}
printf ("we made it here [4]\n");

	sprintf(userhost, "%s@%s", userp->username, userp->p_Hostname->hostname);

	if (!match(adminp->hostmask, userhost))
	{
		hostMatch = 1;
	}
printf ("we made it here [5]\n");
	if (!hostMatch)
	{
		ssprintf(s,
			"%s O %s :Authentication failed! [2]\n",
			myuserp->numeric, argv[0]);


		return 0;
	}
printf ("we made it here [6]\n");
	if (!strcmp(adminp->password, argv[4]))
	{
		ssprintf(s,
			"%s O %s :Authentication successful!\n",
			myuserp->numeric, argv[0]);

		if (IsAdminSuspended(adminp))
		{
			ssprintf(s,
				"%s O %s :... But you are suspended!\n",
				myuserp->numeric, argv[0]);


			return 0;
		}
printf ("we made it here [7]\n");
		SetAdminLogin(adminp);
		SetAdministrator(userp);

		userp->adminp = adminp;
		adminp->userp = userp;


		return 1;
	}
	else
	{
		ssprintf(s,
			"%s O %s :Authentication failed! [3]\n",
			myuserp->numeric, argv[0]);


		return 0;
	} */


	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_deauth ()
                |
 DESCRIPTION    |    This function is used to deauthenticate with uworld, again this will be moved to nickserv in a later release
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_deauth (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
/*	Admin_List *adminp;


	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	userp = Nickname_Find(argv[0]);
	if (!userp)
	{
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	if (IsAdministrator(userp))
	{
		adminp = ircsp_admin_find (userp->adminp->nickname);
		if (!adminp)
		{
			ssprintf(s,
				"%s O %s :You're not logged in.\n",
				myuserp->numeric, argv[0]);

			return 0;
		}

		if (IsAdminLogin(adminp))
		{
			DelAdminLogin(adminp);
			DelAdministrator(userp);
			userp->adminp = NULL;
			adminp->userp = NULL;

			ssprintf(s,
				"%s O %s :Deauthorization successfull!\n",
				myuserp->numeric, argv[0]);

			return 1;
		}
		else
		{
			ssprintf(s,
				"%s O %s :You're not logged in.\n",
				myuserp->numeric, argv[0]);
			return 0;
		}
	}
	else
	{
		ssprintf(s,
			"%s O %s :You're not logged in.\n",
			myuserp->numeric, argv[0]);
		return 0;
	} */

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_glinelist ()
                |
 DESCRIPTION    |    This function will send a current list of global glines back to the sender, this will be changed in future
                |    releases to use GLINE, AKILL, etc. Depending ont he protocol being used.
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_glinelist (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	Gline *glinep;
	int numGlines = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command GLINELIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
///			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ssprintf(s,
		"%s O %s :Global \002GLINE\002 list\n",
		myuserp->numeric, argv[0]);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		ssprintf(s,
			"%s O %s :Gline [%s]\n",
			myuserp->numeric, argv[0], glinep->mask);
		ssprintf(s,
			"%s O %s :|- Added on  : [%s]\n",
			myuserp->numeric, argv[0], chop(ctime(&glinep->setwhen)));
		ssprintf(s,
			"%s O %s :|- Expires on: [%s]\n",
			myuserp->numeric, argv[0], chop(ctime(&glinep->expires)));
		ssprintf(s,
			"%s O %s :|- Set By    : [%s]\n",
			myuserp->numeric, argv[0], glinep->setby);
		ssprintf(s,
			"%s O %s :|- Reason    : [%s]\n",
			myuserp->numeric, argv[0], glinep->reason);
		numGlines++;
	}

	ssprintf(s,
		"%s O %s :[\002%d\002] GLINES listed.\n",
		myuserp->numeric, argv[0], numGlines);

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command GLINELIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_chanlist ()
                |
 DESCRIPTION    |    This function will send a complete list of channels on the network back to the sender.
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_chanlist (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	channel_t *channelp;
	int numChannels = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command CHANLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
		// Failed to locate SENDER nickname in our linked list.

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in our linked list.

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002CHANLIST\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :Global \002CHANNEL\002 list\n",
		myuserp->numeric, argv[0]);

	for (channelp = channel_h; channelp; channelp = channelp->next)
	{
		ssprintf(s,
			"%s O %s :Channel [%s]\n",
			myuserp->numeric, argv[0], channelp->name);
		numChannels++;
	}

	ssprintf(s,
		"%s O %s :[\002%d\002] CHANNELS listed.\n",
		myuserp->numeric, argv[0], numChannels);


//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command CHANLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_gline ()
                |
 DESCRIPTION    |    This funtion will add a gline to our database and globally to the network.
                |
                |    argv[0] = Sender
                |    argv[3] = Mask
                |    argv[4] = Duration
                |    argv[5] .. argv[parc] = Reason
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_gline (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	Gline *glinep;
	int duration, curarg = 3, is_bad_gline = 0, numdots, curdot;
	int eduration;
	char reason[ 512 ], *maskp, *uidp, mask[ 256 ];


//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command GLINE\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in linked list

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in linked list

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	strcpy(mask, argv[3]);

	/* First, we need to check to see if this G:Line is a *bit*
	   excessive.  The following are considered Bad:
	
	   1) Bans that are equivalent to *@*
	   2) Bans of entire TLD's */


	/* First lets check for *@* */

	if (!match(argv[3], "*@*"))
	{
		is_bad_gline = 1;
	}

	/* Next we'll check for TLD bans. EVERY character up to the
	   next to last token (by period) must be ? or * and the
	   last must not be (it should be caught by above anyway). */


	if (!is_bad_gline)
	{
		uidp = strtok(mask, "@");
		if (!match(uidp, "*"))      /* userid = * or equivalent */
		{
			maskp = strtok(NULL, " ");
			numdots = tokenize(maskp, '.');

			if (!numdots)
			{
				is_bad_gline = 1;
			}
			else
			{
				is_bad_gline = 1;
				for(curdot = 1; curdot < numdots; curdot++)
					if(match(token(maskp, curdot), "*") != 0)
						is_bad_gline = 0;
			}
		}
	}

	if (is_bad_gline)
	{
		ssprintf(s,
			"%s O %s :Excessive GLINE - please narrow the mask.\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	if (isdigit(argv[4][0]))
	{
		duration = atoi(argv[4]);
		if (argc < 6)
		{
			ssprintf(s,
				"%s O %s :\002GLINE\002 requires a REASON.\n",
				myuserp->numeric, userp->numeric);
			return 0;
		}

		curarg = 5;
	}
	else
	{
//		duration = config->gline_auto_len;
		duration = 3600;
		curarg = 4;
	}

	eduration = duration * 8;

	glinep = glineHEAD;
	while (glinep)
	{
		/* Do we already have an exact gline for this host? */

		if (!strcasecmp(glinep->mask, argv[3]))
		{
			/* Gline exists - check the expiration versus our new
			   time -- if it's greater, update the expiration
			   else ignore it. */

			if ((time(NULL) + duration) > glinep->expires)
			{
				glinep->expires += eduration;
				glinep->setwhen = time(NULL);

				ssprintf(s,
					"%s O %s :\002GLINE\002 Expiration reset.\n",
					myuserp->numeric, userp->numeric);

				ssprintf(s,
					"%s GL * +%s %lu %lu :%s\n",
					config->server_numeric, glinep->mask,
					eduration, time(NULL), glinep->reason);

				ssprintf(s,
					"%s WA :[%s] reset \002GLINE\002 Expiration on [%s]\n",
					config->server_numeric, userp->nickname, glinep->mask);
				return 1;
			}

			ssprintf(s,
				"%s O %s :\002GLINE\002 for [\002%s\002] already exists.\n",
				myuserp->numeric, userp->numeric, glinep->mask);
			return 0;
		}

		else if (!match(glinep->mask, argv[3]))
		{
			ssprintf(s,
				"%s O %s :\002GLINE\002 covering [\002%s\002] already exists [\002%\002]\n",
				myuserp->numeric, userp->numeric, argv[3], glinep->mask);
			return 1;
		}

		else if (!match(argv[3], glinep->mask))
		{
			ssprintf(s,
				"%s O %s :Removing lower precedence \002GLINE\002 for [\002%s\002]\n",
				myuserp->numeric, userp->numeric, glinep->mask);

			ssprintf(s,
				"%s GL * -%s %lu %lu\n",
				config->server_numeric, glinep->mask, (time(NULL) - 60),
				time(NULL));
			delGline(glinep->mask);
		}
		glinep = glinep->next;
	}

	/* Create our reason */

	strcpy(reason, argv[curarg]);

	argc--;
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002GLINE\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	glinep = addGline (argv[3], time(NULL), time(NULL) + duration, reason, userp->nickname);
	ssprintf(s,
		"%s GL * +%s %lu %lu :(Expires: %s (%s))\n",
		config->server_numeric, argv[3], time(NULL) + eduration,
		time(NULL), chop(ctime(&glinep->expires)), reason);

	ssprintf(s,
		"%s WA :[\002%s\002] Adding \002GLINE\002 for %s (Expires: %s (%s))\n",
		config->server_numeric, userp->nickname, argv[3],
		chop(ctime(&glinep->expires)), reason);

	ssprintf(s,
		"%s O %s :Adding \002GLINE\002 for %s (Expires %s (%s))\n",
		myuserp->numeric, argv[0], glinep->mask,
		chop(ctime(&glinep->expires)), reason);


//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command GLINE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_remgline ()
                |
 DESCRIPTION    |    This function will remove a gline from our database and from the network
                |
                |    argv[0] = Sender
                |    argv[3] = Hostmask
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_remgline (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	Gline *glinep;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command REMGLINE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
		/* Failed to find SENDER nickname in our linked list */

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@()# - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
		/* Failed to find UWORLD nickname in our linked list */

//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	glinep = findGline (argv[3]);
	if (!glinep)
	{
		ssprintf(s,
			"%s O %s :No such \002GLINE\002\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002REMGLINE\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s GL * -%s %lu %lu\n",
		config->server_numeric, glinep->mask, (time(NULL) - 172800),
		time(NULL));

	ssprintf(s,
		"%s O %s :\002GLINE\002 [%s] Removed.\n",
		myuserp->numeric, userp->numeric, glinep->mask);

	ssprintf(s,
		"%s WA :[%s] has removed \002GLINE\002 for %s\n",
		config->server_numeric, userp->nickname, glinep->mask);

	delGline (glinep->mask);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_whoison ()
                |
 DESCRIPTION    |    This function will list all users and their status (chanop, chan voice) on a specified channel.
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_whoison (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	channel_t *channelp;
	channel_nickname_t *nicklistp;

	int numUsers = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command WHOISON\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find(argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
		ssprintf(s,
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002WHOISON\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :\002WHOISON\002 Channel %s\n",
		myuserp->numeric, argv[0], argv[3]);


	nicklistp = channelp->channel_nickname_h;
	while (nicklistp)
	{
		ssprintf(s,
			"%s O %s :[%s] is on [%s]\n",
			myuserp->numeric, argv[0], nicklistp->nickname_p->nickname,
			channelp->name);

		if (nicklistp->flags & IS_CHANOP)
			ssprintf(s,
				"%s O %s :|- IS A CHANNEL OP\n",
				myuserp->numeric, argv[0]);

		if (nicklistp->flags & IS_CHANVOICE)
			ssprintf(s,
				"%s O %s :|- IS A CHANNEL VOICE\n",
				myuserp->numeric, argv[0]);

		nicklistp = nicklistp->next;
	}

	ssprintf(s,
		"%s O %s :End of WHOISON LIST\n",
		myuserp->numeric, argv[0]);


//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command WHOISON\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_banlist ()
                |
 DESCRIPTION    |    This command will display a list of BANS on a specified channel
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_banlist (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	channel_t *channelp;
	channel_ban_t	*banp;

	int numBans = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command BANLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
		ssprintf(s,
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002BANLIST\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :\002BANLIST\002 Channel %s\n",
		myuserp->numeric, argv[0], argv[3]);

	banp = channelp->channel_ban_h;
	while (banp)
	{
		if (!strcasecmp(channelp->name, argv[3]))
		{
			ssprintf(s,
				"%s O %s :\002%s\002\n",
				myuserp->numeric, argv[0], banp->mask);

			numBans += 1;
		}
		banp = banp->next;
	}
	ssprintf (s, "%s O %s :[%d] Bans\n",
		myuserp->numeric, argv[0]);

	ssprintf(s,
		"%s O %s :End of BANLIST LIST\n",
		myuserp->numeric, argv[0]);

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command BANLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_xlate ()
                |
 DESCRIPTION    |    This function will lookup a specified numeric and return the nickname attached to it.
                |
                |    argv[0] = Sender
                |    argv[3] = Numeric to lookup
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_xlate (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;	/* UWorld nickname */
	nickname_t *userp;	/* Sender nickname */
	nickname_t *xuserp;	/* xlate nickname */

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command XLATE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002XLATE\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s WA :\002XLATE\002 - [%s] used me to lookup %s\n",
		config->server_numeric, userp->nickname, argv[3]);

	xuserp = Nickname_Find (argv[3]);
	if (!xuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find XLATE nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf(s,
			"%s O %s :No such numeric\n",
			myuserp->numeric, userp->numeric);

		return 0;
	}
	else
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  XLATE nickname [%s] found\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf(s,
			"%s O %s :Numeric \002%s\002 belongs to nickname \002%s\002.\n",
			myuserp->numeric, userp->numeric, xuserp->numeric, xuserp->nickname);

		return 1;
	}

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command XLATE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_whois ()
                |
 DESCRIPTION    |    This function will return extended whois information on the speified user.
                |
                |    argv[0] = Sender
                |    argv[3] = nickname to loopup
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_whois (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;		/* UWorld nickname */
	nickname_t *userp;		/* Sender nickname */
	nickname_t *wuserp;		/* Whois nickname */
	server_t *serverp;		/* Whois nickname's server */
	nickname_channel_t *chanlistp;	/* Whois nickname's channel list */

	char channels[512];
	int chanlen, chanlistlen = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command WHOIS\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002WHOIS\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	wuserp = Nickname_Find (argv[3]);
	if (!wuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  No such nickname\n",
//			__FILE__, __LINE__, __FUNCTION__);

		ssprintf(s,
			"%s O %s :No such nickname \002%s\002\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

	serverp = wuserp->server_p;
//	serverp = Server_Find (wuserp->server);
	if (!serverp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find USERS server\n",
//			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	ssprintf(s,
		"%s O %s :Whois information on \002%s\002\n",
		myuserp->numeric, userp->numeric, wuserp->nickname);
	ssprintf(s,
		"%s O %s :User: %s (%s@%s)\n",
		myuserp->numeric, userp->numeric, wuserp->nickname,
		wuserp->username, wuserp->hostname_p->hostname);
	ssprintf(s,
		"%s O %s :Numeric: %s\n",
		myuserp->numeric, userp->numeric, wuserp->numeric);
	ssprintf(s,
		"%s O %s :Server: %s\n",
		myuserp->numeric, userp->numeric, serverp->name);

/*	if (wuserp->adminp)
	{
		if (IsAdminLogin(wuserp->adminp))
			ssprintf(s,
				"%s O %s :%s is a level \002%d\002 %s administrator\n",
				myuserp->numeric, userp->numeric,
				wuserp->nickname, wuserp->adminp->level,
				myuserp->nickname);
	} */

	if (wuserp->flags & NICK_IS_OPER)
	{
		ssprintf(s,
			"%s O %s :%s is an IRC Operator\n",
			myuserp->numeric, userp->numeric, wuserp->nickname);
	}

	if (wuserp->flags & NICK_IS_SERVICE)
	{
		ssprintf (s,
			"%s O %s :%s is a IRC Service\n",
			myuserp->numeric, userp->numeric,
			wuserp->nickname);
	}

	if (!(wuserp->flags & NICK_IS_SERVICE))
	{
		chanlistp = wuserp->channel_h;
		if (chanlistp)
		{
			chanlistlen += strlen(chanlistp->channel_p->name);
			if (chanlistlen < 512)
				strcpy(channels, chanlistp->channel_p->name);

			chanlistp = chanlistp->next;
			while (chanlistp)
			{
				strcat(channels, " ");
				if (chanlistp->flags & IS_CHANOP)
				{
					strcat(channels, "@");
					chanlistlen++;
				}
				else if (chanlistp->flags & IS_CHANVOICE)
				{
					strcat(channels, "+");
					chanlistlen++;
				}

				chanlistlen += strlen(chanlistp->channel_p->name) + 1;
				if (chanlistlen < 512)
				{
					strcat(channels, chanlistp->channel_p->name);
					chanlistp = chanlistp->next;
				}
				else
				{
					ssprintf(s,
						"%s O %s :Channels: %s\n",
						myuserp->numeric, userp->numeric,
						channels);
					chanlistlen = 0;
					channels[0] = '\0';
				}
			}
			ssprintf(s,
				"%s O %s :Channels: %s\n",
				myuserp->numeric, userp->numeric,
				channels);
		}
	}



//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command WHOIS\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_clearbans ()
                |
 DESCRIPTION    |    This command will remove all bans on a specified channel.
                |
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_clearbans (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	channel_t *channelp;
	channel_ban_t *banp;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command CLEARBANS\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
		ssprintf(s,
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002CLEARBANS\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	banp = channelp->channel_ban_h;
	while (banp)
	{
		ssprintf(s,
			"%s M %s -b %s\n",
			config->server_numeric, channelp->name, banp->mask);
		ChannelBan_Del (channelp, banp->mask);
		banp = banp->next;
	}

	ssprintf(s,
		"%s WA :[%s] used me to \002CLEARBANS\002 on %s\n",
		config->server_numeric, userp->nickname, channelp->name);

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command CLEARBANS\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_opermsg ()
                |
 DESCRIPTION    |    This function will send a NOTICE out to all OPERATORS.  We do need to change how this works, Right now we
                |    search our NICKNAME list and any NICK that has the OPER flag set on it gets returned.  This works fine on
                |    smaller networks, but when you have thousands of users this could be slow.  What i'm thinking is instead
                |    of adding a flag to the user when they /oper, we add an entry into a OPER list and link the nickname from
                |    the OPER list back to the NICK list so for example:
                |
                |        struct _operators {
                |            struct _nick *nickp;
                |        } Operators;
                |
                |    When a user OPERS in a add operator function we just (operator->nickp = nickp;)
                |
                |    I will need to do some more thinking about this first. but i think this is going to be the fastest.
                |
                |
                |    argv[0] = Sender
                |    argv[3...parc] = Message
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_opermsg (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;	/* UWorld Nickname */
	nickname_t *userp;	/* Sender Nickname */
	nickname_t *ouserp;	/* Oper Nickname */

	char message[512];
	int curarg = 3;
	argc--;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command OPERMSG\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	strcpy(message, argv[3]);
	while (++curarg <= argc)
	{
		strcat(message, " ");
		strcat(message, argv[curarg]);
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002OPERMSG\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s WA :[%s] just issued a \002OPERMSG\002\n",
		config->server_numeric, userp->nickname);

	for (ouserp = nickname_h; ouserp; ouserp = ouserp->next)
	{
		if (ouserp->flags & NICK_IS_OPER)
		{
			ssprintf(s,
				"%s O %s :[\002OPERMSG\002] (%s) %s\n",
				myuserp->numeric, ouserp->numeric,
				userp->nickname, message);
		}
	}

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command OPERMSG\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;

}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_clearops ()
                |
 DESCRIPTION    |    This function will clear all ops on the specified channel
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_clearops (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;		/* Uworld nickname */
	nickname_t *userp;		/* Sender nickname */
	channel_t *channelp;		/* Channel */
	channel_nickname_t *nicklistp;	/* Users on channel */

	int numDeops = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command CLEAROPS\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find requested channel [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist\n",
			myuserp->numeric, userp->numeric, argv[3]);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002CLEAROPS\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	nicklistp = channelp->channel_nickname_h;
	while (nicklistp)
	{
		if ((nicklistp->flags & IS_CHANOP) && (!(nicklistp->nickname_p->flags & NICK_IS_SERVICE)) )
		{
			ssprintf(s,
				"%s M %s -o %s\n",
				config->server_numeric, channelp->name,
				nicklistp->nickname_p->numeric);

			nicklistp->flags &= ~IS_CHANOP;
		}

		if (nicklistp->flags & IS_CHANVOICE);
		{
			ssprintf (s,
				"%s M %s -v %s\n",
				config->server_numeric, channelp->name,
				nicklistp->nickname_p->numeric);

			nicklistp->flags &= ~IS_CHANVOICE;
		}

		nicklistp = nicklistp->next;
	}

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command CLEAROPS\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_opcom ()
                |
 DESCRIPTION    |    This function will oper/admins to op/deop voice/devoice any user on any channel
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |    argv[4] = Modes
                |    argv[5.parc] = Users
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_opcom (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;		/* UWorld nickname */
	nickname_t *userp;		/* Sender nickname */
	nickname_t *muserp;		/* User modes were set on */
	channel_t *channelp;		/* Channel */
	nickname_channel_t *chanlistp;
	channel_nickname_t *nicklistp;

	char nicks[512], numerics[512];
	int pos = -1, addModes = 0, curarg = 4, numSet = 0;
	argc--;

	strcpy(nicks, " ");
	strcpy(numerics, " ");

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command OPCOM\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find requested channel [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist\n",
			myuserp->numeric, userp->numeric, argv[3]);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002OPCOM\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	while (argv[4][++pos])
	{
		if (argv[4][pos] == '+')
		{
			addModes = 1;
			continue;
		}

		if (argv[4][pos] == '-')
		{
			addModes = 0;
			continue;
		}

		if (argv[4][pos] == 'o')
		{
			++curarg;

			if (curarg > argc)
			{
				goto dumpEarly;
			}


			muserp = Nickname_Find(argv[curarg]);
			if (!muserp)
				continue;

			chanlistp = NicknameChannel_Find (muserp, channelp);
			if (!chanlistp)
				continue;

			nicklistp = ChannelNickname_Find (channelp, muserp);
			if (!nicklistp)
				return 0;

			if (addModes)
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				nicklistp->flags |= IS_CHANOP;
				chanlistp->flags |= IS_CHANOP;
			}
			else
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				nicklistp->flags &= ~IS_CHANOP;
				chanlistp->flags &= ~IS_CHANOP;
			}
		}

		if (argv[4][pos] == 'v')
		{
			++curarg;

			if (curarg > argc)
			{
				goto dumpEarly;
			}


			muserp = Nickname_Find(argv[curarg]);
			if (!muserp)
				continue;

			chanlistp = NicknameChannel_Find (muserp, channelp);
			if (!chanlistp)
				continue;

			nicklistp = ChannelNickname_Find (channelp, muserp);
			if (!nicklistp)
				return 0;

			if (addModes)
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");


				nicklistp->flags |= IS_CHANVOICE;
				chanlistp->flags |= IS_CHANVOICE;
			}
			else
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				nicklistp->flags &= ~IS_CHANVOICE;
				chanlistp->flags &= ~IS_CHANVOICE;
			}
		}

	}

	ssprintf(s,
		"%s WA :%s is using me to: MODE %s %s%s\n",
		config->server_numeric, userp->nickname, argv[3], argv[4], nicks);

dumpEarly:
	ssprintf (s,
		"%s M %s %s %s\n",
		config->server_numeric, argv[3], argv[4], numerics);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_clearmodes ()
                |
 DESCRIPTION    |    This function will clear ALL modes on the specified channel
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_clearmodes (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	channel_t *channelp;

	char undo[64];

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

	strcpy(undo, "-ntismpkl ");

	if (channelp->flags & CHANNEL_HAS_KEY)
		strcat(undo, channelp->key);

	ssprintf (s,
		"%s M %s %s\n",
		config->server_numeric, channelp->name, undo);

	channelp->flags &= ~CHANNEL_HAS_KEY;
	channelp->flags &= ~CHANNEL_HAS_LIMIT;

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002CLEARMODES\002 issued by \002%s\002 on \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname, channelp->name);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_scan ()
                |
 DESCRIPTION    |    This function will scan a hostmask and return a list of users matching the hostmask
                |
                |    argv[0] = Sender
                |    argv[3] = Hostmask
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_scan (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	nickname_t *suserp;
	int numFound = 0;

	char userhost[1024];

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command SCAN\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	for (suserp = nickname_h; suserp; suserp = suserp->next)
	{
		server_t *serverp;

		sprintf(userhost, "%s@%s", suserp->username, suserp->hostname_p->hostname);

		if (!match(argv[3], userhost))
		{
//			serverp = Server_Find (userp->server);
			serverp = userp->server_p;
			ssprintf(s,
				"%s O %s :%s (%s) %s\n",
				myuserp->numeric, userp->numeric, suserp->nickname,
				userhost, serverp->name);
			numFound++;
		}
	}

	if (numFound)
	{
		ssprintf (s,
			"%s O %s :End of list - %d matches found\n",
			myuserp->numeric, userp->numeric, numFound);
		return 1;
	}
	else
	{
		ssprintf (s,
			"%s O %s :No matches found!\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_mode ()
                |
 DESCRIPTION    |    This function will allow admins to changes modes on a channel without having to be an op
                |
                |    argv[0] = Sender
                |    argv[3] = Channel
                |    argv[4] = Mode String
                |    argv[5] = (Optional) KEY or LIMIT
                |    argv[6] = (Optional) KEY OR LIMIT
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_mode (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
	channel_t *channelp;
	int addModes, pos = -1, curarg = 5;
	char mode_string[512], limit[6];


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	channelp = Channel_Find (argv[3]);
	if (!channelp)
	{
		ssprintf (s,
			"%s O %s :No such channel.\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	strcpy (mode_string, argv[4]);
	while (argv[4][++pos] != '\0')
	{
		if (argv[4][pos] == '+')
		{
			addModes = 1;
		}

		else if (argv[4][pos] == '-')
		{
			addModes = 0;
		}

		else if (argv[4][pos] == 'l')
		{
			if (addModes)
			{
				if (argc >= curarg)
				{
					sprintf(limit, "%d", atoi(argv[curarg]));
					strcat(mode_string, " ");
					strcat(mode_string, limit);
					curarg++;
				}
			}
		}

		else if (argv[4][pos] == 'k')
		{
			if (addModes)
			{
				if (argc >= curarg)
				{
					strcat(mode_string, " ");
					strcat(mode_string, argv[curarg]);
					curarg++;
					channelp->flags |= CHANNEL_HAS_KEY;
				}
			}

			else if ((channelp->flags & CHANNEL_HAS_KEY) && !addModes)
			{
				strcat (mode_string, " ");
				strcat (mode_string, channelp->key);
			}
		}
	}

	ssprintf (s,
		"%s M %s %s\n",
		config->server_numeric, channelp->name, mode_string);

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002MODE\002 issued by \002%s\002 on channel \002%s\002 with modes \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname,
//		channelp->name, mode_string);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_masskill ()
                |
 DESCRIPTION    |    This function will kill any user that matches a specified hostmask
                |
                |    argv[0] = Sender
                |    argv[3] = Hostmask
                |    argv[4] = Reason
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_masskill (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	nickname_t *kuserp;

	int curarg = 4;
	char reason[512], hostmask[512];

//	ircsp_debug(
//		"@(#) - (%s:%d) %s():  UWorld command MASSKILL\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find(argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	argc--;
	strcpy(reason, argv[curarg]);
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002MASSKILL\002 issued on \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	for (kuserp = nickname_h; kuserp; kuserp = kuserp->next)
	{
		sprintf(hostmask, "%s@%s", kuserp->username, kuserp->hostname_p->hostname);

		if (!match(argv[3], hostmask))
		{
			ssprintf(s,
				"%s D %s :%s!%s (MASSKILL (%s)\n",
				config->server_numeric, kuserp->numeric, myuserp->hostname_p->hostname, myuserp->nickname, reason);

			if (kuserp->hostname_p->numHosts > 1)
				kuserp->hostname_p->numHosts--;
			else
			{
				Hostname_Del (kuserp->hostname_p->hostname);
				kuserp->hostname_p = NULL;
			}
			Nickname_Del (kuserp->nickname);
		}
	}


	ssprintf(s,
		"%s WA :[\002%s\002] Complete \002MASSKILL\002 on host [%s]\n",
		config->server_numeric, userp->nickname, argv[3]);


//	ircsp_debug(
//		"@(#) - (%s:%d) %s():  End UWorld command MASSKILL\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;

}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_save ()
                |
 DESCRIPTION    |    This command will save all of our databases.  This will eventually change, i'm thinking for future versions
                |    of IRCSP we use MySQL for our data storage instead of files.
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_save (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command SAVE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002SAVE\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :Saving Databases...\n",
		myuserp->numeric, userp->numeric);


	ssprintf(s,
		"%s WA :\002SAVE\002 Requsted by [\002%s\002]\n",
		config->server_numeric, userp->nickname);

	uw_saveGlineDB ();
	uw_saveCommand ();

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command SAVE\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_die ()
                |
 DESCRIPTION    |    Save our databases and exit
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_die (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;



	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	if (!userp->admin_p)
	{
		ssprintf(s,
			"%s O %s :\002ACCESS DENIED\002\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}


//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002DIE\002 issued \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :Saving databases, and shutting down.\n",
		myuserp->numeric, userp->numeric);

	uw_saveGlineDB ();
	uw_saveCommand ();


	ircsp_exit (0, "Shutdown Requested by %s", userp->nickname);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_cmdlist ()
                |
 DESCRIPTION    |    This function will give a list of all UWorld commands and their levels and status, I might use this for
                |    the help index file to get a list of commands.
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_cmdlist (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	int cmd;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command CMDLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002CMDLIST\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s O %s :\002CMDLIST\002 \n",
		myuserp->numeric, argv[0]);


	for (cmd = 0; cmd < UW_NUM_CMDS; cmd++)
	{
		ssprintf(s,
			"%s O %s :\002%s\002 - Level: \002%d\002\n",
			myuserp->numeric, argv[0], UWorldCommand[cmd].command,
			UWorldCommand[cmd].minlevel);

		if (UWorldCommand[cmd].disabled)
			ssprintf(s,
				"%s O %s :|-  \002COMMAND DISABLED\002\n",
				myuserp->numeric, argv[0]);
	}

	ssprintf(s,
		"%s O %s :End of \002CMDLIST\002\n",
		myuserp->numeric, argv[0]);

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command CMDLIST\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_disablecmd ()
                |
 DESCRIPTION    |    This function will allow admins to disable commands
                |
                |    argv[0] = Sender
                |    argc[3] = Command
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_disablecmd (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	int cmd, exists = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command DISABLECMD\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	for (cmd = 0; cmd <= UW_NUM_CMDS; cmd++)
	{
		if ((!strcasecmp(argv[3], "AUTH")) ||
			(!strcasecmp(argv[3], "DISABLECMD")) ||
			(!strcasecmp(argv[3], "ENABLECMD")))
		{
			ssprintf(s,
				"%s O %s :We cannot disable \002%s\002\n",
				myuserp->numeric, argv[0], argv[3]);
			return 0;
		}
		else if (!strcasecmp(UWorldCommand[cmd].command, argv[3]))
		{
			exists = 1;
			if (UWorldCommand[cmd].disabled)
			{
				ssprintf(s,
					"%s O %s :\002%s\002 already \002DISABLED\002\n",
					myuserp->numeric, argv[0], UWorldCommand[cmd].command);
				return 0;
			}
			else
			{
				ssprintf(s,
					"%s O %s :\002%s\002 is now \002DISABLED\002\n",
					myuserp->numeric, argv[0], UWorldCommand[cmd].command);

				UWorldCommand[cmd].disabled = 1;
				uw_saveCommand ();
				return 1;
			}
		}
	}

	if (!exists)
	{
		ssprintf(s,
			"%s O %s :Command [%s] not found.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command DISABLECMD\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_enablecmd ()
                |
 DESCRIPTION    |    This function will allow admins to re-enable disabled commands
                |
                |    argv[0] = Sender
                |    argv[3] = Command
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_enablecmd (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *userp;
	nickname_t *myuserp;
	int cmd, exists = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command ENABLECMD\n",
//		__FILE__, __LINE__, __FUNCTION__);


	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	for (cmd = 0; cmd <= UW_NUM_CMDS; cmd++)
	{
		if ((!strcasecmp(argv[3], "AUTH")) ||
			(!strcasecmp(argv[3], "DISABLECMD")) ||
			(!strcasecmp(argv[3], "ENABLECMD")))
		{
			ssprintf(s,
				"%s O %s :\002%s\002 is \002ENABLED\002 by default.\n",
				myuserp->numeric, argv[0], argv[3]);

			return 0;
		}

		else if (!strcasecmp(UWorldCommand[cmd].command, argv[3]))
		{
			exists = 1;
			if (!UWorldCommand[cmd].disabled)
			{
				ssprintf(s,
					"%s O %s :\002%s\002 is already \002ENABLED\002\n",
					myuserp->numeric, argv[0], argv[3]);
				return 0;
			}
			else
			{
				ssprintf(s,
					"%s O %s :\002%s\002 is \002ENABLED\002\n",
					myuserp->numeric, argv[0], argv[3]);
				UWorldCommand[cmd].disabled = 0;
				uw_saveCommand ();
				return 1;
			}
		}
	}

	if (!exists)
	{
		ssprintf(s,
			"%s O %s :\002%s\002 was not found.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command ENABLECMD\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_restart ()
                |
 DESCRIPTION    |    This function will cause IRCSP to split off, save its databases, and restart the binary.  In order for this
                |    command to work, IRCSP needs to be configured to use DAEMON mode, if its in the forground we will ignore
                |    this request.
                |
                |    argv[0] = Sender
                |    argv[3] = Reason
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_restart (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;

	char reason[512];
	int curarg = 3;
	argc--;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command RESTART\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	if (!config->fork)
	{
		ssprintf(s,
			"%s O %s :\002WARNING\002 - Proccess is not forked, this command is disabled\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	// Copy our reason

	strcpy(reason, argv[3]);
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

	ssprintf (s,
		"%s O %s :Saving databases, and restarting.\n",
		myuserp->numeric, userp->numeric);

	uw_saveGlineDB ();
	uw_saveCommand ();

//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002RESTART\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s WA :\002RESTART\002 requested by \002%s\002 (%s)\n",
		config->server_numeric, userp->nickname, reason);


	close(s);

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command RESTART\n",
//		__FILE__, __LINE__, __FUNCTION__);

	system ("source/ircsp");

	raise (SIGTERM);

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_rehash ()
                |
 DESCRIPTION    |    This function should remove all admins and glines from memory, channels, nicks, etc (maybe), save, and
                |    reload from our databases
                |
                |    argv[0] = Sender
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_rehash (argc, argv)
	int argc;
	char *argv[];
{
	nickname_t *myuserp;
	nickname_t *userp;
//	Admin_List *adminp;
	Gline *glinep;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  UWorld command REHASH\n",
//		__FILE__, __LINE__, __FUNCTION__);

	userp = Nickname_Find (argv[0]);
	if (!userp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = Nickname_Find (config->uworld_nickname);
	if (!myuserp)
	{
//		ircsp_debug (
//			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
//			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


//	ircsp_notice (
//		"@(#) - (%s:%d) %s():  UWorld command \002REHASH\002 issued by \002%s\002\n",
//		__FILE__, __LINE__, __FUNCTION__, userp->nickname);



	ssprintf (s,
		"%s WA :REHASHING [Requested: %s]\n",
		config->server_numeric, userp->nickname);

	ircsp->rehashing = 1;


	// Remove GLINES from memory.

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		delGline (glinep->mask);
	}


	// Remove Administrators from memory and send out notice


	// Reload ADMINS from DB

//	loadAdminDB ();


	// Reload GLINES from the DB

	uw_loadGlineDB ();

	// Now we need to SYNC our glines.

	synch ();

	ircsp->rehashing = 0;

//	ircsp_debug (
//		"@(#) - (%s:%d) %s():  End UWorld command REHASH\n",
//		__FILE__, __LINE__, __FUNCTION__);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    uw_chlevel (0
                |
 DESCRIPTION    |    This function will allow admins to change the minimum level required for a command without the need of
                |    having to recompile.
                |
                |    argv[0] = Sender
                |    argv[3] = Command
                |    argv[4] = New Minimum Level
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int uw_chlevel (argc, argv)
	int		argc;
	char		*argv[];
{
	nickname_t	*p_nickname_src;
	nickname_t	*p_nickname_dst;

	int		command;
	int		found;


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	 Search Nickname_List for the Source Nickname (argv[0])
	-------------------------------------------------------------------------------------------------------------------------
	*/
	p_nickname_src = Nickname_Find (argv[0]);
	if (p_nickname_src == NULL)
	{
		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_NOSUCHNICK),
				__FILE__, __LINE__, __FUNCTION__, argv[0]);

			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__);
		}


		return 0;
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	 Search Nickname_List for our Destination Nickname (config->uworld_nickname)
	-------------------------------------------------------------------------------------------------------------------------
	*/
	p_nickname_dst = Nickname_Find (config->uworld_nickname);
	if (p_nickname_dst == NULL)
	{
		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_NOSUCHNICK),
				__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__);
		}


		return 0;
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	 See what command the source is trying to run chlevel against, if the command is "CHLEVEL" we ignore it. We don't
	 Want the access level for this command to ever change.
	
	 We should maybe consider doing the same for "DIE", "RESTART', "ENABLECMD" and "DISABLECMD"
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if ((!strcasecmp(argv[3], "chlevel")))
	{
		cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname, "The minimum level on \002CHLEVEL\002 cannot be changed");
		return 0;
	}


	cmd_notice_operators (p_nickname_dst->nickname, "UWorld command \002CHLEVEL\002 issued by \002%s\002", p_nickname_src->nickname);


	if (atoi(argv[4]) > 1000)
	{
		cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname, "The max level that can be used is 1000");
		return 0;
	}

	for (command = 0; command < UW_NUM_CMDS; command++)
	{
		if (!strcasecmp(UWorldCommand[command].command, argv[3]))
		{
			UWorldCommand[command].minlevel = atoi(argv[4]);
			cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname,
				"The minimum level for \002%s\002 has been changed to \002%d\002",
				UWorldCommand[command].command,
				UWorldCommand[command].minlevel);

			return 1;
		}
		else
		{
			found = 0;
		}
	}

	if (!found)
	{
		cmd_notice (p_nickname_dst->nickname, p_nickname_src->nickname, "Command [\002%s\002] Not found", argv[3]);
		return 0;
	}

	return TRUE;
}
