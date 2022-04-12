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
#include	<stdarg.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<signal.h>


/* IRCSP Core Includes */
#include	"log.h"
#include	"signals.h"
#include	"ircsp.h"
#include	"rehash.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_signal_handler ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_signal_handler (void)
{
	struct sigaction sa;

	/* SIGHUP */
	sa.sa_handler = ircsp_signal_sighup;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGHUP, &sa, (struct sigaction *) 0);

	/* SIGUSR1 */
	sa.sa_handler = ircsp_signal_sigusr1;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGUSR1, &sa, (struct sigaction *) 0);

	/* SIGSEGV */
	sa.sa_handler = ircsp_signal_sigsegv;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGSEGV, &sa, (struct sigaction *) 0);

	/* SIGINT */
	sa.sa_handler = ircsp_signal_sigint;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGINT, &sa, (struct sigaction *) 0);

	/* SIGTERM */
	sa.sa_handler = ircsp_signal_sigterm;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGTERM, &sa, (struct sigaction *) 0);

	return TRUE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_signal_sighup ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_signal_sighup (num)
	int num;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_log (F_MAINLOG,
		"[%s:%d:%s()]: Signal HUP received, rehashing",
		__FILE__, __LINE__, __FUNCTION__
	);

	ircsp_rehash ();

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
 FUNCTION       |    ircsp_signal_sigusr1 ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_signal_sigusr1 (num)
	int num;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Signal USR1 received, time to panic\n",
		__FILE__, __LINE__, __FUNCTION__);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	exit (1);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_signal_sigsegv ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_signal_sigsegv (num)
	int num;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Signal SEGV received, time to panic\n",
		__FILE__, __LINE__, __FUNCTION__);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_panic ("SIGSEGV Received");
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_signal_sigint ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_signal_sigint (num)
	int num;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Signal INT received, beginning shutdown proceedures\n",
		__FILE__, __LINE__, __FUNCTION__);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_exit (0, "SIGINT");
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_signal_sigterm ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_signal_sigterm (num)
	int num;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Signal TERM received, beginning shutdown proceedures\n",
		__FILE__, __LINE__, __FUNCTION__);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_exit (0, "SIGTERM");
}
