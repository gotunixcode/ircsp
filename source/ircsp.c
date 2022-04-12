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
#include	<sys/stat.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<ctype.h>
#include	<time.h>
#include	<errno.h>
#include	<netinet/in.h>
#include	<dlfcn.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"log.h"
#include	"ircsp.h"
#include	"rehash.h"
#include	"synch.h"
#include	"misc.h"
#include	"conf.h"
#include	"version.h"
#include	"signals.h"
#include	"alloc.h"
#include	"text.h"
#include	"socket.h"
#include	"channels.h"
#include	"users.h"
#include	"servers.h"
#include	"cmds.h"
#include	"modules.h"
#include	"scheduler.h"

/* Debugging */
#include	"debug.h"

/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 COPYRIGHT HEADER
---------------------------------------------------------------------------------------------------------------------------------
*/
static const char copyright[] = \
	"@(#) IRCSP [Internet Relay Chat Services Package]\n" \
	"@(#) Copyright (C) 1995 - 2019, Justin Ovens and\n" \
	"@(#)                            GOTUNIX NETWORKS\n" \
	"@(#) ALL RIGHTS RESERVED.\n\n";


/*
---------------------------------------------------------------------------------------------------------------------------------
 GLOBAL VARIABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
FILE	*F_MAINLOG;
int	MAINLOG_OPENED;

/*
#if defined (WANT_DEBUG)
	int DEBUG = 1;
#else
	int DEBUG = 0;
#endif
*/


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_cmdline_options ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
static void ircsp_cmdline_options (argc, argv)
	int argc;
	char **argv;
{
	const char *options = "VvhH";
	int opt, loop;

	while ((opt = getopt (argc, argv, options)) != EOF)
	{
		switch (opt)
		{
			case 'V':
			case 'v':
				(void)printf ("---------------------------------------------------------------------------------------------------------------------------------\n");
				for (loop = 0; info_text[loop]; loop++)
				{
					(void)printf ("%s\n", info_text[loop]);
				}

				(void)printf ("---------------------------------------------------------------------------------------------------------------------------------\n");
				(void)printf ("IRCSP VERSION INFORMATION:\n");
				(void)printf ("-- CODE NAME:    %s\n", code_name);
				(void)printf ("--   VERSION:    %s\n", version_number);
				(void)printf ("--     BUILD:    %s\n", version_build);
				(void)printf ("---------------------------------------------------------------------------------------------------------------------------------\n");
				exit (0);
				break;

			case 'H':
			case 'h':
			default:
				(void)printf ("Usage: %s [options] [arguments]\n", argv[0]);
				(void)printf ("-- -h    |    Display this screen\n");
				(void)printf ("-- -v    |    Display version information\n");
				exit (1);
				break;
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_exit ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_exit (int exit_code, char *format, ...)
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	char		buf[256];
	va_list		msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	LOG (main_logfile_p, LOG_INFO,
		"[INFO] - [%s:%d:%s()]: IRCSP EXIT - Exit Code: %d - Reason: %s\n",
		__FILE__, __LINE__, __FUNCTION__,  exit_code, buf
	);


	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	exit (exit_code);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_panic
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_panic (char *format, ...)
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	char		buf[256];
	va_list		msg;

	va_start	(msg, format);
	vsprintf	(buf, format, msg);

	LOG (main_logfile_p, LOG_CRITICAL,
		"[CRIT] - [%s:%d:%s] - PANIC!! - %s\n",
		__FILE__, __LINE__, __FUNCTION__, buf
	);

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);

	ircsp_exit (0, buf);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_file_exists ()
                |
 DESCRIPTION    |    This function is used to check that a specified file exists.
                |
 RETURNS        |    true  - file exists
                |    false - file is missing
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_file_exists (filename)
	char *filename;
{
	struct stat st;
	int i;

	i = stat (filename, &st);
	if (i == 0)
	{
		return 1;
	}
	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_check_access ()
                |
 DESCRIPTION    |    This function is used to check permissions on a specified file or folder
                |
 RETURNS        |    true  - permissions match
                |    false - permissions do not match
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_check_access (path, mode)
	const char *path;
	int mode;
{
	if (!access (path, mode))
	{
		return 1;
	}
	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_write_pidfile ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
static int ircsp_write_pidfile (void)
{
	if ((!ircsp_check_access (VARPATH, R_OK)) || (!ircsp_check_access (VARPATH, W_OK)))
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Unable to read/write to [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}
	else
	{
		FILE *pidFile;

		pidFile = fopen(PIDFILE, "w");
		if (!pidFile)
		{
			ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Unable to write PIDFILE [%s] so we won't fork()\n",
				__FILE__, __LINE__, __FUNCTION__, PIDFILE);

			return 0;
		}
		else
		{
			// Dump out our PIDFILE and then fork ()

			fprintf (pidFile, "%d\n", ircsp->pid);
			fclose (pidFile);

			return 1;
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_init ()
                |
 DESCRIPTION    |    This function will handle our basic startup routines, such as opening log files, loading our configuration
                |    files, databases, initialize all of our lists and pointers to keep them from pointing to micellaneous
                |    garbage.
                |
 RETURNS        |    true  - startup was successfull
                |    false - startup failed
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_init (argc, argv)
	int argc;
	char *argv[];
{
	ircsp_signal_handler ();
	printf ("Initializing MainLog - %s\n", MAINLOG);

	main_logfile_p = logfile_register ("ircsp.log");
	assert (main_logfile_p != NULL);
	main_logfile_p->log_file = fopen (MAINLOG, "a");
	assert (main_logfile_p->log_file != NULL);

	main_logfile_p->log_open = TRUE;



	ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_IRCSP_BUILD),
		__FILE__, __LINE__, __FUNCTION__, version_number, code_name, version_build);


	ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_IRCSP_INIT),
		__FILE__, __LINE__, __FUNCTION__);


	/* Initialize memory for IRCSP structure */
	ircsp = (IRCSP *)ircsp_calloc(1, sizeof(IRCSP));
	if (ircsp != NULL)
	{
		ircsp->argc = argc;
		ircsp->argv = argv;
		ircsp->pid = getpid ();
		ircsp->uid = getuid ();
		ircsp->euid = geteuid ();
		ircsp->uptime = time (NULL);
		ircsp->connected = 0;
		ircsp->initial_burst = 0;
		ircsp->services_online = 0;
		ircsp->rehashing = 0;
		ircsp->quitting = 0;
		ircsp->last_save_cycle = time (NULL);
		ircsp->last_gline_cycle = time (NULL);


		ircsp->last_nickserv_save = time (NULL);
		ircsp->last_nickserv_expire = time (NULL);
		ircsp->last_nickserv_jupe_expire = time (NULL);
		ircsp->last_nickserv_validate = time (NULL);
		ircsp->scheduler_last_run = time (NULL);
	}
	else
	{
		ircsp_error (TRUE, LOG_MESSAGE_OUTOFMEMORY, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			     __FILE__, __LINE__, __FUNCTION__);
	}


	/* Initialize memory for status structure */
	status = (Status *)ircsp_calloc(1, sizeof(Status));
	if (status != NULL)
	{
		status->numUsers = 0;
		status->numServers = 0;
		status->numChannels = 0;
		status->numOpers = 0;
		status->numAdmins = 0;
		status->numHosts = 0;
		status->numGlines = 0;
		status->numNumeric = 0;
	}
	else
	{
		ircsp_error (TRUE, LOG_MESSAGE_OUTOFMEMORY, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			     __FILE__, __LINE__, __FUNCTION__);
	}


	/* Initialize our lists */
	nickname_h 		= NULL;		/* Nickname List */
	nickname_numeric_h	= NULL;		/* NicknameNumeric List */
	operator_h 		= NULL;		/* Operator List */
	channel_h 		= NULL;		/* Channel List */
	hostname_h	 	= NULL;		/* Hostname List */
	server_h		= NULL;		/* Server List */
	scheduler_h		= NULL;		/* Scheduler List */


	/* Initialize memory for our configure structures */
	config = (Config *)ircsp_calloc (1, sizeof (Config));
	if (config != NULL)
	{
		config->database = (struct _config_database *)ircsp_calloc (1, sizeof (struct _config_database));
		if (!config->database)
		{
			ircsp_error (TRUE, LOG_MESSAGE_OUTOFMEMORY, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
				     __FILE__, __LINE__, __FUNCTION__);
		}
	}
	else
	{
		ircsp_error (TRUE, LOG_MESSAGE_OUTOFMEMORY, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			     __FILE__, __LINE__, __FUNCTION__);
	}

	return 1;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    main ()
                |
 DESCRIPTION    |    Lets keep this as short as we can, here we setup our environment, load our config files, connect to the
                |    UPLINK, and then move to our main loop.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int main (argc, argv)
	int argc;
	char *argv[0];
{
	char	inBuf[2048];
	ConfigModule	*p_ConfigModule;


	MAINLOG_OPENED = 0;

	(void)printf ("%s", copyright);
	(void)printf ("IRCSP\n");
	(void)printf ("-  VERSION: %s\n", version_number);
	(void)printf ("-    BUILD: %s\n", version_build);

	/* Parse command line options */
	ircsp_cmdline_options (argc, argv);

	if (ircsp_init (argc, argv))
	{
		if (ircsp_loadConfig ())
		{
			if (!Module_Load_Protocol ())
			{
				ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to load protocol\n",
					__FILE__, __LINE__, __FUNCTION__);

				exit (EXIT_FAILURE);
			}

			if (!Module_Load_Services ())
			{
				ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failed to load service modules\n",
					__FILE__, __LINE__, __FUNCTION__);

				exit (EXIT_FAILURE);
			}

//			uw_loadGlineDB ();
//			uw_loadCommand ();

			if ( (config->fork) && (ircsp_write_pidfile ()) )
			{
				if (fork ())
				{
					exit (0);
				}
			}

			if (!config->nickserv_loaded)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: NickServ is required to function",
					__FILE__, __LINE__, __FUNCTION__
				);

				ircsp_panic ("Startup failed, NickServ not loaded");
			}

/*			if (!config->chanserv_loaded)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: ChanServ is required to function",
					__FILE__, __LINE__, __FUNCTION__);

				ircsp_panic ("Startup failed, ChanServ not loaded");
			}

			if (!config->operserv_loaded)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: OperServ is required to function",
					__FILE__, __LINE__, __FUNCTION__
				);

				ircsp_panic ("Startup failed, OperServ not loaded");
			}
*/

			ircsp_connect ();

			if (ircsp->connected)
			{
				introdouce_server ();
				ircsp_mainloop ();
			}
		}
	}

	/* We should never reach this point */
	ircsp_exit(0,"Program Stop - You have reached the end of the internet, please call back.");
	return 0;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_spawnUW ()
                |
 DESCRIPTION    |    Spawn our operserv nick, add them to the nick list, this will eventually be moved into 2 parts. The main
                |    spawn function will be moved into the protocol source and will be different based on the protocol we are
                |    using, it will be called by the source of the service requesting a nick being created.
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
/*
void ircsp_spawnUW (void)
{
	if (!config->uworld_online)
	{
		nickname_t		*myuserp;
		channel_t		*channelp;
		nickname_channel_t	*chanlistp;
		channel_nickname_t	*nicklistp;
		server_t		*myserverp;
		hostname_t		*hostp;
		nickname_numeric_t	*nnumericp;
		operator_t		*p_operator;

		char *numeric;
		numeric = (char *)ircsp_calloc(1, 10);
		NicknameNumeric_Generate (numeric);
		nnumericp = NicknameNumeric_Add (numeric);

		ircsp_log (F_MAINLOG, "[%s:%d:%s()]:  Spawning service bot [%s / %s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname, numeric);

		myserverp = Server_Find(config->server_name);

		ssprintf(s, "%s N %s 1 0 %s %s %s AAAAAA %s :%s\n",
			myserverp->numeric, config->uworld_nickname,
			config->uworld_username, config->uworld_hostname,
			config->uworld_ircmodes, numeric,
			config->uworld_realname);

		ircsp_log (F_MAINLOG, "[%s:%d:%s()]:  Nickname[%s] Numeric[%s] Online - Adding Nickname\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname, numeric);

		hostp = Hostname_Find (config->uworld_hostname);
		if (!hostp)
		{
			ircsp_log (F_MAINLOG, "[%s:%d:%s()]:  Adding hostname [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, config->uworld_hostname);

			hostp = Hostname_Add (config->uworld_hostname);
		}
		else
		{
			ircsp_log (F_MAINLOG, "[%s:%d:%s()]:  Increasing numHosts\n",
				__FILE__, __LINE__, __FUNCTION__);

			hostp->numHosts++;
		}

		myuserp = Nickname_Add (config->uworld_nickname, config->uworld_username,
				hostp, myserverp, 0);
		myuserp->numeric = (char *)ircsp_calloc (1, strlen (numeric) + 5);
		if (myuserp->numeric)
		{
			strcpy (myuserp->numeric, numeric);
		}

		status->numUsers++;

		ircsp_log (F_MAINLOG, "[%s:%d:%s()]:  Setting flags, joining channels, setting modes\n",
			__FILE__, __LINE__, __FUNCTION__);


		myuserp->flags |= NICK_IS_OPER;
		myuserp->flags |= NICK_IS_SERVICE;

		ssprintf(s, "%s M %s :%s\n", myuserp->numeric,
			config->uworld_nickname,
			config->uworld_ircmodes);



		channelp = Channel_Find (config->operchannel_name);
		if (!channelp)
			channelp = Channel_Add (config->operchannel_name, 0, time(NULL), NULL, 0);

		nicklistp = ChannelNickname_Add(channelp, myuserp);
		chanlistp = NicknameChannel_Add(myuserp, channelp);

		chanlistp->flags |= IS_CHANOP;
		nicklistp->flags |= IS_CHANOP;


		ssprintf(s, "%s J %s\n",
			myuserp->numeric, config->operchannel_name);

		printf ("%s - %s - %s\n", myserverp->numeric, config->operchannel_name, config->operchannel_modes);

		ssprintf(s, "%s M %s %s\n", myserverp->numeric,
			config->operchannel_name, config->operchannel_modes);

		ssprintf(s,
			"%s M %s +o %s\n",
			myserverp->numeric, config->operchannel_name,
			myuserp->numeric);

		ircsp_free(numeric);

		config->uworld_online = TRUE;
	}
}
*/

/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_join_operchannel ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_join_operchannel (void)
{
	nickname_t		*p_nickname;
	nickname_channel_t	*p_nickname_chanlist;
	channel_t		*p_channel;
	channel_nickname_t	*p_channel_nicklist;
	server_t		*p_server;


	ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART), __FILE__, __LINE__, __FUNCTION__);

/*
	p_nickname = Nickname_Find (config->operserv->nickname);
	if (p_nickname != NULL)
	{
		if ((config->operchannel_name != NULL) || (config->operchannel_modes != NULL))
		{
			p_channel = Channel_Find (config->operchannel_name);
			if (p_channel == NULL)
			{
				p_channel = Channel_Add (config->operchannel_name, 0, time (NULL), NULL, 0);
			}

			cmd_join (p_nickname->nickname, p_channel->name);
		}
		else
		{
			ircsp_log (F_MAINLOG, "[%s:%d:%s()]: OPERCHANNEL options are missing.\n",
					__FILE__, __LINE__, __FUNCTION__);
		}
	}
	else
	{
		ircsp_log (F_MAINLOG, "[%s:%d:%s()]: NICKNAME [%s] not found in Nickname_List\n",
				__FILE__, __LINE__, __FUNCTION__, config->operserv->nickname);
	} */


	ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND), __FILE__, __LINE__, __FUNCTION__);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_mainloop ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_mainloop (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t		*p_Module;
	module_callback_t	*p_ModuleCallback;
	scheduler_t		*p_Scheduler;
	char			inBuf [2048];

	while (!ircsp->quitting)
	{
		if (ssread (s, inBuf))
		{
			if ((DEBUG) && (DEBUG_SOCKET))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Socket (READ): %s",
					__FILE__, __LINE__, __FUNCTION__, inBuf
				);
			}


			m_parse (inBuf);
		}

		if (ircsp->initial_burst)
		{
			if (!ircsp->services_online)
			{
				p_Module = module_h;
				while (p_Module)
				{
					if (p_Module->ModuleType & MODULETYPE_SERVICE)
					{
						if ((DEBUG) && (DEBUG_MODULE))
						{
							ircsp_log (F_MAINLOG,
								"[%s:%d:%s()]: Initializing service module [%s]\n",
								__FILE__, __LINE__, __FUNCTION__, p_Module->FileName
							);
						}

						if (p_Module->ServiceNickname)
						{
							ModuleCallback_Call (p_Module, "init_service");
						}
					}

					p_Module = p_Module->next;
				}
//				ircsp_spawnUW ();
				ircsp->services_online = 1;
			}
		}

		if ((time (NULL) - ircsp->scheduler_last_run) > 30)
		{
			if ((DEBUG) && (DEBUG_SCHEDULER))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Running task scheduler\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			p_Scheduler = scheduler_h;
			while (p_Scheduler)
			{
				if ((time (NULL) - p_Scheduler->task_last_run) > p_Scheduler->task_interval)
				{
					if ((DEBUG) && (DEBUG_SCHEDULER))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Running scheduled task [%s]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Scheduler->task_name
						);
					}

					p_Scheduler->func ();
					p_Scheduler->task_last_run = time (NULL);
				}
				p_Scheduler = p_Scheduler->next;
			}
			ircsp->scheduler_last_run = time (NULL);
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}
}
