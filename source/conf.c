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
#include	<sys/types.h>
#include	<sys/time.h>
#include	<ctype.h>
#include	<time.h>
#include	<signal.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"misc.h"
#include	"conf.h"
#include	"alloc.h"
#include	"log.h"
#include	"ircsp.h"
#include	"signals.h"
#include	"text.h"
#include	"users.h"
#include	"modules.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 ConfigOptions
---------------------------------------------------------------------------------------------------------------------------------
*/
struct _ConfigOptions ConfigOptions[] = {
	{	"SERVER",			&ircsp_config_option_server,				},
	{	"SERVER-NUMERIC",		&ircsp_config_option_server_numeric,			},
	{	"UPLINK",			&ircsp_config_option_uplink,				},
	{	"NUMERIC-GENERATOR",		&ircsp_config_option_numeric_generator,			},
	{	"UWORLD",			&ircsp_config_option_uworld,				},
	{	"OPERCHANNEL",			&ircsp_config_option_operchannel,			},
	{	"DATABASE",			&ircsp_config_option_database_connection,		},
	{	"OPERATOR-NOTICE",		&ircsp_config_option_operator_notice,			},
	{	"MODULE",			&ConfigOption_Module,					},
};


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ConfigModule_Add ()
                |
 DESCRIPTION    |    This function will add Modules to our ConfigModule linked list, these will be used later
                |    to load/initialize our modules.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
ConfigModule *ConfigModule_Add (ModuleName)
	char		*ModuleName;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigModule	*p_ConfigModule;

	assert (ModuleName != NULL);

	p_ConfigModule = (ConfigModule *)ircsp_calloc (1, sizeof (ConfigModule));
	p_ConfigModule->ModuleName = (char *)ircsp_calloc (1, strlen (ModuleName) + 5);

	if ((!p_ConfigModule) || (!p_ConfigModule->ModuleName))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for ConfigModule_p\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return NULL;
	}

	strcpy (p_ConfigModule->ModuleName, ModuleName);

	LOG (main_logfile_p, LOG_INFO,
		"[INFO] - [%s:%d:%s()]:  Module [%s] added at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__,
		p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
	);

	if (!config->ConfigModuleHead)
	{
		config->ConfigModuleHead = p_ConfigModule;
		p_ConfigModule->next = NULL;
	}
	else
	{
		p_ConfigModule->next = config->ConfigModuleHead;
		config->ConfigModuleHead = p_ConfigModule;
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return p_ConfigModule;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ConfigModule_Find ()
                |
 DESCRIPTION    |    This function will search ConfigModule for a specific Module name.
                |
 RETURNS        |    Returns pointer to module if found, otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
ConfigModule *ConfigModule_Find (ModuleName)
	char		*ModuleName;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigModule	*p_ConfigModule;

	assert (ModuleName != NULL);

	for (p_ConfigModule = config->ConfigModuleHead; p_ConfigModule; p_ConfigModule = p_ConfigModule->next)
	{
		if (!strcasecmp (p_ConfigModule->ModuleName, ModuleName))
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Module [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return p_ConfigModule;
		}
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ConfigModule_FindProtocol ()
                |
 DESCRIPTION    |    This function will search ConfigModule for a protocol module, we need to load at least one and only
                |    allow one to be loaded.
                |
 RETURNS        |    Returns pointer to module if found, otherwise NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
ConfigModule *ConfigModule_FindProtocol (void)
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigModule	*p_ConfigModule;

	for (p_ConfigModule = config->ConfigModuleHead; p_ConfigModule; p_ConfigModule = p_ConfigModule->next)
	{
		if (p_ConfigModule->ModuleType & MODULETYPE_PROTOCOL)
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Protocol Module [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return p_ConfigModule;
		}
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ConfigModule_Del ()
                |
 DESCRIPTION    |    This function will delete a module from ConfigModule, not sure if we will ever need this. Maybe eventually
                |    we will use this to remove a module then rewrite the config file for a rehash()
                |
 RETURNS        |    Returns nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void ConfigModule_Del (ModuleName)
	char		*ModuleName;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigModule	*p_ConfigModule;
	ConfigModule	*p_Delete;

	assert (ModuleName != NULL);

	for (p_ConfigModule = config->ConfigModuleHead; p_ConfigModule; p_ConfigModule = p_ConfigModule->next)
	{
		if (!strcasecmp (p_ConfigModule->ModuleName, ModuleName))
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Deleting Module [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
			);

			if (config->ConfigModuleHead == p_ConfigModule)
			{
				config->ConfigModuleHead = p_ConfigModule->next;
			}
			else
			{
				for (p_Delete = config->ConfigModuleHead; (p_Delete->next != p_ConfigModule) && p_Delete; p_Delete = p_Delete->next);
				p_Delete->next = p_ConfigModule->next;
			}

			ircsp_free (p_ConfigModule->ModuleName);
			p_ConfigModule->ModuleType = 0;
			ircsp_free (p_ConfigModule);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return ;
		}
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ConfigServiceModule_add ()
                   |
    DESCRIPTION    |    This function will create a new record on our ConfigServiceModule list to store information about
                   |    service bots (nickname, username, hostname, etc).
                   |
    RETURNS        |   On success we return a pointer to the record, NULL on failure.
---------------------------------------------------------------------------------------------------------------------------------
*/
ConfigServiceModule *ConfigServiceModule_add (p_Module, nickname, username, hostname, ircmodes, realname)
	struct _module	*p_Module;
	char		*nickname;
	char		*username;
	char		*hostname;
	char		*ircmodes;
	char		*realname;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigServiceModule		*p_ConfigServiceModule;

	assert (nickname != NULL);
	assert (username != NULL);
	assert (hostname != NULL);
	assert (ircmodes != NULL);
	assert (realname != NULL);

	p_ConfigServiceModule = ConfigServiceModule_find (nickname);
	if (!p_ConfigServiceModule)
	{
		p_ConfigServiceModule = (ConfigServiceModule *)ircsp_calloc (1, sizeof (ConfigServiceModule));
		p_ConfigServiceModule->nickname = (char *)ircsp_calloc (1, strlen (nickname) + 5);
		p_ConfigServiceModule->username = (char *)ircsp_calloc (1, strlen (username) + 5);
		p_ConfigServiceModule->hostname = (char *)ircsp_calloc (1, strlen (hostname) + 5);
		p_ConfigServiceModule->ircmodes = (char *)ircsp_calloc (1, strlen (ircmodes) + 5);
		p_ConfigServiceModule->realname = (char *)ircsp_calloc (1, strlen (realname) + 5);

		if ((!p_ConfigServiceModule) || (!p_ConfigServiceModule->nickname) ||
			(!p_ConfigServiceModule->username) || (!p_ConfigServiceModule->hostname) ||
			(!p_ConfigServiceModule->ircmodes) || (!p_ConfigServiceModule->realname))
		{
			LOG (main_logfile_p, LOG_CRITICAL,
				"[CRIT] - [%s:%d:%s()]: Failed to allocate memory for ConfigServiceModule_p\n",
				__FILE__, __LINE__, __FUNCTION__
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return NULL;
		}



		strcpy (p_ConfigServiceModule->nickname, nickname);
		strcpy (p_ConfigServiceModule->username, username);
		strcpy (p_ConfigServiceModule->hostname, hostname);
		strcpy (p_ConfigServiceModule->ircmodes, ircmodes);
		strcpy (p_ConfigServiceModule->realname, realname);
		p_ConfigServiceModule->p_Module = p_Module;
		p_ConfigServiceModule->p_Nickname = NULL;
		p_ConfigServiceModule->flags = 0L;


		LOG (main_logfile_p, LOG_INFO,
			"[INFO] - [%s:%d:%s()]:  Service Module Configuration options [%s (%s@%s) added for module [%s]\n",
			__FILE__, __LINE__, __FUNCTION__,
			p_ConfigServiceModule->nickname, p_ConfigServiceModule->username,
			p_ConfigServiceModule->hostname, p_ConfigServiceModule->p_Module->FileName
		);

		if (!ConfigServiceModuleHead)
		{
			ConfigServiceModuleHead = p_ConfigServiceModule;
			p_ConfigServiceModule->next = NULL;
		}
		else
		{
			p_ConfigServiceModule->next = ConfigServiceModuleHead;
			ConfigServiceModuleHead = p_ConfigServiceModule;
		}

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return p_ConfigServiceModule;
	}
	else
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - Duplicate config found, returning original\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return p_ConfigServiceModule;
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ConfigServiceModule_find ()
                   |
    DESCRIPTION    |    This function will find a record on our ConfigServiceModule list
                   |
    RETURNS        |   On success we return a pointer to the record, NULL on failure.
---------------------------------------------------------------------------------------------------------------------------------
*/
ConfigServiceModule *ConfigServiceModule_find (nickname)
	char		*nickname;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigServiceModule		*p_ConfigServiceModule;

	assert (nickname != NULL);

	p_ConfigServiceModule = ConfigServiceModuleHead;
	while (p_ConfigServiceModule)
	{
		if (!strcasecmp (p_ConfigServiceModule->nickname, nickname))
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Configuration for Service [%s] Module [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__,
				p_ConfigServiceModule->nickname, p_ConfigServiceModule->p_Module->FileName,
				p_ConfigServiceModule->nickname
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return p_ConfigServiceModule;
		}

		p_ConfigServiceModule = p_ConfigServiceModule->next;
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_loadConfig ()
                |
 DESCRIPTION    |    Load IRCSP config file and push it to our parser
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_loadConfig (void)
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	FILE		*f_config;
	int		i_line = 0;
	int		i_option = 0;
	char		c_read[256];
	char		*argv[2];

	if (!ircsp_check_access (CONFPATH, R_OK))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]: [%s] Missing or incorrect permissons\n",
			__FILE__, __LINE__, __FUNCTION__, CONFPATH
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	if (!ircsp_file_exists (CONFFILE))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Config file [%s] is missing\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	f_config = fopen (CONFFILE, "r");
	if (!f_config)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]: Failed to read from config file [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	while (!feof (f_config))
	{
		if (fgets (c_read, 256, f_config))
		{
			i_line++;

			if (is_comment (c_read))
			{
				continue;
			}

			if (is_blank (c_read))
			{
				continue;
			}


			argv[0] = strtok (c_read, " ");
			argv[1] = strtok (NULL, "\n");

			for (i_option = 0; i_option <= CONFIG_OPTIONS_COUNT; i_option++)
			{
				if (!strcasecmp (argv[0], ConfigOptions[i_option].command))
				{
					if (ConfigOptions[i_option].func (argv, i_line))
					{
						LOG (main_logfile_p, LOG_INFO,
							"[INFO] - [%s:%d:%s()]: Pushing function for ConfigOption [%s]\n",
							__FILE__, __LINE__, __FUNCTION__,
							ConfigOptions[i_option].command
						);
					}
				}
			}


			/* Legacy options */
			if (!strcasecmp (argv[0], "DENY-MESSAGE"))
			{
				config->deny_message = (char *)ircsp_calloc(1, strlen (argv[1]) + 5);
				if (!config->deny_message)
				{
					LOG (main_logfile_p, LOG_CRITICAL,
						"[CRIT] - [%s:%d:%s()]: Failed to allocate memory for config->deny_message\n",
						__FILE__, __LINE__, __FUNCTION__
					);

					LOG (main_logfile_p, LOG_FUNC,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);

					return FALSE;
				}

				strcpy (config->deny_message, argv[1]);
			}

			else if (!strcasecmp (argv[0], "FORK"))
			{
				config->fork = atoi (argv[1]);
			}

			else if (!strcasecmp (argv[0], "ADMIN-DB"))
			{
				config->admin_db = (char *)ircsp_calloc (1, strlen (argv[1]) + 5);
				if (!config->admin_db)
				{
					LOG (main_logfile_p, LOG_CRITICAL,
						"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->admin_db\n",
						__FILE__, __LINE__, __FUNCTION__
					);

					LOG (main_logfile_p, LOG_FUNC,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);

					return FALSE;
				}

				strcpy (config->admin_db, argv[1]);
			}

			else if (!strcasecmp (argv[0], "GLINE-DB"))
			{
				config->gline_db = (char *)ircsp_calloc (1, strlen (argv[1]) + 5);
				if (!config->gline_db)
				{
					LOG (main_logfile_p, LOG_CRITICAL,
						"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->gline_db\n",
						__FILE__, __LINE__, __FUNCTION__
					);

					LOG (main_logfile_p, LOG_FUNC,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);

					return FALSE;
				}

				strcpy (config->gline_db, argv[1]);
			}

			else if (!strcasecmp (argv[0], "UWORLD-CMD-DB"))
			{
				config->uw_cmddb = (char *)ircsp_calloc (1, strlen (argv[1]) + 5);
				if (!config->uw_cmddb)
				{
					LOG (main_logfile_p, LOG_CRITICAL,
						"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->uw_cmddb\n",
						__FILE__, __LINE__, __FUNCTION__
					);

					LOG (main_logfile_p, LOG_FUNC,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);

					return FALSE;
				}

				strcpy (config->uw_cmddb, argv[1]);
			}
		}
	}

	if ( (!config->server_name) || (!config->server_comment) )
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Config options for \"SERVER\" missing from config file [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE
		);

		return FALSE;
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_server ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "SERVER" option from our config file
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_server (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	int	i_count = 0;
	char	*c_server;
	char	*add_server[2];

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	c_server = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_server)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_server\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_server, argv[1]);
	i_count = tokenize (c_server, ':');
	if (i_count < 2)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_server[0] = strtok (c_server, " ");		/* Server name */
	add_server[1] = strtok (NULL, "\n");		/* Server comment */

	config->server_name = (char *)ircsp_calloc (1, strlen (add_server[0]) + 10);
	config->server_comment = (char *)ircsp_calloc (1, strlen (add_server[1]) + 10);

	if ((!config->server_name) || (!config->server_comment))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->server\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->server_name, add_server[0]);
	strcpy (config->server_comment, add_server[1]);

	ircsp_free (c_server);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_server_numeric ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "SERVER-NUMERIC" option from our config file, this is only
                |    required for IRCU based networks.
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_server_numeric (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	char	c_numeric [4];
	char	*p_temp;

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	p_temp = intobase64 (c_numeric, atoi (argv[1]), 2);
	config->server_numeric = (char *)ircsp_calloc (1, strlen (p_temp) + 5);
	if (!config->server_numeric)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->server_numeric\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->server_numeric, p_temp);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_numeric_generator ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "NUMERIC-GENERATOR" option from our config file, this is only
                |    required for IRCU based networks.
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_numeric_generator (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	int	i_count = 0;
	char	*c_numeric;
	char	*add_numeric[3];

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	c_numeric = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_numeric)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_numeric\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_numeric, argv[1]);
	i_count = (tokenize (c_numeric, ':'));
	if (i_count < 3)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_numeric[0] = strtok (c_numeric, " ");
	add_numeric[1] = strtok (NULL, " ");
	add_numeric[2] = strtok (NULL, "\n");

	config->xPos = atoi (add_numeric[0]);
	config->yPos = atoi (add_numeric[1]);
	config->yyPos = atoi (add_numeric[2]);

	ircsp_free (c_numeric);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_uplink ()
                |
 DESCRIPTION    |    This function will parse the UPLINK option from the config file.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_uplink (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	int	i_count;
	char	*c_uplink;
	char	*add_uplink[3];

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	c_uplink = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_uplink)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_uplink\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_uplink, argv[1]);
	i_count = tokenize (c_uplink, ':');
	if (i_count < 3)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_uplink[0] = strtok (c_uplink, " ");		// Uplink
	add_uplink[1] = strtok (NULL, " ");		// Port
	add_uplink[2] = strtok (NULL, "\n");		// Password

	config->uplink = (char *)ircsp_calloc (1, strlen (add_uplink[0]) + 10);
	config->uplink_pass = (char *)ircsp_calloc (1, strlen (add_uplink[2]) + 10);

	if ((!config->uplink) || (!config->uplink_pass))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->uplink\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->uplink, add_uplink[0]);
	config->uplink_port = atoi (add_uplink[1]);
	strcpy (config->uplink_pass, add_uplink[2]);

	ircsp_free (c_uplink);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_operchannel ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "OPERCHANNEL" option from our config file
                |
                |        OPERCHANNEL #Channel:Modes
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_operchannel (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	int	i_count = 0;
	char	*c_operchannel;
	char	*add_operchannel[2];

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	c_operchannel = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_operchannel)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_operchannel\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_operchannel, argv[1]);
	i_count = tokenize (c_operchannel, ':');
	if (i_count < 2)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_operchannel[0] = strtok (c_operchannel, " ");	// Channel
	add_operchannel[1] = strtok (NULL, "\n");		// Channel modes

	config->operchannel_name = (char *)ircsp_calloc (1, strlen (add_operchannel[0]) + 10);
	config->operchannel_modes = (char *)ircsp_calloc (1, strlen (add_operchannel[1]) + 10);

	if ((!config->operchannel_name) || (!config->operchannel_modes))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->operchannel\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->operchannel_name, add_operchannel[0]);
	strcpy (config->operchannel_modes, add_operchannel[1]);

	ircsp_free (c_operchannel);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_uworld ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "UWORLD" option from our config file.
                |
                |        UWORLD nickname:username:hostname:modes:enabled:realname
                |
                |    LEAGACY! This will be removed once we write our new OperServ
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_uworld (argv, line)
	char	*argv[];
	int	line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	int	i_count = 0;
	char	*c_uworld;
	char	*add_service[7];

	c_uworld = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_uworld)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_uworld\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_uworld, argv[1]);
	i_count = tokenize (c_uworld, ':');
	if (i_count < 6)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_service[0] = strtok (c_uworld, " ");		/* Nickname */
	add_service[1] = strtok (NULL, " ");			/* Username */
	add_service[2] = strtok (NULL, " ");			/* Hostname */
	add_service[3] = strtok (NULL, " ");			/* Modes */
	add_service[4] = strtok (NULL, " ");			/* Enabled */
	add_service[5] = strtok (NULL, "\n");			/* Real Name */

	config->uworld_nickname = (char *)ircsp_calloc (1, strlen (add_service[0]) + 10);
	config->uworld_username = (char *)ircsp_calloc (1, strlen (add_service[1]) + 10);
	config->uworld_hostname = (char *)ircsp_calloc (1, strlen (add_service[2]) + 10);
	config->uworld_ircmodes = (char *)ircsp_calloc (1, strlen (add_service[3]) + 10);
	config->uworld_realname = (char *)ircsp_calloc (1, strlen (add_service[5]) + 10);

	if ((!config->uworld_nickname) || (!config->uworld_username) || (!config->uworld_hostname) ||
		(!config->uworld_ircmodes) || (!config->uworld_realname))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->uworld\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->uworld_nickname, add_service[0]);
	strcpy (config->uworld_username, add_service[1]);
	strcpy (config->uworld_hostname, add_service[2]);
	strcpy (config->uworld_ircmodes, add_service[3]);
	strcpy (config->uworld_realname, add_service[5]);
	config->uworld_enable = atoi (add_service[4]);

	ircsp_free (c_uworld);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_database_connection ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "DATABASE" option from our config file
                |
                |        DATABASE server:port:password:database
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_database_connection (argv, line)
	char		*argv[];
	int		line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	char		*c_database;
	char		*add_database[4];
	int		i_count = 0;

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);
	assert (config->database != NULL);

	c_database = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_database)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_database\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_database, argv[1]);
	i_count = tokenize (c_database, ':');
	if (i_count < 4)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax erro rin config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE, line
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	add_database[0] = strtok (c_database, " ");		/* Database Host */
	add_database[1] = strtok (NULL, " ");			/* Database Username */
	add_database[2] = strtok (NULL, " ");			/* Database Password */
	add_database[3] = strtok (NULL, "\n");			/* Database Name */

	config->database->server = (char *)ircsp_calloc (1, strlen (add_database[0]) + 5);
	config->database->username = (char *)ircsp_calloc (1, strlen (add_database[1]) + 5);
	config->database->password = (char *)ircsp_calloc (1, strlen (add_database[2]) + 5);
	config->database->dbname = (char *)ircsp_calloc (1, strlen (add_database[3]) + 5);

	if ((!config->database->server) || (!config->database->username) || (!config->database->password) ||
		(!config->database->dbname))
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for config->database\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (config->database->server, add_database[0]);
	strcpy (config->database->username, add_database[1]);
	strcpy (config->database->password, add_database[2]);
	strcpy (config->database->dbname, add_database[3]);

	ircsp_free (c_database);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_config_option_operator_notice ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "OPERNOTICE" option from our config file
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_config_option_operator_notice (argv, line)
	char		*argv[];
	int		line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	if (!strcasecmp (argv[1], "CHANNEL"))
	{
		config->operator_notice |= OPERNOTICE_CHANNEL;
	}
	else if (!strcasecmp (argv[1], "DIRECT"))
	{
		config->operator_notice |= OPERNOTICE_DIRECT;
	}
	else if (!strcasecmp (argv[1], "WALLOPS"))
	{
		config->operator_notice |= OPERNOTICE_WALLOPS;
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ConfigOption_Module ()
                |
 DESCRIPTION    |    This function will handle parsing/checking the "MODULE" option from our config file
                |
                |        Example: MODULE PROTOCOL:libundernet.so
                |                 MODULE SERVICE:NickServ:libnickserv.so
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ConfigOption_Module (argv, line)
	char		*argv[];
	int		line;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	ConfigModule	*p_ConfigModule;
	char		*c_Module;
	char		*AddModule[2];
	int		i_Count;

	assert (argv != NULL);
	assert (line != 0);
	assert (config != NULL);

	c_Module = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
	if (!c_Module)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Failed to allocate memory for c_module\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	strcpy (c_Module, argv[1]);
	i_Count = tokenize (c_Module, ':');
	if (i_Count < 2)
	{
		LOG (main_logfile_p, LOG_CRITICAL,
			"[CRIT] - [%s:%d:%s()]:  Syntax error in config file [%s] on line [%d]\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}

	AddModule[0] = strtok (c_Module, " ");		/* Module Type (PROTOCOL, SERVICE, CORE, etc) */
	AddModule[1] = strtok (NULL,     " ");		/* Module Name (File name) */


	p_ConfigModule = ConfigModule_Find (AddModule[1]);
	if (p_ConfigModule)
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - [%s:%d:%s()]:  Duplicate Module [%s] found at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__,
			p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return FALSE;
	}


	if (!strcasecmp (AddModule[0], "PROTOCOL"))
	{
		p_ConfigModule = ConfigModule_FindProtocol ();
		if (p_ConfigModule)
		{
			LOG (main_logfile_p, LOG_WARNING,
				"[WARN] - [%s:%d:%s()]:  Protocol [%s] already loaded, skipping [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, p_ConfigModule->ModuleName, AddModule[1]
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return FALSE;
		}

		p_ConfigModule = ConfigModule_Add (AddModule[1]);
		assert (p_ConfigModule != NULL);

		LOG (main_logfile_p, LOG_INFO,
			"[INFO] - [%s:%d:%s()]:  Protocol Module [%s] loaded at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__,
			p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
		);

		p_ConfigModule->ModuleType |= MODULETYPE_PROTOCOL;
	}

	else if (!strcasecmp (AddModule[0], "SERVICE"))
	{
		p_ConfigModule = ConfigModule_Add (AddModule[1]);
		assert (p_ConfigModule != NULL);

		LOG (main_logfile_p, LOG_INFO,
			"[INFO] - [%s:%d:%s()]:  Service Module [%s] loaded at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__,
			p_ConfigModule->ModuleName, p_ConfigModule->ModuleName
		);

		p_ConfigModule->ModuleType |= MODULETYPE_SERVICE;
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	return TRUE;
}
