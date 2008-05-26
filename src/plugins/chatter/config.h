/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#define FS_SERVER "irc.freenode.net"
#define FS_CHANNEL "#qt-ru"
#define FS_PORT 6667
#define FS_REALNAME "Fucking simple irc v3"
#define FS_VERSION "3.0"
#define FS_COM_CHAR "/"
#define FS_VERSION_REPLY QString("VERSION leechcraft chatter v")+QString(FS_VERSION)+tr(", using Qt v")+QString(QT_VERSION_STR)

#define ACT_URI 0
#define ACT_NICK 1
#define ACT_CHANNEL 2
#define ACT_ENCODING 3
#define ACT_QUIT 4

#endif
