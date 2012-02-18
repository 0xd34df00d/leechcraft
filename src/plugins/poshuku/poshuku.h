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

#ifndef PLUGINS_POSHUKU_POSHUKU_H
#define PLUGINS_POSHUKU_POSHUKU_H
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
#include <interfaces/ihavediaginfo.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "browserwidget.h"

class QGraphicsWebView;

namespace LeechCraft
{
namespace Poshuku
{
	class Poshuku : public QObject
					, public IInfo
					, public IHaveTabs
					, public IPluginReady
					, public IHaveSettings
					, public IEntityHandler
					, public IHaveShortcuts
					, public IHaveDiagInfo
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
				IHaveDiagInfo
				IActionsExporter
				IHaveRecoverableTabs)

		QMenu *ToolMenu_;
		QAction *ImportXbel_;
		QAction *ExportXbel_;
		QAction *CheckFavorites_;
		QAction *ReloadAll_;

		std::auto_ptr<QTranslator> Translator_;
		std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
	public:
		virtual ~Poshuku ();
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const LeechCraft::Entity&) const;
		void Handle (LeechCraft::Entity);

		void Open (const QString&);
		IWebWidget* GetWidget () const;
		QGraphicsWebView* CreateWindow ();

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		QString GetDiagInfoString () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		void RecoverTabs (const QList<QByteArray>&);
	private:
		void InitConnections ();
		void RegisterSettings ();
	private slots:
		void createTabFirstTime ();
		void viewerSettingsChanged ();
		void developerExtrasChanged ();
		void cacheSettingsChanged ();
		void handleError (const QString&);
		void handleNewTab ();
		void handleSettingsClicked (const QString&);
		void handleCheckFavorites ();
		void handleReloadAll ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void couldHandle (const LeechCraft::Entity&, bool*);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);

		void tabRecovered (const QByteArray&, QWidget*);
	};
}
}

#endif
