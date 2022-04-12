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

#if defined (LINUX)
	#include	<crypt.h>
#endif


/* IRCSP Core Includes */
#include	"ircsp.h"
#include	"text.h"
#include	"conf.h"
#include	"log.h"
#include	"alloc.h"


/* Debugging */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    is_comment ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int is_comment (string)
	const char *string;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	if ((string[0] == ';') || (string[0] == '#'))
	{
		if ((DEBUG) && (DEBUG_FUNC))
		{
			ircsp_log (F_MAINLOG,
				get_log_message (LOG_MESSAGE_FUNCEND),
				__FILE__, __LINE__, __FUNCTION__
			);
		}

		return TRUE;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return FALSE;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    is_blank ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int is_blank (string)
	const char *string;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	int i = 0;
	while (string[i])
	{
		if ((string[i] != ' ') && (string[i] != '\n') && (string[i] != '\t'))
		{
			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return FALSE;
		}
		i++;
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
 FUNCTION       |    lrange ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *lrange (tokenlist, from)
	char *tokenlist;
	int from;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	static char      token[1024];
	unsigned int	    curtok=1, tokenpos=0, curpos=0;

	while (curtok < from)
	{
		if (tokenlist[curpos++] == ' ')
		{
			curtok++;
		}
		else if ((tokenlist[curpos] == '\n') || (tokenlist[curpos] == '\0'))
		{
			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return NULL;
		}
	}

	while ((tokenlist[curpos] != '\n') &&
	       (tokenlist[curpos] != '\0') &&
	       (tokenlist[curpos] != '\r'))
	{
		token[tokenpos++] = tokenlist[curpos++];
	}

	token[tokenpos] = '\0';

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return (char *)token;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    translate ()
                |
 DESCRIPTION    |    Translate certain control characters into fields from help files.
                |
                |        ^U    Nickname of OperService
                |        ^N    Nickname of Nickservice
                |        ^C    Nickname of ChannelService
                |        ^B    BOLD
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *translate(inBuf, outBuf)
	char *inBuf;
	char *outBuf;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	int inBufPos = -1, outBufPos = -1;
	int pos = -1;
	int cmd;


	while(inBuf[ ++inBufPos ] != '\0')
	{
		if(inBuf[ inBufPos ] == '^')
		{
			switch(inBuf[ ++inBufPos ])
			{
				case 'U':	/* UWorld Nickname */
					while(config->uworld_nickname[++pos] != '\0')
					{
						outBuf[++outBufPos] = config->uworld_nickname[pos];
					}

					break;

				case 'B':
					outBuf[ ++outBufPos ] = '\002';

					break;
			}
		}
		else
		{
			outBuf[ ++outBufPos ] = inBuf[ inBufPos ];
		}
	}

	outBuf[ outBufPos ] = '\0';

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return outBuf;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    token ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *token (tokenlist, index)
	char *tokenlist;
	int index;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	static char token[512];
	unsigned int curtok=1, curpos=0, tokenpos=0;

	while (curtok < index)
	{
		if (tokenlist[curpos++] == ' ')
		{
			curtok++;
		}
		if (tokenlist[curpos] == '\n')
		{
			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return NULL;
		}
	}

	while ((tokenlist[curpos] != ' ')  && (tokenlist[curpos] != '\r') &&
		(tokenlist[curpos] != '\n') && (tokenlist[curpos] != '\0'))
	{
		token[tokenpos++] = tokenlist[curpos++];
	}

	token[tokenpos] = '\0';

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return (char *)token;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    tokenize ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int tokenize (inlist, sep)
	char *inlist;
	char sep;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	char outlist[ 1024 ];
	int  pos=0, num=1;

	while ((inlist[pos] != '\n') && (inlist[pos] != '\0'))
	{
		if (inlist[pos] == sep)
		{
			outlist[pos] = ' ';
			++num;
		}
		else
		{
			outlist[pos] = inlist[pos];
		}

		++pos;
	}

	outlist[pos] = '\0';

	strncpy(inlist, outlist, strlen(inlist));

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return num;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    strtoupper ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *strtoupper(string)
	char *string;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	int i=0;
	while(i < strlen(string))
	{
		string[i] = toupper(string[i++]);
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return string;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    numtokens ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int numtokens(line)
	char *line;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	int pos=0, num_tok = 1;
	for (;pos<strlen(line); ++pos)
	{
		if (line[pos] == ' ')
		{
			num_tok += 1;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return num_tok;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    strcascmp ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
int strcascmp (in1, in2)
	char *in1;
	char *in2;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	unsigned int pos = 0;

	while((in1[pos] != '\n') && (in1[pos] != '\0'))
	{
		if (tolower(in1[pos]) != tolower(in2[pos]))
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
		++pos;
	}
	if (strlen(in1) == strlen(in2))
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
	else
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
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    chop ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *chop (chopme)
	char *chopme;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	int x = strlen(chopme) - 1;

	if((chopme[ x ] == '\n') || (chopme[ x ] == '\r'))
	{
		chopme[ x ] = '\0';
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return chopme;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    crypt_sha256 ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *crypt_sha256 (string)
	char		*string;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	char		saltString[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
	char		salt[21];
	char		*hash;

	srandom (time (NULL));
	salt[0] = '$';
	salt[1] = '5';
	salt[2] = '$';
	salt[3] = string[0];
	salt[4] = string[1];
	salt[5] = '$';
	salt[6] = '\0';

	hash = crypt (string, salt);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return hash;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    crypt_sha512 ()
                |
 DESCRIPTION    |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
char *crypt_sha512 (string)
	char		*string;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	char		saltString[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
	char		salt[21];
	char		*hash;

	srandom (time (NULL));
	salt[0] = '$';
	salt[1] = '6';
	salt[2] = '$';
	salt[3] = string[0];
	salt[4] = string[1];
	salt[5] = '$';
	salt[6] = '\0';

	hash = crypt (string, salt);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return hash;
}
