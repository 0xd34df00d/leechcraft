/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "astrality.h"
#include <QIcon>
#include <util/util.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
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

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Astrality";
	}

	QString Plugin::GetName () const
	{
		return "Astrality";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for protocols provided by Telepathy.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return QList<QObject*> ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_astrality, LeechCraft::Azoth::Astrality::Plugin);
