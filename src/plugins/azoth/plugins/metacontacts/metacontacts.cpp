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

#include "metacontacts.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/iclentry.h>
#include "metaprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_metacontacts");
		
		Proto_ = new MetaProtocol (this);
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Metacontacts";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Metacontacts";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth Metacontacts provides support for joining different contacts into one metacontact.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return result;
	}
	
	QObject* Plugin::GetObject ()
	{
		return this;
	}
	
	QList<QObject*> Plugin::GetProtocols () const
	{
		QList<QObject*> result;
		result << Proto_;
		return result;
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_metacontacts, LeechCraft::Azoth::Metacontacts::Plugin);
