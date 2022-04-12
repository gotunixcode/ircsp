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


/* Stdandard Includes */
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<mariadb/mysql.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"signals.h"
#include	"ircsp.h"
#include	"log.h"
#include	"alloc.h"
#include	"mysql.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"



/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_mysql_connect ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void *ircsp_mysql_connect (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*db_connection;

	if ((DEBUG) && (DEBUG_MYSQL))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Connecting to mysql server [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->database->server
		);
	}

	db_connection = mysql_init (NULL);
	if (!db_connection)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: mysql_init() failed\n",
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

	if (mysql_real_connect (db_connection, config->database->server, config->database->username,
		config->database->password, config->database->dbname, 0, NULL, 0) == NULL)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: mysql_real_connect() error %s\n",
					__FILE__, __LINE__, __FUNCTION__,
					mysql_error (db_connection)
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

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return db_connection;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_mysql_disconnect ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_mysql_disconnect (db_connection)
	MYSQL		*db_connection;
{
	if (DEBUG)
	{
		if (DEBUG_FUNC)
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		if (DEBUG_MYSQL)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Disconnecting from mysql server\n",
				__FILE__, __LINE__, __FUNCTION__
			);
		}

	}

	mysql_close (db_connection);

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
 FUNCTION       |    ircsp_mysql_backup_table ()
                |
 DESCRIPTION    |
                |
 RETURNS        |    TRUE  = Success
                |    FALSE = Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_mysql_backup_table (table)
	char		*table;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*connection;
	char		*c_query;

	connection = ircsp_mysql_connect ();

	c_query = (char *)ircsp_calloc(1, 1024);
	if (!c_query)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for c_query\n",
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

	sprintf (c_query, "DROP TABLE IF EXISTS %s%s", table, "BACKUP");
	if (mysql_query (connection, c_query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (connection)
			);
		}

		ircsp_mysql_disconnect (connection);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}


	sprintf (c_query, "RENAME TABLE %s to %s%s", table, table, "BACKUP");
	if (mysql_query (connection, c_query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (connection)
			);
		}

		ircsp_mysql_disconnect (connection);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	ircsp_free (c_query);

	ircsp_mysql_disconnect (connection);

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
 FUNCTION       |    ircsp_mysql_create_table ()
                |
 DESCRIPTION    |
                |
 RETURNS        |    TRUE  = Success
                |    FALSE = Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_mysql_create_table (table, data)
	char		*table;
	char		*data;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*connection;
	char		*c_query;

	connection = ircsp_mysql_connect ();

	c_query = (char *)ircsp_calloc(1, 8192);
	if (!c_query)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for c_query\n",
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

	sprintf (c_query, "CREATE TABLE IF NOT EXISTS %s %s", table, data);
	if (mysql_query (connection, c_query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (connection)
			);
		}

		ircsp_mysql_disconnect (connection);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	ircsp_free (c_query);
	ircsp_mysql_disconnect (connection);

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
 FUNCTION       |    ircsp_mysql_insert ()
                |
 DESCRIPTION    |
                |
 RETURNS        |    TRUE  = Success
                |    FALSE = Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_mysql_insert (char *table, char *format, ...)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*db_connection;
	char		buf[256];
	char		*query;
	va_list		msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	db_connection = ircsp_mysql_connect ();
	if (!db_connection)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to database server\n",
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

	query = ircsp_calloc (1, 8192);
	if (!query)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for query\n",
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

	sprintf (query, "INSERT INTO %s VALUES (%s)", table, buf);
	if (mysql_query (db_connection, query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);
		}

		ircsp_free (query);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
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
 FUNCTION       |    ircsp_mysql_update ()
                |
 DESCRIPTION    |
                |
 RETURNS        |    TRUE  = Success
                |    FALSE = Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_mysql_update (char *table, char *format, ...)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*db_connection;
	char		buf[256];
	char		*query;
	va_list		msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	db_connection = ircsp_mysql_connect ();
	if (!db_connection)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to database server\n",
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

	query = ircsp_calloc (1, 8192);
	if (!query)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for query\n",
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

	sprintf (query, "UPDATE %s %s", table, buf);
	if (mysql_query (db_connection, query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);
		}

		ircsp_free (query);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	ircsp_free (query);

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
 FUNCTION       |    ircsp_mysql_delete ()
                |
 DESCRIPTION    |
                |
 RETURNS        |    TRUE  = Success
                |    FALSE = Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ircsp_mysql_delete (char *table, char *format, ...)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	MYSQL		*db_connection;
	char		buf[256];
	char		*query;
	va_list		msg;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	db_connection = ircsp_mysql_connect ();
	if (!db_connection)
	{
		if (DEBUG)
		{
			if (DEBUG_MYSQL)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to database server\n",
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

	query = ircsp_calloc (1, 8192);
	if (!query)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for query\n",
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

	sprintf (query, "DELETE FROM %s WHERE %s", table, buf);
	if (mysql_query (db_connection, query))
	{
		if ((DEBUG) && (DEBUG_MYSQL))
		{
			ircsp_log (F_MAINLOG,
				"[%s:$d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);
		}

		ircsp_free (query);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return FALSE;
	}

	ircsp_free (query);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return TRUE;
}
