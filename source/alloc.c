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
#include	<errno.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"log.h"
#include	"alloc.h"


/* Debbing */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_malloc ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void		*ircsp_malloc					(size)
	long		size;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	void		*data;

	if (size == 0)
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - [%s:%d:%s()]:  Attempted to allocate a NULL pointer\n",
			__FILE__, __LINE__, __FUNCTION__
		);


		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return NULL;
	}
	else
	{
		data = malloc (size);
		if (data == NULL)
		{
			if (errno == ENOMEM)
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					get_log_message (LOG_MESSAGE_OUTOFMEMORY),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
			else
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					"[CRIT] - [%s:%d:%s()]:  Unknown error while trying to allocate memory",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return NULL;
		}
		else
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]: Allocated [%ld] bytes at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, size, data
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return data;
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_calloc ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void		*ircsp_calloc					(els, elsize)
	long		els;
	long		elsize;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	void		*data;

	if ((els == 0) || (elsize == 0))
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - [%s:%d:%s()]:  Attempted to allocate a NULL pointer\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		LOG (main_logfile_p, LOG_FUNC,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);

		return NULL;
	}
	else
	{
		data = calloc (elsize, els);
		if (data == NULL)
		{
			if (errno == ENOMEM)
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					get_log_message (LOG_MESSAGE_OUTOFMEMORY),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
			else
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					"[CRIT] - [%s:%d:%s()]:  Unknown error while trying to allocate memory\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return NULL;
		}
		else
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Allocated [%ld] bytes at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, (elsize * els), data
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return data;
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_realloc ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void		*ircsp_realloc					(ptr, size)
	void		*ptr;
	long		size;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	void		*data;

	if (size == 0)
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - [%s:%d:%s()]:  Attempted to realloc() a NULL pointer\n",
			__FILE__, __LINE__, __FUNCTION__
		);

		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return NULL;
	}
	else
	{
		data = realloc (ptr, size);
		if (data == NULL)
		{
			if (errno == ENOMEM)
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					get_log_message (LOG_MESSAGE_OUTOFMEMORY),
					__FILE__, __LINE__, __FUNCTION__
				);
			}
			else
			{
				LOG (main_logfile_p, LOG_CRITICAL,
					"[CRIT] - [%s:%d:%s()]:  Unknown error when attempting to realloc memory\n",
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return NULL;
		}
		else
		{
			LOG (main_logfile_p, LOG_INFO,
				"[INFO] - [%s:%d:%s()]:  Reallocated to [%ld] bytes at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, size, data
			);

			LOG (main_logfile_p, LOG_FUNC,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);

			return data;
		}
	}
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_free ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void		ircsp_free					(ptr)
	void		*ptr;
{
	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCSTART),
		__FILE__, __LINE__, __FUNCTION__
	);

	if (ptr == NULL)
	{
		LOG (main_logfile_p, LOG_WARNING,
			"[WARN] - [%s:%d:%s()]:  Attempted to free() a NULL pointer\n",
			__FILE__, __LINE__, __FUNCTION__
		);
	}
	else
	{
		LOG (main_logfile_p, LOG_INFO,
			"[INFO] - [%s:%d:%s()]:  Released memory at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, ptr
		);

		free (ptr);
	}

	LOG (main_logfile_p, LOG_FUNC,
		get_log_message (LOG_MESSAGE_FUNCEND),
		__FILE__, __LINE__, __FUNCTION__
	);
}
