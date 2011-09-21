/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "onlinebookmarks.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "accountssettings.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("poshuku_onlinebookmarks"));
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukuonlinebookmarkssettings.xml");

		Core::Instance ().SetProxy (proxy);
	}

	void Plugin::SecondInit ()
	{
		SettingsDialog_->SetCustomWidget ("Accounts", new AccountsSettings);
		SettingsDialog_->SetDataSource ("ActiveServices",
				Core::Instance ().GetActiveServicesModel ());
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku OnlineBookmarks";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Sync local bookmarks with your account in online bookmark services like Read It Later");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/onlinebookmarks/resources/images/onlinebookmarks.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_onlinebookmarks,
		LeechCraft::Poshuku::OnlineBookmarks::Plugin);
 