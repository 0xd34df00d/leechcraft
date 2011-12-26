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

#include "vader.h"
#include <QIcon>
#include "core.h"
#include "mrimprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().GetProtocol ()->setParent (this);
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().GetProtocol ()->Init ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Vader";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Vader";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the Mail.ru Agent protocol.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/vader/resources/images/vader.svg");
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
		return QList<QObject*> () << Core::Instance ().GetProtocol ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetProxy (proxy);
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_vader, LeechCraft::Azoth::Vader::Plugin);
