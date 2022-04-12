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
#include	<dlfcn.h>
#include	<assert.h>


/* IRCSP Core Includes */
#include	"users.h"
#include	"channels.h"
#include	"servers.h"
#include	"modules.h"
#include	"ircsp.h"
#include	"conf.h"
#include	"log.h"
#include	"alloc.h"


/* Debugging */
#include	"debug.h"


/* Compile Generator Includes */
#include	"config.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
Module Pointers
---------------------------------------------------------------------------------------------------------------------------------
*/

/* Default handler for module-implemented functions */
static void unimplemented (void)
{

}


/* Protocol Handlers */
FUNCPTR (void, cmd_notice, (char *, char *, char *, ...))
	= (void *)unimplemented;
FUNCPTR (void, cmd_wallops, (char *, char *, ...))
	= (void *)unimplemented;
FUNCPTR (void, cmd_join, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_part, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_chanop, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_chandeop, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_chanvoice, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_chandevoice, (char *, char *))
	= (void *)unimplemented;
FUNCPTR (void, cmd_kill, (char *, char *, char *, ...))
	= (void *)unimplemented;
FUNCPTR (void, cmd_nickjupe, (char *, char *, char *, char *, ...))
	= (void *)unimplemented;
FUNCPTR (void, introdouce_service, (char *[]))
	= (void *)unimplemented;
FUNCPTR (void, introdouce_server, (void))
	= (void *)unimplemented;
FUNCPTR (void, m_parse, (char *))
	= (void *)unimplemented;



/* NickServ Handlers */
//FUNCPTR (void, services_root_unregister, (struct _nickname *))
//	= (void *)unimplemented;


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Module_Add ()
                |
 DESCRIPTION    |    This function will add a new Module to our Modules list.
                |
 RETURNS        |    If successful we return a pointer to the newly created module otherwise we return NULL
---------------------------------------------------------------------------------------------------------------------------------
*/
module_t *Module_Add (name)
	char		*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t	*p_Module;


	p_Module = (module_t *)ircsp_calloc (1, sizeof (module_t));
	p_Module->FileName = (char *)ircsp_calloc (1, strlen (name) + 5);
	if ((!p_Module) || (!p_Module->FileName))
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for module_p\n",
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

	strcpy (p_Module->FileName, name);
	p_Module->ModuleName = NULL;
	p_Module->ModuleVersion = NULL;
	p_Module->ModuleType = 0;
	p_Module->dllHandler = NULL;
	p_Module->module_callback_h = NULL;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Added module [%s] at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, p_Module->FileName,
			p_Module->FileName
		);
	}

	if (!module_h)
	{
		module_h = p_Module;
		p_Module->next = NULL;
	}
	else
	{
		p_Module->next = module_h;
		module_h = p_Module;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_Module;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Module_Find ()
                |
 DESCRIPTION    |    This function will find a specific Module on our Modules list.
                |
 RETURNS        |    Returns pointer to Module if successful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_t *Module_Find (name)
	char		*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t	*p_Module;


	for (p_Module = module_h; p_Module; p_Module = p_Module->next)
	{
		if (!strcasecmp (p_Module->FileName, name))
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Module [%s] found at [%p]\n",
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

			return p_Module;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Module_FindProtocol ()
                |
 DESCRIPTION    |    This function will find our Protocol  Module.
                |
 RETURNS        |    Returns pointer to Module if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_t *Module_FindProtocol (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t	*p_Module;

	for (p_Module = module_h; p_Module; p_Module = p_Module->next)
	{
		if (p_Module->ModuleType == MODULETYPE_PROTOCOL)
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Protocol module [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Module->FileName,
						p_Module->FileName
					);
				}

				if (DEBUG)
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}
			}

			return p_Module;
		}
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    Module_Find_Service ()
                   |
    DESCRIPTION    |    This function will find a Service Module using the ServiceNickname.
                   |
    RETURNS        |    Returns pointer to Module if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_t *Module_Find_Service (nickname)
	char		*nickname;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}


	module_t	*p_Module;

	assert (nickname != NULL);


	for (p_Module = module_h; p_Module; p_Module = p_Module->next)
	{
		if (p_Module->ModuleType & MODULETYPE_SERVICE)
		{
			if (!strcasecmp (p_Module->ServiceNickname, nickname))
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Module [%s] found at [%p]\n",
							__FILE__, __LINE__, __FUNCTION__, p_Module->FileName,
							p_Module->FileName
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

				return p_Module;
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

	return NULL;
}
/*
---------------------------------------------------------------------------------------------------------------------------------
    FUNCTION       |    module_find_nickserv ()
                   |
    DESCRIPTION    |    This function will find the NickServ module if its loaded
                   |
    RETURNS        |    Returns pointer to Module if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_t
*module_find_nickserv (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t		*module_p;

	module_p = module_h;
	while (module_p)
	{
		if (module_p->ServiceType & SERVICETYPE_NICKSERV)
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: NickServ Module [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__,
						module_p->FileName, module_p->FileName
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

			return module_p;
		}

		module_p = module_p->next;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    Module_Del ()
                |
 DESCRIPTION    |    This function will remove a Module, if it has been unloaded. If there are still calbacks or
                |    the handler is not NULL we will assume its still being used.
                |
 RETURNS        |    Returns pointer to Module if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
void Module_Del (name)
	char		*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t	*p_Module;
	module_t	*p_Delete;
	module_t	*p_Next;

	for (p_Module = module_h; p_Module; p_Module = p_Module->next)
	{
		if (!strcasecmp (p_Module->FileName, name))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Module [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, p_Module->FileName,
					p_Module->FileName
				);
			}

			if (p_Module->ModuleType == MODULETYPE_PROTOCOL)
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Module [%s] found at [%p] is the protocol module and cannot be removed\n",
							__FILE__, __LINE__, __FUNCTION__, p_Module->FileName, p_Module->FileName
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

				return ;
			}

			if (!p_Module->dllHandler)
			{
				if ((DEBUG) && (DEBUG_LIST))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Deleting module [%s] found at [%p]\n",
						__FILE__, __LINE__, __FUNCTION__, p_Module->FileName, p_Module->FileName
					);
				}

				if (module_h == p_Module)
				{
					module_h = p_Module->next;
				}
				else
				{
					for (p_Delete = module_h; (p_Delete->next != p_Module) && p_Delete; p_Delete = p_Delete->next);
					p_Delete->next = p_Module->next;
				}

				ircsp_free (p_Module->FileName);
				p_Module->ModuleVersion		= NULL;
				p_Module->ModuleType		= 0;
				p_Module->module_callback_h	= NULL;
				ircsp_free (p_Module);


				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return;
			}
			else
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Module [%s] found at [%p] but its still open\n",
							__FILE__, __LINE__, __FUNCTION__, p_Module->FileName, p_Module->FileName
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

				return ;
			}
		}
	}

}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ModuleCallback_Add ()
                |
 DESCRIPTION    |    This function will add a Callback for a specific module and module function.
                |
                |
 RETURNS        |    Returns pointer to ModuleCallback if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_callback_t *ModuleCallback_Add (p_Module, name)
	struct _module		*p_Module;
	char			*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	p_ModuleCallback = (module_callback_t *)ircsp_calloc (1, sizeof (module_callback_t));
	p_ModuleCallback->name = (char *)ircsp_calloc (1, strlen (name) + 5);

	if ((!p_ModuleCallback) || (!p_ModuleCallback->name))
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for ModuleCallback_p\n",
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

	strcpy (p_ModuleCallback->name, name);
	p_ModuleCallback->module_p = p_Module;

	if ((DEBUG) && (DEBUG_LIST))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: ModuleCallback [%s] added at [%p] for module [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name, p_ModuleCallback->name,
			p_Module->FileName);
	}

	if (!p_Module->module_callback_h)
	{
		p_Module->module_callback_h = p_ModuleCallback;
		p_ModuleCallback->next = NULL;
	}
	else
	{
		p_ModuleCallback->next = p_Module->module_callback_h;
		p_Module->module_callback_h = p_ModuleCallback;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return p_ModuleCallback;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ModuleCallback_Find ()
                |
 DESCRIPTION    |    This function will find a Callback for a specific module and module function.
                |
                |
 RETURNS        |    Returns pointer to ModuleCallback if succesful otherwise NULL.
---------------------------------------------------------------------------------------------------------------------------------
*/
module_callback_t *ModuleCallback_Find (p_Module, name)
	struct _module		*p_Module;
	char			*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	assert (p_Module != NULL);
	assert (name != NULL);

	p_ModuleCallback = p_Module->module_callback_h;
	while (p_ModuleCallback)
	{
		if (!strcasecmp (p_ModuleCallback->name, name))
		{
			if (DEBUG)
			{
				if (DEBUG_LIST)
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Callback [%s] found for Module [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name,
						p_Module->FileName
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

			return p_ModuleCallback;
		}

		p_ModuleCallback = p_ModuleCallback->next;
	}

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return NULL;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ModuleCallback_Del ()
                |
 DESCRIPTION    |    This function will remove a specific Callback for a specific module.
                |
                |
 RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void ModuleCallback_Del (p_Module, name)
	struct _module		*p_Module;
	char			*name;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;
	module_callback_t	*p_Next;
	module_callback_t	*p_Delete;

	for (p_ModuleCallback = p_Module->module_callback_h; p_ModuleCallback; p_ModuleCallback = p_ModuleCallback->next)
	{
		if (!strcasecmp (p_ModuleCallback->name, name))
		{
			if (!(p_Module->ModuleType & MODULETYPE_PROTOCOL))
			{
				if ((DEBUG) && (DEBUG_LIST))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Deleting ModuleCallback [%s] found at [%p] on module [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name, p_ModuleCallback->name,
						p_Module->FileName
					);
				}

				if (p_Module->module_callback_h == p_ModuleCallback)
				{
					p_Module->module_callback_h = p_ModuleCallback->next;
				}
				else
				{
					for (p_Delete = p_Module->module_callback_h; (p_Delete->next != p_ModuleCallback) && p_Delete; p_Delete = p_Delete->next);
					p_Delete->next = p_ModuleCallback->next;
				}

				p_ModuleCallback->module_p = NULL;
				p_Module->module_callback_h = NULL;

				ircsp_free (p_ModuleCallback->name);
				ircsp_free (p_ModuleCallback);

				if ((DEBUG) && (DEBUG_FUNC))
				{
					ircsp_log (F_MAINLOG,
						get_log_message (LOG_MESSAGE_FUNCEND),
						__FILE__, __LINE__, __FUNCTION__
					);
				}

				return ;
			}
			else
			{
				if (DEBUG)
				{
					if (DEBUG_LIST)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: ModuleCallback [%s] found at [%p] on protocol module [%s] cannot be removed\n",
							__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name, p_ModuleCallback->name, p_Module->FileName
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

				return ;
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
 FUNCTION       |    ModuleCallback_DelAll ()
                |
 DESCRIPTION    |    This function will remove a ALL Callback for a specific module.
                |
                |
 RETURNS        |    Nothing
---------------------------------------------------------------------------------------------------------------------------------
*/
void ModuleCallback_DelAll (p_Module)
	struct _module		*p_Module;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;
	module_callback_t	*p_Delete;

	for (p_ModuleCallback = p_Module->module_callback_h; p_ModuleCallback; p_ModuleCallback = p_ModuleCallback->next)
	{
		if (!(p_Module->ModuleType & MODULETYPE_PROTOCOL))
		{
			if ((DEBUG) && (DEBUG_LIST))
			{
				ircsp_log (F_MAINLOG,
					"[%d:%s:%s()]: Deleting ModuleCallback [%s] found at [%p] on module [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name, p_ModuleCallback->name,
					p_Module->FileName
				);
			}

			if (p_Module->module_callback_h == p_ModuleCallback)
			{
				p_Module->module_callback_h = p_ModuleCallback->next;
			}
			else
			{
				for (p_Delete = p_Module->module_callback_h; (p_Delete->next != p_ModuleCallback) && p_Delete; p_Delete = p_Delete->next);
				p_Delete->next = p_ModuleCallback->next;
			}

			p_ModuleCallback->module_p = NULL;

			ircsp_free (p_ModuleCallback->name);
			ircsp_free (p_ModuleCallback);

			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					get_log_message (LOG_MESSAGE_FUNCEND),
					__FILE__, __LINE__, __FUNCTION__
				);
			}

			return ;
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
 FUNCTION       |    ircsp_dlopen ()
                |
 DESCRIPTION    |    This function will handle opening of modules - dlopen()
                |
                |
 RETURNS        |    Pointer to the Module
---------------------------------------------------------------------------------------------------------------------------------
*/
void *ircsp_dlopen (module)
	char		*module;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	void		*handler;
	char		*c_File;

	c_File = (char *)ircsp_calloc (1, strlen (MODULEPATH) + strlen (module) + 10);
	if (!c_File)
	{
		if (DEBUG)
		{
			if (DEBUG_ALLOC)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to allocate memory for c_file\n",
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

	sprintf (c_File, "%s/%s", MODULEPATH, module);

	if ((DEBUG) && (DEBUG_MODULE))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Opening module [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, c_File
		);
	}

	handler = dlopen (c_File, RTLD_NOW | RTLD_GLOBAL);

	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	return handler;
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_dlclose ()
                |
 DESCRIPTION    |    This function will handle opening of modules - dlclose()
                |
                |
 RETURNS        |    Pointer to the Module
---------------------------------------------------------------------------------------------------------------------------------
*/
void ircsp_dlclose (handler)
	void		*handler;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t	*p_Module;

	for (p_Module = module_h; p_Module; p_Module = p_Module->next)
	{
		if (p_Module->dllHandler == handler)
		{
			if (p_Module->ModuleType == MODULETYPE_PROTOCOL)
			{
				if (DEBUG)
				{
					if (DEBUG_MODULE)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Protocol Module [%s] found at [%s] cannot be closed\n",
							__FILE__, __LINE__, __FUNCTION__, p_Module->FileName, p_Module->FileName
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

				return ;
			}
			else
			{
				if ((DEBUG) && (DEBUG_MODULE))
				{
					ircsp_log (F_MAINLOG,
						"[%s:%d:%s()]: Module [%s] found at [%p] has been closed\n",
						__FILE__, __LINE__, __FUNCTION__, p_Module->FileName,
						p_Module->FileName
					);
				}

				dlclose (handler);
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
 FUNCTION       |    ircsp_dlsym ()
                |
 DESCRIPTION    |    This function will handle retreiving a symbol from a module given its handle.
                |
                |
 RETURNS        |    Pointer to the Module
---------------------------------------------------------------------------------------------------------------------------------
*/
void *ircsp_dlsym (handler, symbol)
	void		*handler;
	char		*symbol;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	if (!handler)
	{
		if ((DEBUG) && (DEBUG_MODULE))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Handler is NULL, Module not opened?\n",
				__FILE__, __LINE__, __FUNCTION__
			);
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

	return dlsym (handler, symbol);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ircsp_get_module_symbol ()
                |
 DESCRIPTION    |    This function will retrieve the value of the named symbol in a given module. Return NULL
                |    if no such symbol exists.
                |
                |
 RETURNS        |
---------------------------------------------------------------------------------------------------------------------------------
*/
void *ircsp_get_module_symbol (p_Module, symbol)
	struct _module		*p_Module;
	char			*symbol;
{
	return ircsp_dlsym (p_Module ? p_Module->dllHandler : NULL, symbol);
}


/*
---------------------------------------------------------------------------------------------------------------------------------
 FUNCTION       |    ModuleCallback_Register ()
                |
 DESCRIPTION    |    This function will handle Registering and adding a new callback to a specified module
                |
                |
 RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int ModuleCallback_Register (p_Module, function)
	struct _module		*p_Module;
	char			*function;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;
	void			*symbol;

	if (!p_Module->dllHandler)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Module [%s] appears to not be open\n",
					__FILE__, __LINE__, __FUNCTION__, p_Module->FileName
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


	symbol = ircsp_get_module_symbol (p_Module, function);
	if (!symbol)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Symbol [%s] does not exist in Module [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, function, p_Module->FileName
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

	p_ModuleCallback = ModuleCallback_Add (p_Module, function);
	if (!p_ModuleCallback)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to add new Callback [%s] to Module [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, function, p_Module->FileName
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

	if ((DEBUG) && (DEBUG_MODULE))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: ModuleCallback [%s] added at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, p_ModuleCallback->name,
			p_ModuleCallback->name
		);
	}

	p_ModuleCallback->func = ircsp_dlsym (p_Module->dllHandler, function);

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
    FUNCTION       |    ModuleCallback_Call ()
                   |
    DESCRIPTION    |    This function will handle running a registered callback
                   |
                   |
    RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
void ModuleCallback_Call (p_Module, callback)
	struct _module		*p_Module;
	char			*callback;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	assert (p_Module != NULL);
	assert (callback != NULL);

	p_ModuleCallback = ModuleCallback_Find (p_Module, callback);
	if (!p_ModuleCallback)
	{
		if ((DEBUG) && (DEBUG_MODULE))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Callback [%s] not found in Module [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, callback, p_Module->FileName
			);
		}
	}
	else
	{
		p_ModuleCallback->func (p_Module);
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
    FUNCTION       |    ModuleCallback_Call_1_Array ()
                   |
    DESCRIPTION    |    This function will handle running a registered callback with a single array as a variable
                   |
                   |
    RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
void ModuleCallback_Call_1_Array (p_Module, callback, argv)
	struct _module		*p_Module;
	char			*callback;
	char			*argv[];
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	assert (p_Module != NULL);
	assert (callback != NULL);

	p_ModuleCallback = ModuleCallback_Find (p_Module, callback);
	if (!p_ModuleCallback)
	{
		if ((DEBUG) && (DEBUG_MODULE))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Callback [%s] not found in Module [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, callback, p_Module->FileName
			);
		}
	}
	else
	{
		p_ModuleCallback->func (p_Module, argv);
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
    FUNCTION       |    ModuleCallback_Call_1_Char ()
                   |
    DESCRIPTION    |    This function will handle running a registered callback with a single array as a variable
                   |
                   |
    RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
void ModuleCallback_Call_1_Char (p_Module, callback, var)
	struct _module		*p_Module;
	char			*callback;
	char			*var;
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCSTART),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_callback_t	*p_ModuleCallback;

	assert (p_Module != NULL);
	assert (callback != NULL);

	p_ModuleCallback = ModuleCallback_Find (p_Module, callback);
	if (!p_ModuleCallback)
	{
		if ((DEBUG) && (DEBUG_MODULE))
		{
			ircsp_log (F_MAINLOG,
				"[%s:%d:%s()]: Callback [%s] not found in Module [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, callback, p_Module->FileName
			);
		}
	}
	else
	{
		p_ModuleCallback->func (p_Module, var);
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
 FUNCTION       |    Module_Load_Protocol ()
                |
 DESCRIPTION    |    This function will load our protocol module and register any callbacks
                |
                |
 RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int Module_Load_Protocol (void)
{
	if ((DEBUG) && (DEBUG_FUNC))
	{
		ircsp_log (F_MAINLOG,
			get_log_message (LOG_MESSAGE_FUNCEND),
			__FILE__, __LINE__, __FUNCTION__
		);
	}

	module_t		*p_Module;
	module_callback_t	*p_ModuleCallback;
	ConfigModule		*p_ConfigModule;

	p_ConfigModule = ConfigModule_FindProtocol();
	if (!p_ConfigModule)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: No protocol modules found.\n",
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

	if ((DEBUG) && (DEBUG_MODULE))
	{
		ircsp_log (F_MAINLOG,
			"[%s:%d:%s()]: Loading Protocol Module [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, p_ConfigModule->ModuleName
		);
	}

	p_Module = Module_Add (p_ConfigModule->ModuleName);
	if (!p_Module)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to add Protocol Module [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, p_ConfigModule->ModuleName
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


	p_Module->dllHandler = ircsp_dlopen (p_Module->FileName);
	if (!p_Module->dllHandler)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Protocol Module Error: %s\n",
					__FILE__, __LINE__, __FUNCTION__, dlerror()
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

	p_Module->ModuleVersion = ircsp_dlsym (p_Module->dllHandler, "ModuleVersion");
	p_Module->ModuleName    = ircsp_dlsym (p_Module->dllHandler, "ModuleName"   );
	p_Module->ModuleType |= MODULETYPE_PROTOCOL;

	p_ModuleCallback = ModuleCallback_Add (p_Module, "init_module");
	if (!p_ModuleCallback)
	{
		if (DEBUG)
		{
			if (DEBUG_MODULE)
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Failed to add Protocol init_module Callback\n",
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


	p_ModuleCallback->func = ircsp_dlsym (p_Module->dllHandler, "init_module");
	p_ModuleCallback->func (p_Module);

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
 FUNCTION       |    Module_Load_Services ()
                |
 DESCRIPTION    |    This function will load our services modules and register any callbacks
                |
                |
 RETURNS        |    1 if Successful, 0 if Failure
---------------------------------------------------------------------------------------------------------------------------------
*/
int Module_Load_Services (void)
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
	ConfigModule		*p_ConfigModule;

	p_ConfigModule = config->ConfigModuleHead;
	while (p_ConfigModule)
	{
		if (p_ConfigModule->ModuleType & MODULETYPE_SERVICE)
		{
			if ((DEBUG) && (DEBUG_FUNC))
			{
				ircsp_log (F_MAINLOG,
					"[%s:%d:%s()]: Loading Service Module [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, p_ConfigModule->ModuleName
				);
			}

			p_Module = Module_Add (p_ConfigModule->ModuleName);
			assert (p_Module != NULL);

			p_Module->dllHandler = ircsp_dlopen (p_Module->FileName);
			if (!p_Module->dllHandler)
			{
				if (DEBUG)
				{
					if (DEBUG_MODULE)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Services Module Error: %s\n",
							__FILE__, __LINE__, __FUNCTION__, dlerror ()
						);
					}

					if (DEBUG)
					{
						ircsp_log (F_MAINLOG,
							get_log_message (LOG_MESSAGE_FUNCEND),
							__FILE__, __LINE__, __FUNCTION__
						);
					}
				}

				return FALSE;
			}

			p_Module->ModuleVersion = ircsp_dlsym (p_Module->dllHandler, "ModuleVersion");
			p_Module->ModuleName = ircsp_dlsym (p_Module->dllHandler, "ModuleName");
			p_Module->ModuleType |= MODULETYPE_SERVICE;

			p_ModuleCallback = ModuleCallback_Add (p_Module, "init_module");
			if (!p_ModuleCallback)
			{
				if (DEBUG)
				{
					if (DEBUG_MODULE)
					{
						ircsp_log (F_MAINLOG,
							"[%s:%d:%s()]: Failed to add Service Module init_module Callback\n",
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

			p_ModuleCallback->func = ircsp_dlsym (p_Module->dllHandler, "init_module");
			p_ModuleCallback->func (p_Module);
		}

		p_ConfigModule = p_ConfigModule->next;
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
