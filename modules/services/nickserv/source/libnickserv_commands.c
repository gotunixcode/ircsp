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


/* System Includes */
#include	<dlfcn.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>
#include	<assert.h>
#include	<mariadb/mysql.h>


/* IRCSP Core Includes */
#include	"modules.h"
#include	"ircsp.h"
#include	"log.h"
#include	"text.h"
#include	"match.h"
#include	"alloc.h"
#include	"conf.h"
#include	"servers.h"
#include	"users.h"
#include	"channels.h"
#include	"socket.h"
#include	"version.h"
#include	"mysql.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/* NickServ Module Includes */
#include	"libnickserv.h"
#include	"libnickserv_admin.h"
#include	"libnickserv_commands.h"
#include	"libnickserv_nickinfo.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
    STRUCTURES

       Command, Function, Minargs, Disabled, Ident Required, Minimum level
---------------------------------------------------------------------------------------------------------------------------------
*/
struct nickserv_commands NickServCommand[] = {
	{	"VERSION",			&ns_version,			3,	0,	0,	0,	},
	{	"HELP",				&ns_help,			3,	0,	0,	0,	},
	{	"REGISTER",			&ns_register,			4,	0,	0,	0,	},
	{	"IDENTIFY",			&ns_identify,			4,	0,	0,	0,	},
	{	"DROP",				&ns_drop,			3,	0,	1,	0,	},
	{	"ACCESS",			&ns_access,			4,	0,	1,	0,	},
	{	"SET",				&ns_set,			4,	0,	1,	0,	},
	{	"RECOVER",			&ns_recover,			5,	0,	0,	0,	},
	{	"RELEASE",			&ns_release,			5,	0,	0,	0,	},
};


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    m_privmsg ()
                   |
    DESCRIPTION    |    This function will handle privmsg()'s going to NickServ
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = Command and Arguments
                   |
    RETURNS        |    NONE
---------------------------------------------------------------------------------------------------------------------------------
*/
void m_privmsg (p_Module, src, dst, command)
	struct _module	*p_Module;
	char		*src;
	char		*dst;
	char		*command;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	char			*parv[32];
	int			i_loop = 0;
	int			i_command = 0;
	int			level = 0;
	int			argc = 2;

	assert (src != NULL);
	assert (dst != NULL);
	assert (command != NULL);

	chop (command);
	parv[0] = chop (src);
	parv[1] = chop (dst);
	parv[2] = strtok (command, " ");
	parv[2]++;
	argc = 2;

	while (parv[argc])
	{
		parv[++argc] = strtok (NULL, " ");
	}

	dst_nickname_p = Nickname_Find (parv[1]);
	assert (dst_nickname_p != NULL);

	src_nickname_p = Nickname_Find (parv[0]);
	assert (src_nickname_p != NULL);



	if (src_nickname_p->flags & NICK_IS_ROOT)
	{
		level = 3;
	}
	else if (src_nickname_p->flags & NICK_IS_ADMIN)
	{
		level = 2;
	}
	else if (src_nickname_p->flags & NICK_IS_OPER)
	{
		level = 1;
	}
	else
	{
		level = 0;
	}

	for (i_loop = 0; i_loop <= NICKSERV_NUM_COMMANDS; i_loop++)
	{
		if (!strcasecmp (parv[2], NickServCommand[i_loop].command))
		{
			i_command = TRUE;

			if (level >= NickServCommand[i_loop].minlevel)
			{
				if (NickServCommand[i_loop].ident)
				{
					nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
					if (!nickinfo_p)
					{
						cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
							"You're nickname is not REGISTERED!"
						);

						cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
							"\002/msg %s HELP REGISTER\002",
							dst_nickname_p->nickname
						);

						if (DEBUG)
						{
							ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
								__FILE__, __LINE__, __FUNCTION__
							);
						}

						return ;
					}

					if (!(src_nickname_p->flags & NICK_IS_IDENTIFIED))
					{
						cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
							"Password authentication is required for this command"
						);

						cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
							"Retry after typing: \002/msg %s IDENTIFY [password]\002",
							dst_nickname_p->nickname
						);

						if (DEBUG)
						{
							ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
								__FILE__, __LINE__, __FUNCTION__
							);
						}

						return ;
					}
				}

				if (NickServCommand[i_loop].disabled)
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"Command \002%s\002 is disabled",
						NickServCommand[i_loop].command
					);

					if (DEBUG)
					{
						ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return ;
				}


				if (argc < NickServCommand[i_loop].minargs)
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"Incorrect Syntax - Try /msg %s HELP %s",
						dst_nickname_p->nickname, NickServCommand[i_loop].command
					);

					if (DEBUG)
					{
						ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return;
				}


				if (NickServCommand[i_loop].func (argc, parv))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Pushing Command [%s] for [%s (%s@%s)]\n",
						__FILE__, __LINE__, __FUNCTION__,
						NickServCommand[i_loop].command, src_nickname_p->nickname,
						src_nickname_p->username, src_nickname_p->hostname_p->hostname
					);
				}
			}
			else
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002ACCESS DENIED\002"
				);

				return ;
			}
		}
	}


	if (parv[2][0] == '\001')
	{
		parv[2]++;
		if (!strncasecmp (parv[2], "VERSION", 7))
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\001NickServ IRCSP %s\001",
				version_number
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
		}
	}


	if (!i_command)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Unknown command \002%s\002",
			parv[2]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return ;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_version ()
                   |
    DESCRIPTION    |    NickServ "VERSION" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = Command
                   |
                   |    ADAAg P AIAAA :version
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int ns_version (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickname_t	*src_nickname_p;
	nickname_t	*dst_nickname_p;
	int		i_loop = 0;


	assert (argc != 0);
	assert (argv != NULL);


	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);


	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);


	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "---------------------------------------------------");
	for (i_loop = 0; info_text[i_loop]; i_loop++)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "%s", info_text[i_loop]);
	}
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "---------------------------------------------------");
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "IRCSP Version Information");
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "- Codename: \002%s\002", code_name);
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "-  Version: \002%s\002", version_number);
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "-    BUILD: \002%s\002", version_build);
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "NickServ Module Information");
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "-  Version: \002%s\002", ModuleVersion);
	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "---------------------------------------------------");


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    translate ()
                   |
    DESCRIPTION    |    This function allows us to set arguments inside HELPFILES
                   |
                   |        inBuf  = Input Buffer
                   |        OutBuf = Output Buffer
                   |
    RETURNS        |    string
---------------------------------------------------------------------------------------------------------------------------------
*/
static char *help_translate (inBuf, outBuf)
	char		*inBuf;
	char		*outBuf;
{
	int		inBufPos = -1;
	int		outBufPos = -1;
	int		pos = -1;

	while (inBuf [++inBufPos] != '\0')
	{
		if( inBuf [inBufPos] == '^')
		{
			switch (inBuf [++inBufPos])
			{
				case 'N':
					while (this_module_p->ServiceNickname [++pos] != '\0')
					{
						outBuf [++outBufPos] = this_module_p->ServiceNickname [pos];
					}
					break;

				case 'B':
					outBuf [++outBufPos] = '\002';
					break;
			}
		}
		else
		{
			outBuf [++outBufPos] = inBuf [inBufPos];
		}
	}

	outBuf [outBufPos] = '\0';
	return outBuf;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_help ()
                   |
    DESCRIPTION    |    NickServ "HELP" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = HELP Command
                   |        argv[3] = Help Topic
                   |
                   |    ADAAg P AIAAA :help
                   |    ADAAg P AIAAA :help register
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_help (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	char			buf [256];
	char			newbuf [256];
	char			helpfile [256];
	FILE			*helpfile_fp;
	int			level;
	int			show = TRUE;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	if (argc == 3)
	{
		sprintf (helpfile, "%s/helpfiles/nickserv/index", VARPATH);
	}

	else if (argc == 4)
	{
		sprintf (helpfile, "%s/helpfiles/nickserv/%s", VARPATH, argv[3]);
	}

	else if (argc == 5)
	{
		sprintf (helpfile, "%s/helpfiles/nickserv/%s.%s", VARPATH, argv[3], argv[4]);
	}

	helpfile_fp = fopen (helpfile, "r");
	if (!helpfile_fp)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"No help available on this topic"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}
	else
	{
		while (!feof (helpfile_fp))
		{
			if (fgets (buf, 256, helpfile_fp))
			{
				if (buf[0] == '@')
				{
					buf[0] = '0';
					level = atoi (buf);

					if ( (src_nickname_p->flags & NICK_IS_ROOT) && (src_nickname_p->flags & NICK_IS_OPER) && (level <= 3) )
					{
						show = TRUE;
					}

					else if ( (src_nickname_p->flags & NICK_IS_ADMIN) && (src_nickname_p->flags & NICK_IS_OPER) && (level <= 2) )
					{
						show = TRUE;
					}

					else if ( (src_nickname_p->flags & NICK_IS_OPER) && (level <= 1) )
					{
						show = TRUE;
					}

					else if ( (!(src_nickname_p->flags & NICK_IS_OPER)) & (level == 0) )
					{
						show = TRUE;
					}

					else
					{
						show = FALSE;
					}

					continue;
				}

				if (show)
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname, "%s",
						help_translate (buf, newbuf)
					);
				}
			}
		}
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_register ()
                   |
    DESCRIPTION    |    NickServ "REGISTER" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = REGISTER Command
                   |        argv[3] = Password
                   |
                   |    ADAAg P AIAAA :register XXXXXXX
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_register (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL			*db_connection;
	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	char			*password;
	char			*hostmask;
	char			*query;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
	if (nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Sorry, \002%s\002 is already a registered nickname!",
			src_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (strlen (argv[3]) < 8)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password doesn't meet password length requirements"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	password = crypt_sha512 (argv[3]);

	nickinfo_p = Nickinfo_Create (src_nickname_p->nickname);
	if (!nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Sorry registeration failed."
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	hostmask = (char *)ircsp_calloc (1, strlen (src_nickname_p->username) + strlen (src_nickname_p->hostname_p->hostname) + 10);
	assert (hostmask != NULL);
	sprintf (hostmask, "%s@%s", src_nickname_p->username, src_nickname_p->hostname_p->hostname);

	nickinfo_p->password = (char *)ircsp_calloc (1, strlen (password) + 5);
	nickinfo_p->last_login = (char *)ircsp_calloc (1, strlen (hostmask) + 5);

	assert (nickinfo_p->password != NULL);
	assert (nickinfo_p->last_login != NULL);

	strcpy (nickinfo_p->password, password);
	strcpy (nickinfo_p->last_login, hostmask);

	nickinfo_p->last_seen = time (NULL);
	nickinfo_p->registered = time (NULL);
	nickinfo_p->flags = 0;

	ircsp_free (hostmask);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"Nickname \002%s\002 has been registered successfully!",
		src_nickname_p->nickname
	);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"The password for this nickname has been set to \002%s\002 - Remember this for later use",
		argv[3]
	);

	nickinfo_p->nickname_p = src_nickname_p;
	src_nickname_p->flags |= NICK_IS_IDENTIFIED;

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Lets try to insert the new nickname into the nickserv database, if this fails it should be picked up by our next
	    full save, unless we crash before then.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	db_connection = ircsp_mysql_connect ();
	if (!db_connection)
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]:  Failed to connect to database server\n",
			__FILE__, __LINE__, __FUNCTION__
		);
	}
	else
	{
		ircsp_mysql_insert (NSDB_NICKSERV,
			"0,\"%s\", \"%s\", \"%s\", %lu, %lu, %d",
			nickinfo_p->nickname, nickinfo_p->password, nickinfo_p->last_login,
			nickinfo_p->last_seen, nickinfo_p->registered, nickinfo_p->flags
		);
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_identify ()
                   |
    DESCRIPTION    |    NickServ "IDENTIFY" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = IDENTIFY Command
                   |        argv[3] = Password
                   |
                   |    ADAAg P AIAAA :IDENTIFY XXXXXXX
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_identify (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;
	char			*password;
	char			*hostmask;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	if (src_nickname_p->flags & NICK_IS_IDENTIFIED)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"You have already identified for your nickname \002%s\002",
			src_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
	if (!nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"This nickname \002%s\002 is not registered!",
			src_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Is the nick suspended
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (nickinfo_p->flags & NICKSERV_SUSPENDED)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Sorry, your nick has been suspended"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	/*
	-------------------------------------------------------------------------------------------------------------------------
	    First we need to check if the nickname has SECURE mode enabled
	    If it does we need to make sure that the users hostmask matches something that is listed
	    on the access list for the nickname
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (nickinfo_p->flags & NICKSERV_SECURE)
	{
		hostmask = (char *)ircsp_calloc (
			1, strlen (src_nickname_p->username) + strlen (src_nickname_p->hostname_p->hostname) + 10
		);

		sprintf (hostmask, "%s@%s", src_nickname_p->username, src_nickname_p->hostname_p->hostname);

		nickinfo_access_p = NickinfoAccess_Wildcard (nickinfo_p, hostmask);
		if (!nickinfo_access_p)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002IDENTIFY FAILED\002 - \002%s\002 is not on the access list for this nickname.",
				hostmask
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		ircsp_free (hostmask);
	}

	password = crypt_sha512 (argv[3]);

	if (!strcmp (nickinfo_p->password, password))
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Last Seen: %s",
			lrange (ctime (&nickinfo_p->last_seen), 1)
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Last Login: %s",
			nickinfo_p->last_login
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password accepted - you are now recognized"
		);

		/* Update last_login hostmask */
		hostmask = (char *)ircsp_calloc (
			1, strlen (src_nickname_p->username) + strlen (src_nickname_p->hostname_p->hostname) + 10
		);

		sprintf (hostmask, "%s@%s", src_nickname_p->username, src_nickname_p->hostname_p->hostname);

		ircsp_free (nickinfo_p->last_login);
		nickinfo_p->last_login = (char *)ircsp_calloc (1, strlen (hostmask) + 5);
		assert (nickinfo_p->last_login != NULL);
		strcpy (nickinfo_p->last_login, hostmask);
		ircsp_free (hostmask);

		/* Update last seen time */
		nickinfo_p->last_seen = time (NULL);

		/* Check for services root */
		services_root_p = ServicesRoot_Search (nickinfo_p->nickname);
		if (services_root_p)
		{
			if (services_root_p->flags & ADMIN_SUSPENDED)
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"... Services Root - \002SUSPENDED\002"
				);
			}
			else
			{
				services_root_p->nickinfo_p	= nickinfo_p;
				services_root_p->nickname_p	= src_nickname_p;
				src_nickname_p->flags		|= NICK_IS_ROOT;

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"... Services Root - \002ENABLED\002"
				);

				if (!(nickinfo_p->flags & NICKSERV_SECURE))
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"It is \002RECOMMENDED\002 as a services root user"
					);

					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"That you maintain a host access list and enable SECURE mode"
					);

					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"See \002/msg %s HELP ACCESS\002 and \002/msg %s HELP SET SECURE\002",
						dst_nickname_p->nickname, dst_nickname_p->nickname
					);
				}
			}
		}

		/* Check for services admin */
		services_admin_p = ServicesAdmin_Search (nickinfo_p->nickname);
		if (services_admin_p)
		{
			if (services_admin_p->flags & ADMIN_SUSPENDED)
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"... Services Admin - \002SUSPENDED\002"
				);
			}
			else
			{
				services_admin_p->nickinfo_p	= nickinfo_p;
				services_admin_p->nickname_p	= src_nickname_p;
				src_nickname_p->flags		|= NICK_IS_ADMIN;

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"... Services Admin - \002ENABLED\002"
				);

				if (!(nickinfo_p->flags & NICKSERV_SECURE))
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"It is \002RECOMMENDED\002 as a services admin user"
					);

					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"That you maintain a host access list and enable SECURE mode"
					);

					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"See \002/msg %s HELP ACCESS\002 and \002/msg %s HELP SET SECURE\002",
						dst_nickname_p->nickname, dst_nickname_p->nickname
					);
				}
			}
		}

		/* User is now identified */
		nickinfo_p->nickname_p		= src_nickname_p;
		src_nickname_p->flags		|= NICK_IS_IDENTIFIED;
	}
	else
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Login incorrect"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_drop ()
                   |
    DESCRIPTION    |    NickServ "DROP" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = DROP Command
                   |
                   |    ADAAg P AIAAA :DROP
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_drop (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
	if (!nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"This nickname \002[%s]\002 is not registered.",
			src_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (!(src_nickname_p->flags & NICK_IS_IDENTIFIED))
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password authentication is required for this command."
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Retry after typing: /msg %s IDENTIFY [\002password\002]",
			dst_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	nickinfo_access_p = nickinfo_p->nickinfo_access_h;
	while (nickinfo_access_p)
	{
		ircsp_mysql_delete (NSDB_NICKSERV_ACCESS,
			"nickname=\"%s\" AND hostmask=\"%s\"",
			nickinfo_p->nickname, nickinfo_access_p->hostmask
		);

		NickinfoAccess_Remove (nickinfo_p, nickinfo_access_p->hostmask);
		nickinfo_access_p = nickinfo_access_p->next;
	}

	nickinfo_p->nickinfo_access_h	= NULL;

	src_nickname_p->flags		&= ~NICK_IS_IDENTIFIED;

	ircsp_mysql_delete (NSDB_NICKSERV,
		"nickname=\"%s\"",
		nickinfo_p->nickname
	);

	Nickinfo_Remove (src_nickname_p->nickname);

	cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
		"Nickname [\002%s\002] has been dropped.",
		src_nickname_p->nickname
	);

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_access ()
                   |
    DESCRIPTION    |    NickServ "ACCESS" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = ACCESS Command
                   |        argv[3] = ACCESS Sub-Command
                   |        argv[4] = ACCESS hostmask (Depending on sub-commend add/remove)
                   |
                   |    ADAAg P AIAAA :ACCESS LIST
                   |    ADAAg P AIAAA :ACCESS ADD *@*.foo.com
                   |    ADAAg P AIAAA :ACCESS DEL *@*.foo.com
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_access (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	char			*hostmask;
	char			*username;
	char			*hostname;
	int			curdot = FALSE;
	int			numdot = FALSE;
	int			bad_hostname = FALSE;

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
	if (!nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"This nickname [\002%s\002] is not registered",
			src_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (!(src_nickname_p->flags & NICK_IS_IDENTIFIED))
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password authentication required for this command"
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Retry after typing: /msg %s IDENTIFY [\002password\002]",
			dst_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (!strcasecmp (argv[3], "ADD"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - Try /msg %s HELP ACCESS ADD",
				dst_nickname_p->nickname
			);

			return FALSE;
		}

		hostmask = (char *)ircsp_calloc (1, strlen (argv[4]) + 5);
		assert (hostmask != NULL);
		strcpy (hostmask, argv[4]);

		if (!match (argv[4], "*@*"))
		{
			bad_hostname = TRUE;
		}
		else
		{
			username = strtok (argv[4], "@");
			hostname = strtok (NULL, " ");
			numdot = tokenize (hostname, '.');

			bad_hostname = TRUE;
			for (curdot = 1; curdot < numdot; curdot++)
			{
				if (match (token (hostname, curdot), "*") != 0)
				{
					bad_hostname = FALSE;
				}
			}
		}

		if (bad_hostname)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Bad hostmask - Try something like: username@*.domain.com"
			);

			return FALSE;
		}

		nickinfo_access_p = nickinfo_p->nickinfo_access_h;
		while (nickinfo_access_p)
		{
			if (!strcasecmp (nickinfo_access_p->hostmask, hostmask))
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"Duplicate record found!"
				);

				return FALSE;
			}

			if (!match (hostmask, nickinfo_access_p->hostmask))
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"Wildcard match found! [\002%s\002 -> \002%s\002]",
					hostmask, nickinfo_access_p->hostmask
				);

				return FALSE;

			}

			nickinfo_access_p = nickinfo_access_p->next;
		}

		nickinfo_access_p = NickinfoAccess_Create (nickinfo_p, hostmask);
		assert (nickinfo_access_p != NULL);
		assert (nickinfo_access_p->hostmask != NULL);

		ircsp_mysql_insert (NSDB_NICKSERV_ACCESS, "0,\"%s\",\"%s\"",
			nickinfo_p->nickname, hostmask
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Hostmask [\002%s\002] has been added to your access list",
			nickinfo_access_p->hostmask
		);

		return TRUE;
	}

	else if (!strcasecmp (argv[3], "DEL"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - Try /msg %s HELP ACCESS DEL",
				dst_nickname_p->nickname
			);

			return FALSE;
		}

		nickinfo_access_p = NickinfoAccess_Search (nickinfo_p, argv[4]);
		if (!nickinfo_access_p)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002%s\002 was not found on your access list",
				argv[4]
			);

			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Try running: /msg %s ACCESS LIST for your current access list",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Hostmask [\002%s\002] has been deleted from your access list",
				nickinfo_access_p->hostmask
			);

			ircsp_mysql_delete (NSDB_NICKSERV_ACCESS, "nickname=\"%s\" AND hostmask=\"%s\"",
				nickinfo_p->nickname, nickinfo_access_p->hostmask
			);

			NickinfoAccess_Remove (nickinfo_p, nickinfo_access_p->hostmask);

			return TRUE;
		}
	}

	else if (!strcasecmp (argv[3], "LIST"))
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Hostname access list for nickname: \002%s\002",
			src_nickname_p->nickname
		);

		nickinfo_access_p = nickinfo_p->nickinfo_access_h;
		while (nickinfo_access_p)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002%s\002",
				nickinfo_access_p->hostmask
			);

			nickinfo_access_p = nickinfo_access_p->next;
		}

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"End of list"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return TRUE;
	}

	else
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Incorrect Syntax - Try /msg %s HELP ACCESS",
			dst_nickname_p->nickname
		);

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_set ()
                   |
    DESCRIPTION    |    NickServ "SET" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = SET Command
                   |        argv[3] = SET Option
                   |        argv[4] = Optional (on/off)
                   |
                   |    ADAAg P AIAAA :register XXXXXXX
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_set (argc, argv)
	int		argc;
	char		*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*dst_nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	services_root_t		*services_root_p;
	services_admin_t	*services_admin_p;
	char			*password;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	dst_nickname_p = Nickname_Find (argv[1]);
	assert (dst_nickname_p != NULL);
	assert (dst_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (src_nickname_p->nickname);
	if (!nickinfo_p)
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"This nickname \002%s\002 is not registered.",
			src_nickname_p->nickname
		);

		return FALSE;
	}

	if (!(src_nickname_p->flags & NICK_IS_IDENTIFIED))
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password authentication is required for this command"
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Retry after typing: \002/msg %s IDENTIFY password\002",
			dst_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (!strcasecmp (argv[3], "KILL"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Invalid Syntax - See: \002/msg %s HELP SET KILL\002",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		if (!strcasecmp (argv[4], "ON"))
		{
			if (!(nickinfo_p->flags & NICKSERV_KILLPROTECT))
			{
				nickinfo_p->flags |= NICKSERV_KILLPROTECT;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d WHERE nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002KILL PROTECTION\002 has been enabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return TRUE;
			}
			else
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002KILL PROTECTION\002 is already enabled on nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
		}
		else if (!strcasecmp (argv[4], "OFF"))
		{
			if (!(nickinfo_p->flags & NICKSERV_KILLPROTECT))
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002KILL PROTECTION\002 is already disabled on nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
			else
			{
				nickinfo_p->flags &= ~NICKSERV_KILLPROTECT;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d WHERE nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002KILL PROTECTION\002 has been disabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return TRUE;
			}
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - Try: \002/msg %s HELP SET KILL",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
	}

	else if (!strcasecmp (argv[3], "SECURE"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - See: \002/msg %s HELP SET SECURE\002",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		if (!strcasecmp (argv[4], "ON"))
		{
			if (!(nickinfo_p->flags & NICKSERV_SECURE))
			{
				nickinfo_access_p = nickinfo_p->nickinfo_access_h;
				if (!nickinfo_access_p)
				{
					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"You need to have a hostmask access list to enable this"
					);

					cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
						"See: \002/msg %s HELP ACCESS\002",
						dst_nickname_p->nickname
					);

					if (DEBUG)
					{
						ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return FALSE;
				}

				nickinfo_p->flags |= NICKSERV_SECURE;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d WHERE nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002SECURE\002 has been enabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return TRUE;
			}
			else
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002SECURE\002 is already enabled on nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
		}
		else if (!strcasecmp (argv[4], "OFF"))
		{
			if (!(nickinfo_p->flags & NICKSERV_SECURE))
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002SECURE\002 is already disabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
			else
			{
				nickinfo_p->flags &= ~NICKSERV_SECURE;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d WHERE nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002SECURE\002 has been disabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				return TRUE;
			}
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - Try: \002/msg %s HELP SET SECURE\002",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
	}

	else if (!strcasecmp (argv[3], "AUTOIDENTIFY"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - See: \002/msg %s HELP SET AUTOIDENTIFY\002",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		/* Check if user is a services root */
		services_root_p = ServicesRoot_Search (src_nickname_p->nickname);
		if (services_root_p)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002AUTO IDENTIFY\002 is not allowed for services root users"
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		/* Check if a user is a services admin */
		services_admin_p = ServicesAdmin_Search (src_nickname_p->nickname);
		if (services_admin_p)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002AUTO IDENTIFY\002 is not allowed for services admin users"
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		if (!strcasecmp (argv[4], "ON"))
		{
			nickinfo_access_p = nickinfo_p->nickinfo_access_h;
			if (!nickinfo_access_p)
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"You need to have a hostmask access list enabled to enable this"
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"See: \002/msg %s HELP ACCESS\002",
					dst_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}

			if (!(nickinfo_p->flags & NICKSERV_AUTOIDENTIFY))
			{
				nickinfo_p->flags |= NICKSERV_AUTOIDENTIFY;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d WHERE nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002AUTO IDENTIFY\002 has been enabled on nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return TRUE;
			}
			else
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002AUTO IDENTIFY\002 is already enabled on nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
		}
		else if (!strcasecmp (argv[4], "OFF"))
		{
			if (!(nickinfo_p->flags & NICKSERV_AUTOIDENTIFY))
			{
				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002AUTO IDENTIFY\002 is already disabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}
			else
			{
				nickinfo_p->flags &= ~NICKSERV_AUTOIDENTIFY;

				ircsp_mysql_update (NSDB_NICKSERV, "SET flags=%d where nickname=\"%s\"",
					nickinfo_p->flags, nickinfo_p->nickname
				);

				cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
					"\002AUTO IDENTIFY\002 has been disabled for nickname \002%s\002",
					src_nickname_p->nickname
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return TRUE;
			}
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - See: \002/msg %s HELP SET AUTOIDENTIFY",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
	}

	else if (!strcasecmp (argv[3], "PASSWORD"))
	{
		if (argv[4] == NULL)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Incorrect Syntax - See: \002/msg %s HELP SET PASSWORD\002",
				dst_nickname_p->nickname
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		if (strlen (argv[4]) < 8)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"Password specified does not meet the password length requirements"
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		password = crypt_sha512 (argv[4]);

		ircsp_free (nickinfo_p->password);
		nickinfo_p->password = (char *)ircsp_calloc (1, strlen (password) + 5);
		assert (nickinfo_p->password != NULL);
		strcpy (nickinfo_p->password, password);

		ircsp_mysql_update (NSDB_NICKSERV, "SET password=\"%s\" WHERE nickname=\"%s\"",
			nickinfo_p->password, nickinfo_p->nickname
		);

		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Password for nickname \002%s\002 has been changed to \002%s\002",
			src_nickname_p->nickname, argv[4]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return TRUE;
	}

	else if (!strcasecmp (argv[3], "LIST"))
	{
		/* Kill Protection */
		if (nickinfo_p->flags & NICKSERV_KILLPROTECT)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002KILL PROTECTION\002 - ENABLED"
			);
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002KILL PROTECTION\002 - DISABLED"
			);
		}

		/* Secure Mode */
		if (nickinfo_p->flags & NICKSERV_SECURE)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002SECURE\002 - ENABLED"
			);
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002SECURE\002 - DISABLED"
			);
		}

		/* Auto Identify */
		if (nickinfo_p->flags & NICKSERV_AUTOIDENTIFY)
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002AUTO IDENTIFY\002 - ENABLED"
			);
		}
		else
		{
			cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
				"\002AUTO IDENTIFY\002 - DISABLED"
			);
		}

		return TRUE;
	}

	else
	{
		cmd_notice (dst_nickname_p->nickname, src_nickname_p->nickname,
			"Incorrect Syntax - See: \002/msg %s HELP SET\002",
			dst_nickname_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_recover ()
                   |
    DESCRIPTION    |    NickServ "RECOVER" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = RECOVER Command
                   |        argv[3] = RECOVER Nickname
                   |        argv[4] = RECOVER Password
                   |
                   |    ADAAg P AIAAA :recover evilicey 1234
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_recover (argc, argv)
	int	argc;
	char	*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_jupe_t		*nickinfo_jupe_p;
	char			*password;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (argv[3]);
	if (!nickinfo_p)
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"Nickname [\002%s\002] is not registered!",
			argv[3]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	nickname_p = Nickname_Find (argv[3]);
	if (!nickname_p)
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"Nickname [\002%s\002] is not currently online",
			argv[3]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	password = crypt_sha512 (argv[4]);
	if (!strcmp (nickinfo_p->password, password))
	{
		cmd_kill (config->nickserv_p->nickname, nickname_p->nickname,
			"NICKNAME RECOVERY"
		);

		cmd_nickjupe (nickinfo_p->nickname, config->nickserv_p->username,
			config->server_name, "NickServ JUPE"
		);

		nickname_p = Nickname_Find (nickinfo_p->nickname);
		nickinfo_jupe_p = NickinfoJupe_Create (nickinfo_p->nickname);
		if (nickinfo_jupe_p)
		{
			nickinfo_jupe_p->nickinfo_p = nickinfo_p;
			nickinfo_jupe_p->nickname_p = nickname_p;
		}

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return TRUE;
	}
	else
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"\002ACCESS DENIED\002"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return FALSE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ns_release ()
                   |
    DESCRIPTION    |    NickServ "RELEASE" Command
                   |
                   |        argv[0] = Source Nickname/Numeric
                   |        argv[1] = Destination Nickname/Numeric
                   |        argv[2] = RELEASE Command
                   |        argv[3] = RELEASE Nickname
                   |        argv[4] = RELEASE Password
                   |
                   |    ADAAg P AIAAA :release evilicey 1234
                   |
    RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int
ns_release (argc, argv)
	int	argc;
	char	*argv[];
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	nickname_t		*src_nickname_p;
	nickname_t		*nickname_p;
	nickinfo_t		*nickinfo_p;
	nickinfo_jupe_t		*nickinfo_jupe_p;
	char			*password;

	assert (argc != 0);
	assert (argv != NULL);

	src_nickname_p = Nickname_Find (argv[0]);
	assert (src_nickname_p != NULL);
	assert (src_nickname_p->nickname != NULL);

	nickinfo_p = Nickinfo_Search (argv[3]);
	if (!nickinfo_p)
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"Nickname [\002%s\002] is not registered",
			argv[3]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	nickinfo_jupe_p = NickinfoJupe_Search (nickinfo_p->nickname);
	if (!nickinfo_jupe_p)
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"Nickname [\002%s\0002] is not currently being held by %s",
			argv[3], config->nickserv_p->nickname
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	password = crypt_sha512 (argv[4]);

	if (!strcmp (nickinfo_p->password, password))
	{
		cmd_kill (NULL, nickinfo_jupe_p->nickname, "Removing JUPE");
		NickinfoJupe_Remove (nickinfo_jupe_p->nickname);

		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"Nickname [\002%s\002] has been released",
			argv[3]
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return TRUE;
	}
	else
	{
		cmd_notice (config->nickserv_p->nickname, src_nickname_p->nickname,
			"\002ACCESS DENIED\002"
		);

		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return FALSE;
}
