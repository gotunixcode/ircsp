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
#include	<netinet/in.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"alloc.h"
#include	"match.h"
#include	"log.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    match ()
                |
 DESCRIPTION    |    Long yes, but it is iterative versus recursive.  Recursive would mean multiple function calls which costs
                |    CPU time and memory. Overall, this method is faster than the recursive match, though the source is longer.
                |
 RETURNS        |    0 if match
                |    1 if no match
---------------------------------------------------------------------------------------------------------------------------------
*/
int match(mask, name)
	const char	*mask;
	const char	*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	const char	*m = mask;
	const char	*n = name;
	const char	*ma = mask;
	const char	*na = name;
	int		wild = 0;
	int		q = 0;

	while (1)
	{
		if (*m == '*')
		{
			while (*m == '*')
			{
				m++;
			}
			wild = 1;
			ma = m;
			na = n;
		}

		if (!*m)
		{
			if (!*n)
			{
				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return 0;
			}

			for (m--; (m > mask) && (*m == '?'); m--);
			if ((*m == '*') && (m > mask) && (m[-1] != '\\'))
			{
				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return 0;
			}

			if (!wild)
			{
				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return 1;
			}

			m = ma;
			n = ++na;
		}
		else if (!*n)
		{
			while (*m == '*')
			{
				m++;
			}

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return (*m != 0);
		}

		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
		{
			m++;
			q = 1;
		}
		else
		{
			q = 0;
		}

		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q))
		{
			if (!wild)
			{
				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return 1;
			}

			m = ma;
			n = ++na;
		}
		else
		{
			if (*m)
			{
				m++;
			}

			if (*n)
			{
				n++;
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
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    match_wild ()
                |
 DESCRIPTION    |
                |
                |
 RETURNS        |    0 if match
                |    1 if no match
---------------------------------------------------------------------------------------------------------------------------------
*/
int match_wild(const char *pattern, const char *str)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	char c;
	const char *s;

	/*
	-------------------------------------------------------------------------------------------------------------------------
	This will eventully terminate: either by *pattern == 0, or by a trailing '*'
	-------------------------------------------------------------------------------------------------------------------------
	*/

	for (;;)
	{
		switch (c = *pattern++)
		{
			case 0:
				if (!*str)
				{
					if ((DEBUG) && (DEBUG_FUNC))
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return 1;
				}

				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return 0;

			case '?':
				++str;
				break;

			case '*':
				if (!*pattern)
				{
					if ((DEBUG) && (DEBUG_FUNC))
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return 1;	/* trailing '*' matches everything else */
				}

				s = str;
				while (*s)
				{
					if (*s == *pattern && match_wild(pattern, s))
					{
						if ((DEBUG) && (DEBUG_FUNC))
						{
							ircsp_log (F_MAINLOG,
								get_log_message (LOG_MESSAGE_FUNCEND),
								__FILE__, __LINE__, __FUNCTION__
							);
						}

						return 1;
					}

					++s;
				}
				break;

			default:
				if (*str++ != c)
				{
					if ((DEBUG) && (DEBUG_FUNC))
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}

					return 0;
				}

				break;
		}
	}
}
