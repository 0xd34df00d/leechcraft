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

#include "advancednotifications.h"
#include <QIcon>
#include "generalhandler.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		
		GeneralHandler_.reset (new GeneralHandler (proxy));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AdvancedNotifications";
	}

	void Plugin::Release ()
	{
		GeneralHandler_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Advanced Notifications";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Module for the advanced notifications framework.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
	
	bool Plugin::CouldHandle (const Entity& e) const
	{
		return e.Mime_ == "x-leechcraft/notification" &&
			e.Additional_.contains ("org.LC.AdvNotifications.SenderID") &&
			e.Additional_.contains ("org.LC.AdvNotifications.EventID");
	}
	
	void Plugin::Handle (Entity e)
	{
		GeneralHandler_->Handle (e);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_advancednotifications, LeechCraft::AdvancedNotifications::Plugin);
