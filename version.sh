#!/bin/sh
#
# Build the version.c file which contains all the version related info and
# needs to be updated on a per-build basis.

# $PROGRAM is the string returned as the first part of a /VERSION reply,
# and must not contain spaces.  It is not used anywhere else.
PROGRAM=IRCSP
CODENAME=Photon
VERSION=2.0.1

# Increment Services build number
if [ -f source/version.c ]; then
    BUILD=`fgrep '#define BUILD' source/version.c | sed 's/^#define BUILD.*"\([0-9]*\)".*$/\1/'`
    BUILD=`expr $BUILD + 1 2>/dev/null`
else
    BUILD=1
fi
if [ ! "$BUILD" ]; then
    BUILD=1
fi

DATE=`date`
if [ $? -ne "0" ]; then
    DATE="\" __DATE__ \" \" __TIME__ \""
fi

cat >source/version.c <<EOF
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


/* IRCSP Core Includes */
#include	"version.h"


/*
---------------------------------------------------------------------------------------------------------------------------------
 DEFINES
---------------------------------------------------------------------------------------------------------------------------------
*/
#define BUILD   "$BUILD"


/*
---------------------------------------------------------------------------------------------------------------------------------
 VARIABLES
---------------------------------------------------------------------------------------------------------------------------------
*/
const char program_name[] = "$PROGRAM";
const char code_name[] = "$CODENAME";
const char version_number[] = "$VERSION";
const char version_build[] = "build #" BUILD ", compiled $DATE";


/*
---------------------------------------------------------------------------------------------------------------------------------
 Look folks, please leave this INFO reply intact and unchanged.  If you do
 have the urge to mention yourself, please simply add your name to the list.
 The other people listed below have just as much right, if not more, to be
 mentioned.  Leave everything else untouched.  Thanks.
---------------------------------------------------------------------------------------------------------------------------------
*/

const char *info_text[] = {
	"IRCSP [Internet Relay Chat Service Package]",
	" Copyright (C) 1995 - 2019 GOTUNIX Networks",
	"                       All rights reserved.",
	" ",
	" ",
	"IRCSP HISTORY:",
	"  Version 1:",
	"    Was created around 1995 and based off of",
	"    KWorld Which was created by Anthony Sorinao",
	"    for krush.net A good portion of it was re-built",
	"    to support ircu 2.9",
	" ",
	"  Version 2:",
	"    Was re-written around 2001, mostly from",
	"    scratch with the idea to support some of the",
	"    most common IRCD's (efnet, dalnet and",
	"    undernet P10) ",
	" ",
	"  Some of the original KWorld code can still be",
	"  found but modified quite a bit",
	" ",
	"  IRCSP is currently only developed by",
	"     Justin Ovens <jovens@gotunix.net>",
	" ",
	"  IRCSP may be freely redistributed under the",
	"  GNU General Public License, version 3 or later.",
	" ",
	"  Many people have contributed to the ongoing development",
	"  of IRCSP Particularly noteworthy contributors include:",
	"      Matt Holmes",
	"      Jim Johnson",
	"      Chris Olsen",
	"      Carsten Haitzler",
	"  A full list of contributors and their contributions",
	"  can be found in the AUTHORS file included in the",
	"  IRCSP distribution archive.  Many thanks to all of",
	"  them!",
	0
};
EOF
