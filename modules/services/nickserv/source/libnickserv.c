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
#include	<stdbool.h>


/* IRCSP Core Includes */
#include	"modules.h"
#include	"ircsp.h"
#include	"log.h"
#include	"text.h"
#include	"alloc.h"
#include	"conf.h"
#include	"servers.h"
#include	"users.h"
#include	"channels.h"
#include	"socket.h"
#include	"scheduler.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/* NickServ Module Includes */
#include	"libnickserv.h"
#include	"libnickserv_admin.h"
#include	"libnickserv_database.h"
#include	"libnickserv_nickinfo.h"


char ModuleVersion[] = "1.0.0";
char ModuleName[] = "Nickname Service";
char ModuleType[] = "Services";



/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    ModuleConfig_load ()
                   |
    DESCRIPTION    |    This function will handle loading our configuration options for this module.
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
static int ModuleConfig_load (p_Module)
	struct _module	*p_Module;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	ConfigServiceModule	*p_ConfigServiceModule;
	services_root_t		*services_root_p;
	FILE			*f_config;
	int			i_line;
	int			i_count;
	char			*c_service;
	char			c_read [256];
	char			*argv[8];
	bool			errors = FALSE;


	if (!ircsp_check_access (CONFPATH, R_OK))
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		    This should have been caught when we loaded our main configuration options, but better safe than random
		    SEGFAULTS.
		-----------------------------------------------------------------------------------------------------------------
		*/
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to read from CONFPATH [%s] check permissions\n",
			__FILE__, __LINE__, __FUNCTION__, CONFPATH
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return FALSE;
	}


	if ( (!ircsp_check_access (CONFFILE, R_OK)) && (!ircsp_file_exists (CONFFILE)) )
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to read from CONFFILE or it does not exist\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


		return FALSE;
	}


	f_config = fopen (CONFFILE, "r");
	if (!f_config)
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to open config file [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, CONFFILE
		);


		if (DEBUG)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}


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


			argv[0] = strtok (c_read, " ");					/* Config Option */

			if (!strcasecmp (argv[0], "SERVICE-NICKNAME"))
			{
				argv[1] = strtok (NULL, "\n");

				c_service = (char *)ircsp_calloc (1, strlen (argv[1]) + 20);
				assert (c_service != NULL);
				strcpy (c_service, argv[1]);

				i_count = tokenize (c_service, ':');
				if (i_count < 7)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Syntax error in config file [%s] on line [%d]\n",
						__FILE__, __LINE__, __FUNCTION__, CONFFILE, i_line
					);

					if (DEBUG)
					{
						ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return FALSE;
				}

				argv[1] = strtok (c_service, " ");		/* Service Type */
				argv[2] = strtok (NULL, " ");			/* Nickname */
				argv[3] = strtok (NULL, " ");			/* Username */
				argv[4] = strtok (NULL, " ");			/* Hostname */
				argv[5] = strtok (NULL, " ");			/* Modes */
				argv[6] = strtok (NULL, " ");			/* Enabled Flag */
				argv[7] = strtok (NULL, "\n");			/* Realname */

				if (!strcasecmp (argv[1], "NICKSERV"))
				{
					p_ConfigServiceModule = ConfigServiceModule_add (
						p_Module, argv[2], argv[3], argv[4], argv[5], argv[7]
					);

					assert (p_ConfigServiceModule != NULL);

					p_Module->ServiceNickname = p_ConfigServiceModule->nickname;

					ircsp_free (c_service);
				}
			}

			else if (!strcasecmp (argv[0], "SERVICES-ROOT"))
			{
				argv[1] = strtok (NULL, "\n");

				services_root_p = ServicesRoot_Create (chop(argv[1]));
				assert (services_root_p != NULL);
				assert (services_root_p->nickname != NULL);
			}
		}
	}

	fclose (f_config);
	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    init_service ()
                   |
    DESCRIPTION    |    This function will initialize our service
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void init_service (p_Module)
	struct _module *p_Module;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickname_t		*nickname_p;

	ConfigServiceModule	*p_ConfigServiceModule;
	char			*options[5];


	ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Running for [%s] [%s]\n", __FILE__, __LINE__, __FUNCTION__, p_Module->FileName, p_Module->ServiceNickname);
	p_ConfigServiceModule = ConfigServiceModule_find (p_Module->ServiceNickname);
	assert (p_ConfigServiceModule != NULL);


	if (!(p_ConfigServiceModule->flags & SERVICEMODULE_ONLINE))
	{
		options[0] = p_ConfigServiceModule->nickname;
		options[1] = p_ConfigServiceModule->username;
		options[2] = p_ConfigServiceModule->hostname;
		options[3] = p_ConfigServiceModule->ircmodes;
		options[4] = p_ConfigServiceModule->realname;


		introdouce_service (options);


		p_ConfigServiceModule->flags |= SERVICEMODULE_ONLINE;
		nickname_p = Nickname_Find (p_ConfigServiceModule->nickname);
		assert (nickname_p != NULL);

		config->nickserv_p = nickname_p;
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
    FUNCTION       |    init_module ()
                   |
    DESCRIPTION    |    This function will initialize our module
                   |
    RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void init_module (p_Module)
	struct _module		*p_Module;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	scheduler_t		*scheduler_p;


	if (!ModuleConfig_load (p_Module))
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to load Module [%s] configuration options from [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_Module->FileName, CONFFILE
		);


		exit (EXIT_FAILURE);
	}

	this_module_p		= p_Module;
	p_Module->ModuleType	|= MODULETYPE_SERVICE;
	p_Module->ServiceType	|= SERVICETYPE_NICKSERV;


	/* Initialize Linked list Heads */
	nickinfo_h		= NULL;
	nickinfo_jupe_h		= NULL;


	if (!nickinfo_db_load ())
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to load NickServ database\n",
			__FILE__, __LINE__, __FUNCTION__
		);


		ircsp_panic ("Failure to load NickServ Database!");
	}

	if (!services_admin_db_load ())
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Failed to load Services Admin database\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		ircsp_panic ("Failure to load Services Admin database!");
	}


	scheduler_p = Scheduler_add ("nickinfo_db_save", NICKSERV_SAVE_CYCLE, nickinfo_db_save);
	scheduler_p = Scheduler_add ("services_admin_db_save", SERVICES_ADMIN_SAVE_CYCLE, services_admin_db_save);
	scheduler_p = Scheduler_add ("nickinfo_expire", NICKSERV_EXPIRE_CYCLE, nickinfo_expire);
	scheduler_p = Scheduler_add ("nickinfo_validate", NICKSERV_VALIDATE_CYCLE, nickinfo_validate);
	scheduler_p = Scheduler_add ("nickinfo_jupe_expire", NICKSERV_JUPE_EXPIRE_CYCLE, nickinfo_jupe_expire);


	ModuleCallback_Register (p_Module, "init_service");
	ModuleCallback_Register	(p_Module, "m_privmsg");
	ModuleCallback_Register (p_Module, "m_nick_new");
	ModuleCallback_Register (p_Module, "m_nick_change");
	ModuleCallback_Register (p_Module, "m_quit");
	ModuleCallback_Register (p_Module, "m_kill");
	ModuleCallback_Register (p_Module, "services_root_unregister");
	ModuleCallback_Register (p_Module, "services_admin_unregister");
	ModuleCallback_Register (p_Module, "nickinfo_unregister");


	config->nickserv_loaded = TRUE;

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}
}
