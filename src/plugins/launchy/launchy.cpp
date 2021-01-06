/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "launchy.h"
#include <QIcon>
#include <QAction>
#include <QAbstractItemModel>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/xdg/itemtypes.h>
#include <util/xdg/itemsdatabase.h>
#include "fsdisplayer.h"
#include "favoritesmanager.h"
#include "quarkmanager.h"
#include "itemimageprovider.h"
#include "recentmanager.h"

namespace LC
{
namespace Launchy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("launchy");

		Proxy_ = proxy;

		Finder_ = new Util::XDG::ItemsDatabase (proxy,
				{
					Util::XDG::Type::Application,
					Util::XDG::Type::Dir,
					Util::XDG::Type::URL
				});

		FavManager_ = new FavoritesManager;
		RecentManager_ = new RecentManager;

		ShortcutMgr_ = new Util::ShortcutManager (proxy, this);
		ShortcutMgr_->SetObject (this);

		FSLauncher_ = new QAction (tr ("Open fullscreen launcher..."), this);
		FSLauncher_->setProperty ("ActionIcon", "system-run");
		FSLauncher_->setShortcut (QString ("Meta+R"));
		connect (FSLauncher_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFSRequested ()));

		ShortcutMgr_->RegisterAction ("FSLauncher", FSLauncher_);

		auto itemImageProv = new ItemImageProvider { proxy };
		auto quarkMgr = new QuarkManager (proxy, FavManager_, Finder_, itemImageProv);

		LaunchQuark_.reset (new QuarkComponent ("launchy", "LaunchyQuark.qml"));
		LaunchQuark_->DynamicProps_.push_back ({ "Launchy_itemModel", quarkMgr->GetModel () });
		LaunchQuark_->DynamicProps_.push_back ({ "Launchy_proxy", quarkMgr });
		LaunchQuark_->ImageProviders_.push_back ({ "LaunchyItemIcons", itemImageProv });
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
		new FSDisplayer (Proxy_, Finder_, FavManager_, RecentManager_, this);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_launchy, LC::Launchy::Plugin);
