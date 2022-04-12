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
#include	<stdint.h>
#include	<stdarg.h>
#include	<unistd.h>
#include	<string.h>
#include	<assert.h>
#include	<time.h>


/* IRCSP Core Includes */
#include	"conf.h"
#include	"ircsp.h"
#include	"alloc.h"
#include	"text.h"
#include	"log.h"
#include	"users.h"
#include	"channels.h"


/* IRCSP Module Includes */
#include	"libundernet.h"
#include	"libundernet_parser.h"
#include	"libundernet_m_burst.h"


/* Debug */
#include	"debug.h"


/* Compiler Generated Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    libundernet_m_burst ()
                |
 DESCRIPTION    |    Undernet Protocol (m_burst)
                |    This function will handle Server Message: BURST [B]
                |
                |    Parse incoming channels (BURSTs). This gets semi-complicated becase in large channels there may be the
                |    need to have multiple BURSTs (since there is a limit of 512 bytes per line.
                |
                |    General format for a burst
                |
                |        [N] BURST <#channel> <ts> [+modes] [lim] [key] <users> [:%ban...]
                |
                |    If it is a continuation burst, the first parameter are the same, but it will NOT contain any modes and may
                |    contain only BANS, USERS or a combination of both.
                |
                |
                |        [ircsp.c:725:ircsp_mainloop()]: Socket (READ): AB B #fuck 1575597426
                |
                |        __BUFFER__: AD B #Shit 1 +smti ABABT,ABABS,ACAAO,ACAAN,ACAAM,ADAA2,ADAA1,ADAA0,ABABR,ADAAz,ABABQ,ABABP,AHAAD,
                |                    AGAAG,ADAAy,ABABO,ABABN,ABABM,ABABL,ABABK,ADAAx,ADAAw,ABABJ,ADAAv,ABABI,ABABH,ACAAL,ABABG,AGAAF,
                |                    ACAAK,ABABF,ABABE,ABABD,AGAAE,AGAAD,AGAAC,AHAAC,ACAAJ,ACAAI,ACAAH,ADAAu,ADAAt,AHAAB,ABABC,ABABB,
                |                    ABABA,AGAAB,AGAAA,AHAAA,ABAA],ABAA[,ACAAG,ACAAF,ACAAE,ABAA9,ACAAD,ACAAC,ABAA8,ABAA7,ACAAB,ACAAA,
                |                    ADAAr,ADAAq,ADAAp,ADAAo,ADAAn,ABAA4,ABAA3,ABAA0,ABAAx,ABAAy,ABAAv,ABAAu,ABAAr,ABAAp,ABAAo,ABAAn,
                |                    ABAAm,ABAAl,ABAAk,ABAAj
                |
                |
                |        __BUFFER__: AD B #Shit 1 ABAAi,ABAAg,ADAAg,ABAAR:o,ADAAa :%*!*@*.fuck.com40 *!*@*.fuck.com39 *!*@*.fuck.com38
                |                    *!*@*.fuck.com37 *!*@*.fuck.com36 *!*@*.fuck.com35 *!*@*.fuck.com34 *!*@*.fuck.com33 *!*@*.fuck.com32
                |                    *!*@*.fuck.com31 *!*@*.fuck.com30 *!*@*.fuck.com29 *!*@*.fuck.com28 *!*@*.fuck.com27 *!*@*.fuck.com26
                |                    *!*@*.fuck.com25 *!*@*.fuck.com24 *!*@*.fuck.com23 *!*@*.fuck.com22 *!*@*.fuck.com21 *!*@*.fuck.com20
                |                    *!*@*.fuck.com19 *!*@*.fuck.com18 *!*@*.fuck.com17 *!*@*.fuck.com16 *!*@*.fuck.com15 *!*@*.fuck.com14
                |
                |
                |        __BUFFER__: AD B #Shit 1 :%*!*@*.fuck.com13 *!*@*.fuck.com12 *!*@*.fuck.com11 *!*@*.fuck.com10 *!*@*.fuck.com9
                |                    *!*@*.fuck.com8 *!*@*.fuck.com7 *!*@*.fuck.com6 *!*@*.fuck.com5 *!*@*.fuck.com4 *!*@*.fuck.com3
                |                    *!*@*.fuck.com2 *!*@*.fuck.com1
                |
                |
                |
                |    UNCHANGED
                |        argv[0] = Server
                |        argv[1] = Channel
                |
                |    INITIAL BURST
                |        argv[2] = Modes
                |        argv[3] = Userlist
                |        argv[4] = Banlist
                |        key     = Channels Key (optional)
                |        limit   = Channel Limit (optional)
                |
                |    CONTINUATION BURST
                |        argv[2] = Userlist (Or banlist)
                |        argv[3] = Banlist
                |
                |
 INPUTS         |    sBuf   = Socket Buffer
                |
 RETURNS        |    TRUE
---------------------------------------------------------------------------------------------------------------------------------
*/
int libundernet_m_burst (sBuf)
	char		*sBuf;
{
	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__);
	}


	channel_t		*p_Channel;
	channel_nickname_t	*p_ChannelNickname;
	nickname_t		*p_Nickname;
	nickname_channel_t	*p_NicknameChannel;
	char			*c_burst;				/* Copy of sBuf incase we need it unmodified. */
	char			*argv[9];
	char			*ban = NULL;
	char			*key = NULL;
	char			*userlist;
	char			*modes;
	char			*banlist = NULL;
	char			user[4];
	char			uflags[3];
	char			burst[7];
	char			*c_Ban[512];
	int			i_BanCount = 0;
	int			limit = 0;
	int			count = 0;
	int			flags = 0;
	int			last_mode = 0;
	int			last_flag = 0;
	int			i;
	time_t			created;


	assert (sBuf != NULL);


	c_burst = (char *)ircsp_calloc (1, strlen (sBuf) + 20);
	assert (c_burst !=NULL);


	strcpy (c_burst, sBuf);


	argv[0] = strtok (sBuf, " ");					/* Server Numeric */
	strtok (NULL, " ");						/* Server Message: BURST [B] - Ignore */
	argv[1] = strtok (NULL, " ");					/* Channel */
	strtok (NULL, " ");

	p_Channel = Channel_Find (argv[1]);
	if (p_Channel != NULL)
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		    Processing contuation BURST, this could go a few different ways

		        1) We could be BURSTing a mixture of users and bans
		        2) We could be BURSTing only users
		        3) We could be BURSTing only bans
		-----------------------------------------------------------------------------------------------------------------
		*/
		argv[2] = strtok(NULL, " ");				/* In a contunation burst this could go one of 2 ways.
									   This could be a BANLIST only, or it could be another
									   USER burst followed by a BANLIST. */

		if (!argv[2])
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    I'm not exactly sure why this happens, from the looks of it if we recover from a network split
			    and a channel doesn't have any remote users from our uplink we only get a BURST
			    with a channel and timestamp and nothing else.
			---------------------------------------------------------------------------------------------------------
			*/
			return 0;
		}


		/*
		-----------------------------------------------------------------------------------------------------------------
		    We are going to check the first 2 characters of argv[2] if they are ':%' then we are dealing with
		    only bans.
		-----------------------------------------------------------------------------------------------------------------
		*/
		if ((argv[2][0] == ':') && (argv[2][1] == '%'))
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    Processing BANLIST
			---------------------------------------------------------------------------------------------------------
			*/
			strtok (c_burst, " ");				/* Server Numeric */
			strtok (NULL, " ");				/* Server Message: BURST [B] - Ignore */
			strtok (NULL, " ");				/* Channel */
			strtok (NULL, " ");				/* Timestamp */
			banlist = strtok (NULL, "\n");
			banlist += 2;


			c_Ban [i_BanCount] = strtok (banlist, " ");	/* First ban in the list */
			ChannelBan_Add (p_Channel, c_Ban [i_BanCount]);

			while (c_Ban [i_BanCount])
			{
				c_Ban [++i_BanCount] = strtok (NULL, " ");
				if (c_Ban [i_BanCount] != NULL)
				{
					ChannelBan_Add (p_Channel, c_Ban [i_BanCount]);
				}
			}
		}
		else
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    Processing USERLIST
			---------------------------------------------------------------------------------------------------------
			*/
			count = tokenize (argv[2], ',');
			for (i = 1; i < count + 1; i++)
			{
				strcpy (burst, token (argv[2], i));
				if (tokenize (burst, ':') == 1)
				{
					p_Nickname = Nickname_Find (token (burst, 1));
					assert (p_Nickname != NULL);


					if (!last_mode)
					{
						/*
						---------------------------------------------------------------------------------
						    Processing USER without any modes (last_mode)
						---------------------------------------------------------------------------------
						*/
						p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
						p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


						assert (p_ChannelNickname != NULL);
						assert (p_NicknameChannel != NULL);
					}
					else
					{
						/*
						---------------------------------------------------------------------------------
						    Processing USER with modes (last_mode)
						---------------------------------------------------------------------------------
						*/
						p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
						p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


						assert (p_ChannelNickname != NULL);
						assert (p_NicknameChannel != NULL);


						p_ChannelNickname->flags = last_flag;
						p_NicknameChannel->flags = last_flag;
					}
				} /* tokenize */
				else
				{
					/*
					-----------------------------------------------------------------------------------------
					    Processing USER with modes
					-----------------------------------------------------------------------------------------
					*/
					p_Nickname = Nickname_Find (token (burst, 1));
					assert (p_Nickname != NULL);


					p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
					p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


					assert (p_ChannelNickname != NULL);
					assert (p_NicknameChannel != NULL);


					strcpy(uflags, token (burst, 2));


					if (uflags[0] == 'o' || uflags[1] == 'o')
					{
						last_flag = 0;
						p_ChannelNickname->flags = 0;
						p_ChannelNickname->flags |= IS_CHANOP;
						p_NicknameChannel->flags = 0;
						p_NicknameChannel->flags |= IS_CHANOP;
						last_flag |= IS_CHANOP;
						last_mode = 1;
					}

					else if (uflags[0] == 'v')
					{
						last_flag = 0;
						p_ChannelNickname->flags = 0;
						p_ChannelNickname->flags |= IS_CHANVOICE;
						p_NicknameChannel->flags |= IS_CHANVOICE;
						p_NicknameChannel->flags = 0;
						last_flag |= IS_CHANVOICE;
						last_mode = 1;
					}
				} /* tokenize */
			}


			argv[3] = strtok (NULL, "\n");

			if (argv[3])
			{
				/*
				-------------------------------------------------------------------------------------------------
				    Processing BANLIST
				-------------------------------------------------------------------------------------------------
				*/


				argv[3] += 2;	/* Skip the :% from the first ban */
				c_Ban[i_BanCount] = strtok (argv[3], " ");
				ChannelBan_Add (p_Channel, c_Ban[i_BanCount]);

				while (c_Ban [i_BanCount])
				{
					c_Ban[++i_BanCount] = strtok (NULL, " ");
					if (c_Ban [i_BanCount] != NULL)
					{
						ChannelBan_Add (p_Channel, c_Ban [i_BanCount]);
					}
				}
			}
		}
	}
	else
	{
		/*
		-----------------------------------------------------------------------------------------------------------------
		    Processing Initial BURST
		-----------------------------------------------------------------------------------------------------------------
		*/
		created = time (NULL);
		modes = strtok (NULL, " ");

		if (modes == NULL)
		{
			/*
			---------------------------------------------------------------------------------------------------------
			    Processing Empty Channel
			---------------------------------------------------------------------------------------------------------
			*/
			p_Channel = Channel_Add (argv[1], flags, created, key, limit);


			assert (p_Channel != NULL);
		}
		else
		{
			created = time (NULL);
			if (modes[0] == '+')
			{
				/*
				-------------------------------------------------------------------------------------------------
				    Processing Channel Modes
				-------------------------------------------------------------------------------------------------
				*/
				int		pos = 0;


				while (modes[++pos] != '\0')
				{
					if (modes[pos] == 'l')
					{
						limit = atoi (strtok (NULL, " "));
						flags |= CHANNEL_HAS_LIMIT;
					}

					if (modes[pos] == 'k')
					{
						key = strtok(NULL, " ");
						flags |= CHANNEL_HAS_KEY;
					}

					if (modes[pos] == 'p')
					{

					}

					if (modes[pos] == 'm')
					{

					}

					if (modes[pos] == 's')
					{

					}

					if (modes[pos] == 't')
					{

					}
				}


				userlist = strtok (NULL, " ");
			}
			else
			{
				userlist = modes;
			}

			banlist = strtok (NULL, "\n");

			p_Channel = Channel_Add (argv[1], flags, created, key, limit);


			assert (p_Channel != NULL);


			if (banlist)
			{
				/*
				-------------------------------------------------------------------------------------------------
				    Processing BANLIST
				-------------------------------------------------------------------------------------------------
				*/
				banlist += 2;
				c_Ban [i_BanCount] = strtok (banlist, " ");
				ChannelBan_Add (p_Channel, c_Ban [i_BanCount]);

				while (c_Ban [i_BanCount])
				{
					c_Ban [++i_BanCount] = strtok (NULL, " ");
					if (c_Ban [i_BanCount] != NULL)
					{
						ChannelBan_Add (p_Channel, c_Ban [i_BanCount]);
					}
				}
			}

			count = tokenize (userlist, ',');

			/*
			---------------------------------------------------------------------------------------------------------
			    Processing USERLIST
			---------------------------------------------------------------------------------------------------------
			*/
			for (i = 1; i < count + 1; i++)
			{
				strcpy(burst, token (userlist, i));
				if (tokenize (burst, ':') == 1)
				{
					if (!last_mode)
					{
						/*
						---------------------------------------------------------------------------------
						    Processing USER without modes (last_mode)
						---------------------------------------------------------------------------------
						*/
						p_Nickname = Nickname_Find (token(burst, 1));


						assert (p_Nickname != NULL);


						p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
						p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


						assert (p_ChannelNickname != NULL);
						assert (p_NicknameChannel != NULL);
					}
					else
					{
						/*
						---------------------------------------------------------------------------------
						    Processing USER with modes (last_mode)
						---------------------------------------------------------------------------------
						*/
						p_Nickname = Nickname_Find (token (burst, 1));


						assert (p_Nickname != NULL);


						p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
						p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


						assert (p_ChannelNickname != NULL);
						assert (p_NicknameChannel != NULL);


						p_NicknameChannel->flags = last_flag;
						p_ChannelNickname->flags = last_flag;
					}
				}
				else
				{
					/*
					-----------------------------------------------------------------------------------------
					    Processing MODES
					-----------------------------------------------------------------------------------------
					*/
					strcpy(uflags, token (burst, 2));
					p_Nickname = Nickname_Find (token (burst, 1));


					assert (p_Nickname != NULL);


					p_ChannelNickname = ChannelNickname_Add (p_Channel, p_Nickname);
					p_NicknameChannel = NicknameChannel_Add (p_Nickname, p_Channel);


					assert (p_ChannelNickname != NULL);
					assert (p_NicknameChannel != NULL);


					if (uflags[0] == 'o' || uflags[1] == 'o')
					{
						last_flag = 0;
						p_ChannelNickname->flags = 0;
						p_ChannelNickname->flags |= IS_CHANOP;
						p_NicknameChannel->flags = 0;
						p_NicknameChannel->flags |= IS_CHANOP;
						last_flag |= IS_CHANOP;
						last_mode = 1;
					}

					else if (uflags[0] == 'v')
					{
						p_ChannelNickname->flags |= IS_CHANVOICE;
						p_NicknameChannel->flags |= IS_CHANVOICE;
						last_flag |= IS_CHANVOICE;
						last_mode = 1;
					}
				}
			}
		}
	}


	if (DEBUG)
	{
		ircsp_log (F_MAINLOG, get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__);
	}


	return TRUE;
}



