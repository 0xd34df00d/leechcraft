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

#include "advancednotifications.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/resourceloader.h>
#include <util/util.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "notificationruleswidget.h"
#include "core.h"
#include "enablesoundactionmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("advancednotifications");

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
		SettingsDialog_->SetDataSource ("AudioTheme",
				Core::Instance ().GetAudioThemeLoader ()->GetSubElemModel ());

		GeneralHandler_.reset (new GeneralHandler (proxy));
		connect (GeneralHandler_.get (),
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));

		EnableSoundMgr_ = new EnableSoundActionManager (this);
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
		Core::Instance ().Release ();
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
		return QIcon (":/plugins/advancednotifications/resources/images/advancednotifications.svg");
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_.startsWith ("x-leechcraft/notification") &&
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

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		result << EnableSoundMgr_->GetActions (aep);
		return result;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_advancednotifications, LeechCraft::AdvancedNotifications::Plugin);
