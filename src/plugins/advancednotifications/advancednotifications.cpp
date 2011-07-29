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
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "notificationruleswidget.h"
#include "core.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		
		Core::Instance ().SetProxy (proxy);
		
		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		
		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"advancednotificationssettings.xml");
		SettingsDialog_->SetCustomWidget ("RulesWidget",
				Core::Instance ().GetNRW ());
		
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
	
	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_ == "x-leechcraft/notification" &&
			e.Additional_.contains ("org.LC.AdvNotifications.SenderID") &&
			e.Additional_.contains ("org.LC.AdvNotifications.EventID");

		if (!can)
			return EntityTestHandleResult ();
		
		EntityTestHandleResult result (EntityTestHandleResult::PIdeal);
		result.CancelOthers_ = true;
		return result;
	}
	
	void Plugin::Handle (Entity e)
	{
		GeneralHandler_->Handle (e);
	}
	
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_advancednotifications, LeechCraft::AdvancedNotifications::Plugin);
