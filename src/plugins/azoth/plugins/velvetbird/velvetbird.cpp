/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "velvetbird.h"
#include "protomanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		ProtoMgr_ = new ProtoManager (proxy, this);
		connect (ProtoMgr_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (ProtoMgr_,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.VelvetBird";
	}

	QString Plugin::GetName () const
	{
		return "Azoth VelvetBird";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the protocols provided by the libpurple library.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/azoth/velvetbird/resources/images/velvetbird.svg");
		return icon;
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
		return ProtoMgr_->GetProtoObjs ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_velvetbird, LeechCraft::Azoth::VelvetBird::Plugin);
