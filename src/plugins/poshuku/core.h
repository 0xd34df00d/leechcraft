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

#ifndef PLUGINS_POSHUKU_CORE_H
#define PLUGINS_POSHUKU_CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QUrl>
#include <QTimer>
#include <util/tagscompletionmodel.h>
#include <interfaces/structures.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaveshortcuts.h>
#include "favoritesmodel.h"
#include "historymodel.h"
#include "storagebackend.h"
#include "urlcompletionmodel.h"
#include "pluginmanager.h"
#include "browserwidgetsettings.h"

class QString;
class QWidget;
class QIcon;
class QGraphicsWebView;
class QAbstractItemModel;
class QNetworkReply;
class QNetworkAccessManager;
class IWebWidget;
class IShortcutProxy;

namespace LeechCraft
{
namespace Poshuku
{
	class CustomWebView;
	class BrowserWidget;
	class FavoritesChecker;
	class WebPluginFactory;

	class Core : public QObject
	{
		Q_OBJECT

		typedef std::vector<BrowserWidget*> widgets_t;
		widgets_t Widgets_;
		// title/url pairs;
		QList<QPair<QString, QString> > SavedSessionState_;
		QList<BrowserWidgetSettings> SavedSessionSettings_;

		std::auto_ptr<FavoritesModel> FavoritesModel_;
		std::auto_ptr<HistoryModel> HistoryModel_;
		std::auto_ptr<URLCompletionModel> URLCompletionModel_;
		std::auto_ptr<PluginManager> PluginManager_;
		boost::shared_ptr<StorageBackend> StorageBackend_;
		QNetworkAccessManager *NetworkAccessManager_;
		WebPluginFactory *WebPluginFactory_;

		bool IsShuttingDown_;
		QList<int> RestoredURLs_;

		QMap<QString, QString> SavedSession_;
		QList<QAction*> Unclosers_;
		IShortcutProxy *ShortcutProxy_;

		ICoreProxy_ptr Proxy_;

		FavoritesChecker *FavoritesChecker_;

		bool Initialized_;

		TabClassInfo TabClass_;

		Core ();
	public:
		enum FormRememberType
		{
			FRTRemember_,
			FRTNotNow_,
			FRTNever_
		};

		struct UncloseData
		{
			QUrl URL_;
			QPoint SPoint_;
			QByteArray History_;
		};

		static Core& Instance ();
		void Init ();
		void SecondInit ();
		void Release ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		TabClassInfo GetTabClass () const;

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);

		WebPluginFactory* GetWebPluginFactory ();

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QUrl MakeURL (QString);
		BrowserWidget* NewURL (const QUrl&, bool = false);
		BrowserWidget* NewURL (const QString&, bool = false);
		IWebWidget* GetWidget ();
		CustomWebView* MakeWebView (bool = false);
		void Unregister (BrowserWidget*);
		/** Sets up the connections between widget's signals
			* and our signals/slots only useful in own mode.
			*
			* Calls to SetupConnections internally as well.
			*/
		void ConnectSignals (BrowserWidget *widget);

		void CheckFavorites ();
		void ReloadAll ();

		FavoritesModel* GetFavoritesModel () const;
		HistoryModel* GetHistoryModel () const;
		URLCompletionModel* GetURLCompletionModel () const;
		QNetworkAccessManager* GetNetworkAccessManager () const;
		StorageBackend* GetStorageBackend () const;
		PluginManager* GetPluginManager () const;
		void SetShortcut (const QString& name, const QKeySequences_t& shortcut);
		IShortcutProxy* GetShortcutProxy () const;

		QIcon GetIcon (const QUrl&) const;
		QString GetUserAgent (const QUrl&, const QWebPage* = 0) const;

		bool IsUrlInFavourites (const QString&);
		void RemoveFromFavorites (const QString&);
	private:
		void RestoreSession (bool);
		void HandleHistory (CustomWebView*);
		/** Sets up the connections between widget's signals
			* and our signals/slots that are always useful, both in own
			* and deown mode.
			*/
		void SetupConnections (BrowserWidget *widget);
		void HandleSearchRequest (const QString&);
	public slots:
		void importXbel ();
		void exportXbel ();
	private slots:
		void handleUnclose ();
		void handleTitleChanged (const QString&);
		void handleURLChanged (const QString&);
		void handleIconChanged (const QIcon&);
		void handleNeedToClose ();
		void handleAddToFavorites (QString, QString);
		void handleStatusBarChanged (const QString&);
		void handleTooltipChanged (QWidget*);
		void favoriteTagsUpdated (const QStringList&);
		void saveSession ();
		void saveSingleSession ();
		void restorePages ();
		void postConstruct ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void raiseTab (QWidget*);
		void error (const QString&) const;
		void statusBarChanged (QWidget*, const QString&);
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void couldHandle (const LeechCraft::Entity&, bool*);
		void newUnclose (QAction*);
		void bookmarkAdded (const QString&);
		void bookmarkRemoved (const QString&);
		// Hook support signals
		void hookAddToFavoritesRequested (LeechCraft::IHookProxy_ptr,
				QString title, QString url);
		void hookIconRequested (LeechCraft::IHookProxy_ptr,
				const QUrl& url) const;
		void hookSessionRestoreScheduled (LeechCraft::IHookProxy_ptr,
				const QList<QUrl>& urls);
		void hookTabAdded (LeechCraft::IHookProxy_ptr,
				QObject *browserWidget,
				QGraphicsWebView *view,
				const QUrl& url);
		void hookUserAgentForUrlRequested (LeechCraft::IHookProxy_ptr,
				const QUrl&, const QWebPage*) const;
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::Poshuku::Core::UncloseData);

#endif
