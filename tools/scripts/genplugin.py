#!/usr/bin/env python

from string import Template
from optparse import OptionParser

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
			void Plugin::Init (ICoreProxy_ptr)
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
				return QIcon (":/resources/images/${plug_lower}.svg");
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
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_${plug_lower}, LeechCraft::Plugins::$plug::Plugin);

"""

parser = OptionParser ()
parser.add_option ('-a', '--author', dest='author')
parser.add_option ('-p', '--plugin', dest='plugin')
parser.add_option ('-i', '--interfaces', dest='interfaces', default=None)
(p, args) = parser.parse_args ()

interfaces_array = ['IInfo']

if p.interfaces != None:
	for iface in p.interfaces.split (','):
		interfaces_array.append (iface)

interfaces_includes = map (lambda s: s.lower (), interfaces_array)

d = {}
d ['author'] = p.author
d ['plug_upper'] = p.plugin.upper ()
d ['plug'] = p.plugin
d ['interfaces'] = ' '.join (interfaces_array)
d ['interfaces_inherit'] = '\n						 , public '.join (interfaces_array)
d ['interfaces_includes'] = '.h>\n#include <interfaces/'.join (interfaces_includes)

header = Template (header_str)
hfile = open ('%s.h' % p.plugin.lower (), 'w')
hfile.write (header.substitute (d))

d ['plug_lower'] = p.plugin.lower ()

source = Template (source_str)
sfile = open ('%s.cpp' % p.plugin.lower (), 'w')
sfile.write (source.substitute (d))
