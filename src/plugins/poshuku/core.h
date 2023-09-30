/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <QObject>
#include <QUrl>
#include <QTimer>
#include <util/tags/tagscompletionmodel.h>
#include <interfaces/structures.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "interfaces/poshuku/iwebviewprovider.h"
#include "favoritesmodel.h"
#include "historymodel.h"
#include "storagebackend.h"
#include "urlcompletionmodel.h"
#include "pluginmanager.h"
#include "browserwidgetsettings.h"

class QString;
class QWidget;
class QIcon;
class QAbstractItemModel;
class QNetworkReply;
class QNetworkAccessManager;
class IWebWidget;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Poshuku
{
	class IWebView;
	class IWebViewProvider;
	class BrowserWidget;

	class Core : public QObject
	{
		Q_OBJECT

		typedef std::vector<BrowserWidget*> widgets_t;
		widgets_t Widgets_;

		PluginManager * const PluginManager_;
		URLCompletionModel * const URLCompletionModel_;
		HistoryModel * const HistoryModel_;
		FavoritesModel * const FavoritesModel_;

		std::shared_ptr<StorageBackend> StorageBackend_;
		QNetworkAccessManager *NetworkAccessManager_ = nullptr;

		ICoreProxy_ptr Proxy_;
		Util::ShortcutManager *ShortcutMgr_ = nullptr;

		bool Initialized_ = false;

		TabClassInfo TabClass_;

		QList<IWebViewProvider*> WebViewProviders_;

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
		void Release ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		void SetShortcutManager (Util::ShortcutManager*);
		TabClassInfo GetTabClass () const;

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QUrl MakeURL (QString);
		BrowserWidget* NewURL (const QUrl&, bool = false, const DynPropertiesList_t& = DynPropertiesList_t ());
		BrowserWidget* NewURL (const QString&, bool = false);
		std::unique_ptr<BrowserWidget> CreateWidget ();
		IWebView* MakeWebView (bool = false);
		void Unregister (BrowserWidget*);

		void CheckFavorites ();
		void ReloadAll ();

		FavoritesModel* GetFavoritesModel () const;
		HistoryModel* GetHistoryModel () const;
		URLCompletionModel* GetURLCompletionModel () const;
		QNetworkAccessManager* GetNetworkAccessManager () const;
		StorageBackend* GetStorageBackend () const;
		PluginManager* GetPluginManager () const;

		QIcon GetIcon (const QUrl&) const;
		QString GetUserAgent (const QUrl&) const;

		bool IsUrlInFavourites (const QString&);
		void RemoveFromFavorites (const QString&);

		std::shared_ptr<IWebView> CreateWebView ();
	private:
		BrowserWidget* CreateBrowserWidget (const std::shared_ptr<IWebView>&, const QUrl&,
				bool, const QList<QPair<QByteArray, QVariant>>&);
		void HandleHistory (IWebView*);
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
		void handleAddToFavorites (QString, QString);
		void handleStatusBarChanged (const QString&);
		void handleTooltipChanged (QWidget*);
		void handleWebViewCreated (const std::shared_ptr<IWebView>&, LC::Poshuku::NewWebViewBehavior::Enum);
		void favoriteTagsUpdated (const QStringList&);
	signals:
		void error (const QString&) const;
		void bookmarkAdded (const QString&);
		void bookmarkRemoved (const QString&);
		void browserWidgetCreated (BrowserWidget*);

		// Hook support signals
		void hookAddToFavoritesRequested (LC::IHookProxy_ptr,
				QString title, QString url);
		void hookIconRequested (LC::IHookProxy_ptr,
				const QUrl& url) const;
		void hookTabAdded (LC::IHookProxy_ptr,
				QObject *browserWidget,
				const QUrl& url);
		void hookUserAgentForUrlRequested (LC::IHookProxy_ptr,
				const QUrl&) const;
	};
}
}

Q_DECLARE_METATYPE (LC::Poshuku::Core::UncloseData)
