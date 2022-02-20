/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAction>
#include <QTranslator>
#include <QWidget>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "browserwidget.h"

namespace LC
{
namespace Util
{
	class WkFontsWidget;
	class ShortcutManager;
}

namespace Poshuku
{
	class Poshuku : public QObject
					, public IInfo
					, public IHaveTabs
					, public IPluginReady
					, public IHaveSettings
					, public IEntityHandler
					, public IHaveShortcuts
					, public IWebBrowser
					, public IActionsExporter
					, public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IEntityHandler
				IPluginReady
				IWebBrowser
				IHaveShortcuts
				IActionsExporter
				IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku")

		QMenu *ToolMenu_;
		QAction *ImportXbel_;
		QAction *ExportXbel_;
		QAction *CheckFavorites_;
		QAction *ReloadAll_;

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		Util::ShortcutManager *ShortcutMgr_;

		Util::WkFontsWidget *FontsWidget_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QStringList Provides () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		std::shared_ptr<LC::Util::XmlSettingsDialog> GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const LC::Entity&) const;
		void Handle (LC::Entity);

		std::unique_ptr<IWebWidget> CreateWidget () const;

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		void RecoverTabs (const QList<TabRecoverInfo>&);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;
	private:
		void InitConnections ();
		void PrepopulateShortcuts ();
	private slots:
		void createTabFirstTime ();
		void handleError (const QString&);
		void handleSettingsClicked (const QString&);
		void handleCheckFavorites ();
		void handleReloadAll ();
		void handleBrowserWidgetCreated (BrowserWidget*);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);

		void tabRecovered (const QByteArray&, QWidget*);
	};
}
}
