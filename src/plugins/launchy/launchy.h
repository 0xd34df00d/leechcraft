/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/ihaveshortcuts.h>
#include <util/xdg/xdgfwd.h>

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Launchy
{
	class FavoritesManager;
	class RecentManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
				 , public IHaveShortcuts
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter IHaveShortcuts IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Launchy")

		ICoreProxy_ptr Proxy_;
		Util::XDG::ItemsFinder *Finder_;
		FavoritesManager *FavManager_;
		RecentManager *RecentManager_;

		Util::ShortcutManager *ShortcutMgr_;
		QAction *FSLauncher_;

		QuarkComponent_ptr LaunchQuark_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		QMap<QString, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QString& id, const QKeySequences_t& sequences) override;

		QuarkComponents_t GetComponents () const override;
	private slots:
		void handleFSRequested ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
