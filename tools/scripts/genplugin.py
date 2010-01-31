#!/usr/bin/env python

from string import Template
from optparse import OptionParser

cmake_str = """IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_${plug_lower})

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)

	INCLUDE ($${LEECHCRAFT_USE_FILE})
ENDIF (NOT QT_USE_FILE)

INCLUDE ($${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	$${CMAKE_CURRENT_BINARY_DIR}
	$${Boost_INCLUDE_DIR}
	$${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	$plug_lower.cpp
	)
SET (HEADERS
	$plug_lower.h
	)
QT4_WRAP_CPP (MOC_SRCS $${HEADERS})

ADD_LIBRARY (leechcraft_${plug_lower} SHARED
	$${COMPILED_TRANSLATIONS}
	$${SRCS}
	$${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_${plug_lower}
	$${QT_LIBRARIES}
	$${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_${plug_lower} DESTINATION $${LC_PLUGINS_DEST})
"""

header_str = """/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  $author
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_${plug_upper}_${plug_upper}_H
#define PLUGINS_${plug_upper}_${plug_upper}_H
#include <QObject>
#include <interfaces/$interfaces_includes.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace $plug
		{
			class Plugin : public QObject
						 , public $interfaces_inherit
			{
				Q_OBJECT
				Q_INTERFACES ($interfaces)
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
$interfaces_decls
			};
		};
	};
};

#endif

"""

source_str = """/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  $author
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "${plug_lower}.h"
#include <QIcon>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace $plug
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QString Plugin::GetName () const
			{
				return "$plug";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

$interfaces_defs
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_${plug_lower}, LeechCraft::Plugins::$plug::Plugin);

"""

decls_all = {}
decls_all ['IToolBarEmbedder'] = """				QList<QAction*> GetActions () const;"""

defs_all = {}
defs_all ['IToolBarEmbedder'] = """			QList<QAction*> Plugin::GetActions () const
			{
				QList<QAction*> result;
				return result;
			}"""

parser = OptionParser ()
parser.add_option ('-a', '--author', dest='author')
parser.add_option ('-p', '--plugin', dest='plugin')
parser.add_option ('-i', '--interfaces', dest='interfaces', default=None)
(p, args) = parser.parse_args ()

interfaces_array = []

if p.interfaces != None:
	for iface in p.interfaces.split (','):
		interfaces_array.append (iface)

decls_array = ([decls_all [v] for v in interfaces_array])
defs_array = ([defs_all [v] for v in interfaces_array])

interfaces_array.insert (0, 'IInfo')

interfaces_includes = map (lambda s: s.lower (), interfaces_array)

d = {}
d ['author'] = p.author
d ['plug_upper'] = p.plugin.upper ()
d ['plug_lower'] = p.plugin.lower ()
d ['plug'] = p.plugin
d ['interfaces'] = ' '.join (interfaces_array)
d ['interfaces_inherit'] = '\n						 , public '.join (interfaces_array)
d ['interfaces_includes'] = '.h>\n#include <interfaces/'.join (interfaces_includes)
d ['interfaces_decls'] = '\n\n'.join (decls_array)
d ['interfaces_defs'] = '\n\n'.join (defs_array)

header = Template (header_str)
hfile = open ('%s.h' % p.plugin.lower (), 'w')
hfile.write (header.substitute (d))

source = Template (source_str)
sfile = open ('%s.cpp' % p.plugin.lower (), 'w')
sfile.write (source.substitute (d))

cmake = Template (cmake_str)
sfile = open ('CMakeLists.txt', 'w')
sfile.write (cmake.substitute (d))
