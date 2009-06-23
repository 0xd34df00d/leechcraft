#ifndef PLUGINS_POSHUKU_CORE_H
#define PLUGINS_POSHUKU_CORE_H
#include <memory>
#include <vector>
#include <QObject>
#include <QUrl>
#include <QTimer>
#include <plugininterface/tagscompletionmodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "favoritesmodel.h"
#include "historymodel.h"
#include "storagebackend.h"
#include "urlcompletionmodel.h"
#include "pluginmanager.h"

class QString;
class QWidget;
class QIcon;
class QWebView;
class QAbstractItemModel;
class QNetworkReply;
class QNetworkAccessManager;
class IWebWidget;
class IShortcutProxy;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class CustomWebView;
			class BrowserWidget;

			class Core : public QObject
			{
				Q_OBJECT

				QMenu *PluginsMenu_;

				typedef std::vector<BrowserWidget*> widgets_t;
				widgets_t Widgets_;

				std::auto_ptr<FavoritesModel> FavoritesModel_;
				std::auto_ptr<HistoryModel> HistoryModel_;
				std::auto_ptr<URLCompletionModel> URLCompletionModel_;
				std::auto_ptr<PluginManager> PluginManager_;
				boost::shared_ptr<StorageBackend> StorageBackend_;
				QNetworkAccessManager *NetworkAccessManager_;

				QMap<QString, QObject*> Providers_;

				bool IsShuttingDown_;
				QStringList RestoredURLs_;

				QMap<QString, QString> SavedSession_;
				QList<QAction*> Unclosers_;
				const IShortcutProxy *ShortcutProxy_;

				ICoreProxy_ptr Proxy_;

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
				};

				static Core& Instance ();
				void Init ();
				void Release ();
				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				void SetProvider (QObject*, const QString&);
				QByteArray GetExpectedPluginClass () const;
				void AddPlugin (QObject*);

				QUrl MakeURL (QString) const;
				BrowserWidget* NewURL (const QString&, bool = false);
				IWebWidget* GetWidget ();
				CustomWebView* MakeWebView (bool = false);
				void Unregister (BrowserWidget*);
				QMenu* GetPluginsMenu () const;
				void ConnectSignals (BrowserWidget*);

				FavoritesModel* GetFavoritesModel () const;
				HistoryModel* GetHistoryModel () const;
				URLCompletionModel* GetURLCompletionModel () const;
				QNetworkAccessManager* GetNetworkAccessManager () const;
				void SetNetworkAccessManager (QNetworkAccessManager*);
				StorageBackend* GetStorageBackend () const;
				PluginManager* GetPluginManager () const;
				void SetShortcutProxy (const IShortcutProxy*);
				void SetShortcut (int name, const QKeySequence& shortcut);
				const IShortcutProxy* GetShortcutProxy () const;

				QIcon GetIcon (const QUrl&) const;
			private:
				void RestoreSession (bool);
				void HandleHistory (QWebView*);
			public slots:
				void importXbel ();
				void exportXbel ();
			private slots:
				void handleUnclose ();
				void handleTitleChanged (const QString&);
				void handleURLChanged (const QString&);
				void handleIconChanged (const QIcon&);
				void handleNeedToClose ();
				void handleAddToFavorites (const QString&, const QString&);
				void handleStatusBarChanged (const QString&);
				void handleTooltipChanged (QWidget*);
				void favoriteTagsUpdated (const QStringList&);
				void saveSession ();
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
				void gotEntity (const LeechCraft::DownloadEntity&);
				void couldHandle (const LeechCraft::DownloadEntity&, bool*);
				void newUnclose (QAction*);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::Core::UncloseData);

#endif

