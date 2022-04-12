
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


#ifndef		__INCLUDED_LIBNICKSERV_H__
#define		__INCLUDED_LIBNICKSERV_H__
#define		E			extern

E	char		ModuleVersion[];
E	char		ModuleName[];
E	char		ModuleType[];


module_t	*this_module_p;

/*
---------------------------------------------------------------------------------------------------------------------------------
    DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define		NICKSERV_SAVE_CYCLE		120		/* How often we save our databases */
#define		SERVICES_ADMIN_SAVE_CYCLE	120		/* How often we save our ServicesAdmin database */
#define		NICKSERV_EXPIRE_CYCLE		300		/* How often we check for expired nickname registrations */
#define		NICKSERV_JUPE_EXPIRE_CYCLE	30		/* How often we check for expired nick jupes */
#define		NICKSERV_VALIDATE_CYCLE		30		/* How long before we kill a user that doesn't identify */


/*
---------------------------------------------------------------------------------------------------------------------------------
    PROTOTYPES
---------------------------------------------------------------------------------------------------------------------------------
*/
E	void		init_module					(struct _module *);
E	void		init_service					(struct _module *);

#endif		/* __INCLUDED_LIBNICKSERV_H__ */
