/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "launchy.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/shortcuts/shortcutmanager.h>
#include "itemsfinder.h"
#include "fsdisplayer.h"
#include "favoritesmanager.h"
#include "quarkmanager.h"
#include "itemimageprovider.h"

namespace LeechCraft
{
namespace Launchy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("launchy");

		Proxy_ = proxy;

		Finder_ = new ItemsFinder (proxy);

		FavManager_ = new FavoritesManager;

		ShortcutMgr_ = new Util::ShortcutManager (proxy, this);
		ShortcutMgr_->SetObject (this);

		FSLauncher_ = new QAction (tr ("Open fullscreen launcher..."), this);
		FSLauncher_->setProperty ("ActionIcon", "system-run");
		FSLauncher_->setShortcut (QString ("Meta+R"));
		connect (FSLauncher_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFSRequested ()));

		ShortcutMgr_->RegisterAction ("FSLauncher", FSLauncher_, true);

		auto itemImageProv = new ItemImageProvider;
		auto quarkMgr = new QuarkManager (proxy, FavManager_, Finder_, itemImageProv);
		LaunchQuark_.Url_ = QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML, "launchy", "LaunchyQuark.qml"));
		LaunchQuark_.DynamicProps_.push_back ({ "Launchy_itemModel", quarkMgr->GetModel () });
		LaunchQuark_.DynamicProps_.push_back ({ "Launchy_proxy", quarkMgr });
		LaunchQuark_.ImageProviders_.push_back ({ "LaunchyItemIcons", itemImageProv });
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Launchy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Launchy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Neat application launcher for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::LCTray)
			result << FSLauncher_;
		return result;
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		ShortcutMgr_->SetShortcut (id, sequences);
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { LaunchQuark_ };
	}

	void Plugin::handleFSRequested ()
	{
		auto dis = new FSDisplayer (Proxy_, Finder_, FavManager_, this);
		connect (dis,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_launchy, LeechCraft::Launchy::Plugin);
