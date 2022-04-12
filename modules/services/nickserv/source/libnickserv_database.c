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
#include	<stdbool.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>
#include	<assert.h>
#include	<mariadb/mysql.h>

#if defined (LINUX)
	#include	<crypt.h>
#endif


/* IRCSP Core Includes */
#include	"modules.h"
#include	"ircsp.h"
#include	"log.h"
#include	"text.h"
#include	"alloc.h"
#include	"conf.h"
#include	"mysql.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/* NickServ Module Includes */
#include	"libnickserv.h"
#include	"libnickserv_admin.h"
#include	"libnickserv_database.h"
#include	"libnickserv_nickinfo.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_db_load ()
                   |
    DESCRIPTION    |    This function will load the NickServ database from MYSQL into memory.
                   |
    RETURNS        |    TRUE  - If successful
                   |    FALSE - If failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int
nickinfo_db_load (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	MYSQL			*db_connection;
	MYSQL_RES		*db_result;
	MYSQL_ROW		db_row;
	char			*c_query;
	bool			errors = FALSE;


	/*
	-------------------------------------------------------------------------------------------------------------------------
	    First lets create the NickServ database (if it doesn't already exist).
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if ( (ircsp_mysql_create_table (NSDB_NICKSERV, NICKSERV_TABLE)) &&
		(ircsp_mysql_create_table (NSDB_NICKSERV_ACCESS, NICKSERV_ACCESS_TABLE)) )
	{
		db_connection = ircsp_mysql_connect ();
		if (!db_connection)
		{
			ircsp_log (F_MAINLOG, "[%s:%d:%s()]: Failure to connect to database server\n",
				__FILE__, __LINE__, __FUNCTION__
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return FALSE;
		}


		c_query = (char *)ircsp_calloc (1, 8192);
		if (!c_query)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
				__FILE__, __LINE__, __FUNCTION__
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
		-----------------------------------------------------------------------------------------------------------------
		    Ok Here we are going to start to load the main NickServ registeration database before wee move on to the
		    access list database.
		-----------------------------------------------------------------------------------------------------------------
		*/
		sprintf (c_query, "SELECT * FROM %s", NSDB_NICKSERV);
		if (mysql_query (db_connection, c_query))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return FALSE;
		}


		db_result = mysql_store_result (db_connection);
		if (!db_result)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_store_result() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return FALSE;
		}


		while ((db_row = mysql_fetch_row (db_result)))
		{
			nickinfo_p = Nickinfo_Create (db_row[1]);
			if (!nickinfo_p)
			{
				errors = TRUE;
			}
			else
			{
				nickinfo_p->password = (char *)ircsp_calloc (1, strlen (db_row[2]) + 5);
				nickinfo_p->last_login = (char *)ircsp_calloc (1, strlen (db_row[3]) + 5);


				if ( ( !nickinfo_p->password ) || ( !nickinfo_p->last_login ) )
				{
					errors = TRUE;
				}
				else
				{
					strcpy (nickinfo_p->password, db_row[2]);
					strcpy (nickinfo_p->last_login, db_row[3]);
					nickinfo_p->last_seen		= atoi (db_row[4]);
					nickinfo_p->registered		= atoi (db_row[5]);
					nickinfo_p->flags		= atoi (db_row[6]);
				}
			}
		}


		/*
		-----------------------------------------------------------------------------------------------------------------
		    We will now attempt to load the NickServ ACCESS list table from our database.
		-----------------------------------------------------------------------------------------------------------------
		*/
		sprintf (c_query, "SELECT * FROM %s", NSDB_NICKSERV_ACCESS);
		if (mysql_query (db_connection, c_query))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query () - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return FALSE;
		}


		db_result = mysql_store_result (db_connection);
		if (!db_result)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_store_result() - %s\n",
				__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
			);


			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}


			return FALSE;
		}


		while ((db_row = mysql_fetch_row (db_result)))
		{
			nickinfo_p = Nickinfo_Search (db_row[1]);
			if (!nickinfo_p)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: No NickServ registered nickname found for hostmask [%s (%s)]\n",
					__FILE__, __LINE__, __FUNCTION__, db_row[1], db_row[2]
				);


				errors = TRUE;
			}
			else
			{
				nickinfo_access_p = NickinfoAccess_Create (nickinfo_p, db_row[2]);
				if (!nickinfo_access_p)
				{
					errors = TRUE;
				}
			}
		}


		ircsp_free (c_query);
		ircsp_mysql_disconnect (db_connection);
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	if (errors)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    nickinfo_save_db ()
                   |
    DESCRIPTION    |    This function will save the NickServ database from memory to MYSQL
                   |
    RETURNS        |    TRUE  - If successful
                   |    FALSE - If failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int
nickinfo_db_save (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	nickinfo_t		*nickinfo_p;
	nickinfo_access_t	*nickinfo_access_p;
	MYSQL			*db_connection;
	char			*c_insert;


	assert (config->server_name != NULL);


	c_insert = (char *)ircsp_calloc (1, 8192);
	if (!c_insert)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
			__FILE__, __LINE__, __FUNCTION__
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
	    First we are going to attempt to backup the main nickserv table
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (ircsp_mysql_backup_table (NSDB_NICKSERV))
	{
		if (ircsp_mysql_create_table (NSDB_NICKSERV, NICKSERV_TABLE))
		{
			db_connection = ircsp_mysql_connect ();
			if (!db_connection)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to mysql server\n",
					__FILE__, __LINE__, __FUNCTION__
				);


				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}


				ircsp_free (c_insert);
				return FALSE;
			}


			nickinfo_p = nickinfo_h;
			while (nickinfo_p)
			{
				sprintf (c_insert,
					"INSERT INTO %s (nickname,password,last_login,last_seen,registered,flags) VALUES (\"%s\",\"%s\", \"%s\", %lu, %lu, %d)",
                                        NSDB_NICKSERV, nickinfo_p->nickname, nickinfo_p->password, nickinfo_p->last_login,
					nickinfo_p->last_seen, nickinfo_p->registered, nickinfo_p->flags
				);


				if (mysql_query (db_connection, c_insert))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: mysql_query() - %s\n",
						__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
					);
				}



				nickinfo_p = nickinfo_p->next;
			}


			ircsp_mysql_disconnect (db_connection);
		}
	}


	/*
	-------------------------------------------------------------------------------------------------------------------------
	    Now we will save the NickServ Access list table
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (ircsp_mysql_backup_table (NSDB_NICKSERV_ACCESS))
	{
		if (ircsp_mysql_create_table (NSDB_NICKSERV_ACCESS, NICKSERV_ACCESS_TABLE))
		{
			db_connection = ircsp_mysql_connect ();
			if (!db_connection)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to mysql server\n",
					__FILE__, __LINE__, __FUNCTION__
				);


				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}


				ircsp_free (c_insert);
				return FALSE;
			}


			nickinfo_p = nickinfo_h;
			while (nickinfo_p)
			{
				nickinfo_access_p = nickinfo_p->nickinfo_access_h;
				while (nickinfo_access_p)
				{
					sprintf (c_insert,
						"INSERT INTO %s (nickname,hostmask) VALUES (\"%s\", \"%s\")",
						NSDB_NICKSERV_ACCESS, nickinfo_p->nickname, nickinfo_access_p->hostmask
					);


					if (mysql_query (db_connection, c_insert))
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: mysql_query() - %s\n",
							__FILE__, __LINE__, __FUNCTION__, mysql_error (db_connection)
						);
					}


					nickinfo_access_p = nickinfo_access_p->next;
				}


				nickinfo_p = nickinfo_p->next;
			}


			ircsp_mysql_disconnect (db_connection);
		}
	}


	ircsp_free (c_insert);

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
    FUNCTION       |    services_admin_db_load ()
                   |
    DESCRIPTION    |    This function will load the Services Admin database from MYSQL into memory.
                   |
    RETURNS        |    TRUE  - If successful
                   |    FALSE - If failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int
services_admin_db_load (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	services_admin_t		*sa_p;
	MYSQL				*db_connection;
	MYSQL_RES			*db_result;
	MYSQL_ROW			db_row;
	char				*c_query;
	bool				errors = FALSE;


	/*
	-------------------------------------------------------------------------------------------------------------------------
	    First lets create the Services Admin database if it doesn't already exist.
	-------------------------------------------------------------------------------------------------------------------------
	*/
	if (ircsp_mysql_create_table (NSDB_SERVICES_ADMIN, SERVICES_ADMIN_TABLE))
	{
		db_connection = ircsp_mysql_connect ();
		if (!db_connection)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Failed to connect to database server\n",
				__FILE__, __LINE__, __FUNCTION__
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		c_query = (char *)ircsp_calloc (1, 8192);
		if (!c_query)
		{
			ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_OUTOFMEMORY),
				__FILE__, __LINE__, __FUNCTION__
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}

		sprintf (c_query, "SELECT * FROM %s", NSDB_SERVICES_ADMIN);
		if (mysql_query (db_connection, c_query))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_query () - %s\n",
				__FILE__, __LINE__, __FUNCTION__,
				mysql_error (db_connection)
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			ircsp_free (c_query);
			ircsp_mysql_disconnect (db_connection);
			return FALSE;
		}

		db_result = mysql_store_result (db_connection);
		if (!db_result)
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: mysql_store_result() - %s\n",
				__FILE__, __LINE__, __FUNCTION__,
				mysql_error (db_connection)
			);

			if (DEBUG)
			{
				ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			ircsp_free (c_query);
			ircsp_mysql_disconnect (db_connection);
			return FALSE;
		}

		while ((db_row = mysql_fetch_row (db_result)))
		{
			sa_p = ServicesAdmin_Create (db_row[1]);
			if (!sa_p)
			{
				 ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to add Services Admin [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, db_row[1]
				);

				errors = TRUE;
			}
			else
			{
				sa_p->flags = atoi (db_row[2]);
			}
		}
	}
	else
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Failed to create Services Admin database [%s]",
			__FILE__, __LINE__, __FUNCTION__, NSDB_SERVICES_ADMIN
		);

		return FALSE;
	}

	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	ircsp_free (c_query);
	ircsp_mysql_disconnect (db_connection);

	if (errors)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    services_admin_save_db ()
                   |
    DESCRIPTION    |    This function will save the Services Admin database from memory to MYSQL
                   |
    RETURNS        |    TRUE  - If successful
                   |    FALSE - If failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int
services_admin_db_save (void)
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	services_admin_t		*sa_p;
	MYSQL				*db_connection;

	if (ircsp_mysql_backup_table (NSDB_SERVICES_ADMIN))
	{
		if (ircsp_mysql_create_table (NSDB_SERVICES_ADMIN, SERVICES_ADMIN_TABLE))
		{
			db_connection = ircsp_mysql_connect ();
			if (!db_connection)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to connect to database server\n",
					__FILE__, __LINE__, __FUNCTION__
				);

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return FALSE;
			}

			sa_p = services_admin_h;
			while (sa_p)
			{
				ircsp_mysql_insert (NSDB_SERVICES_ADMIN,
					"0,\"%s\",%d",
					sa_p->nickname, sa_p->flags
				);

				sa_p = sa_p->next;
			}

			ircsp_mysql_disconnect (db_connection);
		}
		else
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Failed to create new Services Admin Table [%s]\n",
				__FILE__, __LINE__, __FUNCTION__,
				NSDB_SERVICES_ADMIN
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
	else
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Failed to backup Services Admin Table [%s -> %sBACKUP]",
			__FILE__, __LINE__, __FUNCTION__,
			NSDB_SERVICES_ADMIN, NSDB_SERVICES_ADMIN
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
