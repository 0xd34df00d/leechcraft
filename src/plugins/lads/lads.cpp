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

#include "lads.h"
#include <QIcon>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QAction>

namespace LeechCraft
{
namespace Lads
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		auto sb = QDBusConnection::sessionBus ();
		const auto& services = sb.interface ()->registeredServiceNames ().value ();
		Action_ = 0;
		if (services.contains ("com.canonical.Unity", Qt::CaseSensitive))
		{
			Action_ = new QAction (tr ("Show/hide LeechCraft window"), this);
			connect (Action_, SIGNAL (triggered ()), this, SLOT (showHideMain ()));
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lads";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Lads";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Ubuntu Unity integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == AEPTrayMenu && Action_)
			result << Action_;
		return result; 
	}
	
	void Plugin::showHideMain () const
	{
		
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lads, LeechCraft::Lads::Plugin);

