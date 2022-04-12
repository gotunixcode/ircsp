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
#include	<string.h>
#include	<time.h>
#include	<sys/time.h>
#include	<signal.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"log.h"
#include	"ircsp.h"
#include	"text.h"
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 VARIABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
FILE	*F_MAINLOG;
int	MAINLOG_OPENED;


/*
---------------------------------------------------------------------------------------------------------------------------------
 LOG MESSAGES
---------------------------------------------------------------------------------------------------------------------------------
*/
static char *log_messages[] =
{
	/* 0000 */	NULL,
	/* 0001 */	"[CRIT] - [%s:%d:%s()]:  OUT OF MEMORY\n",
	/* 0002 */	NULL,
	/* 0003 */	NULL,
	/* 0004 */	"[INFO] - [%s:%d:%s()]:  Function starting\n",
	/* 0005 */	"[INFO] - [%s:%d:%s()]:  Function ending\n",
	/* 0006 */	"[INFO] - [%s:%d:%s()]:  Function ending with ERRORS\n",
	/* 0007 */	NULL,
	/* 0008 */	NULL,
	/* 0009 */	NULL,
	/* 0010 */	"[%s:%d:%s()]: IRCSP [Version: %s] [Code name: %s] [BUILD: %s]\n",
	/* 0011 */	"[%s:%d:%s()]: Initializing IRCSP\n",
	/* 0012 */	NULL,
	/* 0013 */	NULL,
	/* 0014 */	NULL,
	/* 0015 */	"[%s:%d:%s()]: [%s] POINTER IS NULL!\n",
	/* 0016 */	NULL,
	/* 0017 */	NULL,
	/* 0018 */	NULL,
	/* 0019 */	NULL,
	/* 0020 */	"[%s:%d:%s()]: Failed to allocate memory for [%s]\n",
	/* 0021 */	NULL,
	/* 0022 */	NULL,
	/* 0023 */	NULL,
	/* 0024 */	NULL,
	/* 0025 */	NULL,
	/* 0026 */	NULL,
	/* 0027 */	NULL,
	/* 0028 */	NULL,
	/* 0029 */	NULL,
	/* 0030 */	NULL,
	/* 0031 */	NULL,
	/* 0032 */	NULL,
	/* 0033 */	NULL,
	/* 0034 */	NULL,
	/* 0035 */	NULL,
	/* 0036 */	NULL,
	/* 0037 */	NULL,
	/* 0038 */	NULL,
	/* 0039 */	NULL,
	/* 0040 */	"[%s:%d:%s()]:  Nickname_List:  Failed to locate nickname [%s]\n",
	/* 0041 */	NULL,
	/* 0042 */	NULL,
	/* 0043 */	NULL,
	/* 0044 */	NULL,
	/* 0045 */	NULL,
	/* 0046 */	NULL,
	/* 0047 */	NULL,
	/* 0048 */	NULL,
	/* 0049 */	NULL,
	/* 0050 */	NULL,
	/* 0051 */	NULL,
	/* 0052 */	NULL,
	/* 0053 */	NULL,
	/* 0054 */	NULL,
	/* 0055 */	NULL,
	/* 0056 */	NULL,
	/* 0057 */	NULL,
	/* 0058 */	NULL,
	/* 0059 */	NULL,
	/* 0060 */	NULL,
	/* 0061 */	NULL,
	/* 0062 */	NULL,
	/* 0063 */	NULL,
	/* 0064 */	NULL,
	/* 0065 */	NULL,
	/* 0066 */	NULL,
	/* 0067 */	NULL,
	/* 0068 */	NULL,
	/* 0069 */	NULL,
	/* 0070 */	NULL,
	/* 0071 */	NULL,
	/* 0072 */	NULL,
	/* 0073 */	NULL,
	/* 0074 */	NULL,
	/* 0075 */	NULL,
	/* 0076 */	NULL,
	/* 0077 */	NULL,
	/* 0078 */	NULL,
	/* 0079 */	NULL,
	/* 0080 */	NULL,
	/* 0081 */	"[%s:%d:%s()]:  BURST [B] Channel [%s]:  Processing BURST\n",
	/* 0082 */	"[%s:%d:%s()]:  BURST [B] Channel [%s]:  Processing continuation BURST\n",
	/* 0083 */	"[%s:%d:%s()]:  BURST [B] Channel [%s]:  Processing USERLIST\n",
	/* 0084 */	"[%s:%d:%s()]:  BURST [B] Channel [%s]:  Processing BANLIST\n",
};


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    logfile_register ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
logfile_t
*logfile_register (log_name)
	char		*log_name;
{
	logfile_t	*logfile_p;

	assert (log_name != NULL);

	logfile_p = (logfile_t *)calloc (1, sizeof (logfile_t));
	logfile_p->log_name = (char *)calloc (1, strlen (log_name) + 5);

	assert (logfile_p != NULL);
	assert (logfile_p->log_name != NULL);

	strcpy (logfile_p->log_name, log_name);
	logfile_p->log_file = NULL;
	logfile_p->log_open = 0;

	if (!logfile_h)
	{
		logfile_h = logfile_p;
		logfile_p->next = NULL;
	}
	else
	{
		logfile_p->next = logfile_h;
		logfile_h = logfile_p;
	}

	return logfile_p;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    get_log_message ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *get_log_message (numeric)
	int numeric;
{
	if ((numeric < 0 || numeric > LOG_MESSAGE_COUNT) || !log_messages[numeric])
	{
		return ("LOG MESSAGE NOT FOUND\n");
	}
	else
	{
		return (log_messages [numeric]);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    LOG ()
                   |
    DESCRIPTION    |
                   |
    RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void
LOG (logfile_t *logfile_p, int level, char *format, ...)
{
	char		buf [256];
	char		entry [256];
	va_list		msg;
	time_t		logtime;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	if (level <= LOG_LEVEL)
	{
		if ((!logfile_p) || (!logfile_p->log_open))
		{
			(void)printf ("%s", buf);
		}
		else
		{
			time (&logtime);
			sprintf (entry, "[%s] - %s", lrange (ctime (&logtime), 1), buf);
			fprintf (logfile_p->log_file, "%s", entry);
			fflush (logfile_p->log_file);
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_log ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_log (FILE *logfile, char *format, ...)
{
	char buf[256], entry[256];
	va_list msg;
	time_t logtime;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	if (!MAINLOG_OPENED)
	{
		(void)printf ("%s", buf);
	}
	else
	{
		time (&logtime);
		sprintf (entry, "[%s] - %s", lrange (ctime (&logtime), 1), buf);
		fprintf (logfile, "%s", entry);
		fflush (logfile);
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_error ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_error (int fatal, int error_code, char *format, ...)
{
	char buf[256], entry[256];
	va_list msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	if (fatal)
	{
		ircsp_log (F_MAINLOG, "[FATAL ERROR] - %s", buf);
		raise (SIGUSR1);
	}
	else
	{
		ircsp_log (F_MAINLOG, "[ERROR] - %s", buf);
	}
}
